
/** \file bezier.cpp
  * \author Peter Wagener
  * \brief Bezier path utility class
  *
  */

#include "diyyma/bezier.h"

BezierPath::BezierPath() : up{0,0,1} {
  ARRAY_INIT(_points);
}
BezierPath::~BezierPath() {
  ARRAY_DESTROY(_points);
}

void BezierPath::clear() {
  ARRAY_DESTROY(_points);
}

void BezierPath::load(LineScanner *scanner) {
  SubString str;
  size_t pos;
  
  BezierPoint newPoint;
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

Matrixf BezierPath::transformation(double t, int loop) {
  int idx;
  Vector3f p;
  Vector3f f,l,u;
  BezierPoint *p0, *p1;
  
  if (_points_n<2) return Matrixf(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
  
  if (loop) {
    t=fmod(t,(float)_points_n-1.0);
  }
  if ((t<=0) || (_points_n==0)) {
    t=0;
    idx=0;
  } else if (t>(float)(_points_n-1)) {
    t=1;
    idx=_points_n-2;
  } else {
    idx=(int)t;
    t-=idx;
  }
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
  
  if (loop) {
    t=fmod(t,(float)_points_n-1.0);
  }
  if ((t<=0) || (_points_n==0)) {
    t=0;
    idx=0;
  } else if (t>(float)_points_n) {
    t=1;
    idx=_points_n-1;
  } else {
    idx=(int)t;
    t-=idx;
  }
  p0=_points_v+((idx  )%_points_n);
  p1=_points_v+((idx+1)%_points_n);
  
  return Vector3f::Bezier(
    p0->position,
    p0->anchor_right,
    p1->anchor_left,
    p1->position,
    t);
  
}