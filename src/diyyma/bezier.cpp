
/** \file bezier.cpp
  * \author Peter Wagener
  * \brief Bezier path utility class
  *
  */

#include "diyyma/bezier.h"

BezierPath::BezierPath() : up{0,0,1} {
  ARRAY_INIT(_points);
  ARRAY_INIT(_timePoints);
}
BezierPath::~BezierPath() {
  ARRAY_DESTROY(_points);
  ARRAY_DESTROY(_timePoints);
}

void BezierPath::clear() {
  ARRAY_DESTROY(_points);
  ARRAY_DESTROY(_timePoints);
}

void BezierPath::load(LineScanner *scanner) {
  SubString str;
  size_t pos;
  
  BezierPoint     newPoint;
  BezierTimePoint newTimePoint;
  int i;
  
  newPoint.flags=0;
  newPoint.forward.set(0,0,0);
  
  clear();
  
  while(scanner->getLnFirstString(&str)) {
    if (str=="n") {
      pos=scanner->tell();
      while(scanner->getLnFirstString(&str)) {
        if (str=="p") {
          for(i=0;i<9;i++)
            if (!scanner->getFloat(&newPoint.anchor_left.x+i,1)) goto scan_next;
          APPEND(_points,newPoint);
        } else if (str=="fw-target") {
          if (!_points_n) goto scan_next;
          
          for(i=0;i<3;i++)
            if (!scanner->getFloat(
              &_points_v[_points_n-1].forward.x+i,1))
              goto scan_next;
          _points_v[_points_n-1].flags|=BEZIER_FORWARD_TARGET;
        } else if (str=="fw") {
          if (!_points_n) goto scan_next;
          
          for(i=0;i<3;i++)
            if (!scanner->getFloat(
              &_points_v[_points_n-1].forward.x+i,1))
              goto scan_next;
          _points_v[_points_n-1].flags|=BEZIER_FORWARD_DIRECTION;
        } else if (str=="t") {
          for(i=0;i<4;i++)
            if (!scanner->getFloat(&newTimePoint.t_in+i,1)) goto scan_next;
          APPEND(_timePoints,newTimePoint);
        } else if (str=="n") {
          scanner->seek(pos,0);
          goto scan_completed;
        }
        
        scan_next:
        scanner->seekNewLine();
        pos=scanner->tell();
        
      } while(scanner->getLnFirstString(&str));
      
    }
  }
  
  scan_completed:
  ;
}

void BezierPath::load(const char *fn_in, int repositoryMask) {
  char *fn=0;
  void *data=0;
  size_t cb;
  LineScanner *scanner=0;
  int i;
  
  
  if (!(fn=vfs_locate(fn_in,REPOSITORY_MASK_MESH))) {
    LOG_WARNING(
      "WARNING: unable to find bezier path file '%s'\n",
      fn_in);
    goto finalize;
  }
  
  if (!readFile(fn,&data,&cb)) {
    LOG_WARNING(
      "WARNING: unable to load bezier path file '%s'\n",
      fn)
    goto finalize;
  }
  
  scanner=new LineScanner();
  scanner->grab();
  *scanner=(char*)data;
  
  load(scanner);
  
  finalize:
  
  if (fn) free((void*)fn);
  if (scanner) scanner->drop();
  if (data) free(data);
}

int BezierPath::segmentCount() {
  if (_points_n<2) return 0;
  return _points_n-1;
}

void BezierPath::setSegmentCount(int n) {
  if (n<1) return;
  _points_n=n+1;
  _points_v=(BezierPoint*)realloc(
    (void*)_points_v,
    sizeof(BezierPoint)*_points_n);
}

BezierPoint *BezierPath::points() {
  return _points_v;
}

double BezierPath::temporalCorrection(double t, int loop) {
  double t0,tl, s;
  int l;
  if (_points_n<2) return 0;
  
  if (_timePoints_n>1) {
    t0=_timePoints_v[0].t_in;
    tl=_timePoints_v[_timePoints_n-1].t_in-t0;
    
    if (loop) {
      t=fmod(t-t0,tl);
      if (t<0) t+=tl;
      t+=t0;
    } else {
      if (t<t0) return 0;
      if (t>t0+tl) return _points_n-1;
    }
    
    // find the current segment:
    
    for(l=0;l<_timePoints_n-1;l++)
      if (_timePoints_v[l+1].t_in>t) break;
    
    t0=_timePoints_v[l].t_in;
    tl=_timePoints_v[l+1].t_in-t0;
    t=(t-t0)/tl;
    s=1-t;
    
    t=BEZIER_CUBIC(
      _timePoints_v[l].t_out,
      _timePoints_v[l].a_out,
      _timePoints_v[l+1].a_in,
      _timePoints_v[l+1].t_out,
      s,t
    );
  }
  
  if (loop) {
    t=fmod(t,(float)_points_n);
    if (t<0) t+=_points_n;
  } else {
    if (t<0) return 0;
    if (t>_points_n) return _points_n;
  }
  
  return t;
}

Matrixf BezierPath::transformation(double t, int loop) {
  int idx;
  Vector3f p;
  Vector3f f,l,u;
  BezierPoint *p0, *p1;
  
  if (_points_n<2) return Matrixf(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
  
  t=temporalCorrection(t,loop);
  
  idx=(int)t;
  t-=idx;
  
  p0=_points_v+((idx  )%_points_n);
  p1=_points_v+((idx+1)%_points_n);
  
  p=Vector3f::Bezier(
    p0->position,
    p0->anchor_right,
    p1->anchor_left,
    p1->position,
    t);
  
  if ((p0->flags&p1->flags&BEZIER_FORWARD_TARGET)) {
    f=p0->forward*(1-t)+p1->forward*t-p;
    f.normalize();
  } else if ((p0->flags|p1->flags)&BEZIER_FORWARD_MASK) {
    
    if (p0->flags&BEZIER_FORWARD_TARGET) {
      f=p0->forward-p;
    } else if (p0->flags&BEZIER_FORWARD_DIRECTION) {
      f=p0->forward;
    } else {
      f=Vector3f::BezierVelocity(
        p0->position,
        p0->anchor_right,
        p1->anchor_left,
        p1->position,
        t);
    }
    if (p1->flags&BEZIER_FORWARD_TARGET) {
      l=p1->forward-p;
    } else if (p1->flags&BEZIER_FORWARD_DIRECTION) {
      l=p1->forward;
    } else {
      l=Vector3f::BezierVelocity(
        p0->position,
        p0->anchor_right,
        p1->anchor_left,
        p1->position,
        t);
    }
    
    f=f*(1-t)/f.length() + l*t/l.length();
    
    
  } else {
    f=Vector3f::BezierVelocity(
      p0->position,
      p0->anchor_right,
      p1->anchor_left,
      p1->position,
      t);
    f.normalize();
  }
  
  l=(up%f).normal();
  u=f%l;
  
  return Matrixf(
    f.x,l.x,u.x,p.x,
    f.y,l.y,u.y,p.y,
    f.z,l.z,u.z,p.z,
    0,0,0,1);
}

Vector3f BezierPath::position(double t, int loop) {
  int idx;
  BezierPoint *p0, *p1;
  
  if (_points_n<2) return Vector3f(0,0,0);
  
  t=temporalCorrection(t,loop);
  
  idx=(int)t;
  t-=idx;
  
  p0=_points_v+((idx  )%_points_n);
  p1=_points_v+((idx+1)%_points_n);
  
  return Vector3f::Bezier(
    p0->position,
    p0->anchor_right,
    p1->anchor_left,
    p1->position,
    t);
  
}
