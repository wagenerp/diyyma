
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
  
  float buf[9];
  int i;
  
  clear();
  
  while(scanner->getLnFirstString(&str)) {
    if (str=="n") {
      pos=scanner->tell();
      while(scanner->getLnFirstString(&str)) {
        if (str=="p") {
          for(i=0;i<9;i++)
            if (!scanner->getFloat(buf+i,1)) goto scan_next;
          APPEND(_points,*(Vector3f*)(buf));
          APPEND(_points,*(Vector3f*)(buf+3));
          APPEND(_points,*(Vector3f*)(buf+6));
          
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
  if (_points_n<6) return 0;
  return (_points_n)/3-1;
}

Matrixf BezierPath::transformation(double t, int loop) {
  int idx;
  int n;
  Vector3f p;
  Vector3f f,l,u;
  Vector3f a;
  int ip0, ip1, ip2, ip3;
  
  if (_points_n<6) return Matrixf(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
  
  n=(_points_n)/3;
  if (loop) {
    t=fmod(t,(float)n);
  }
  if ((t<=0) || (n==0)) {
    t=0;
    idx=0;
  } else if (t>(float)n) {
    t=1;
    idx=n-1;
  } else {
    idx=(int)t;
    t-=idx;
  }
  idx*=3;
  ip0=(idx+1)%_points_n;
  ip1=(idx+2)%_points_n;
  ip2=(idx+3)%_points_n;
  ip3=(idx+4)%_points_n;
  
  p=Vector3f::Bezier(
    _points_v[ip0],_points_v[ip1],_points_v[ip2],_points_v[ip3],t);
  f=Vector3f::BezierVelocity(
    _points_v[ip0],_points_v[ip1],_points_v[ip2],_points_v[ip3],t).normal();
  
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
  int n;
  Vector3f p;
  Vector3f f,l,u;
  Vector3f a;
  int ip0, ip1, ip2, ip3;
  
  if (_points_n<6) return Vector3f(0,0,0);
  
  n=(_points_n)/3;
  if (loop) {
    t=fmod(t,(float)n);
  }
  if ((t<=0) || (n==0)) {
    t=0;
    idx=0;
  } else if (t>(float)n) {
    t=1;
    idx=n-1;
  } else {
    idx=(int)t;
    t-=idx;
  }
  idx*=3;
  ip0=(idx+1)%_points_n;
  ip1=(idx+2)%_points_n;
  ip2=(idx+3)%_points_n;
  ip3=(idx+4)%_points_n;
  
  return Vector3f::Bezier(
    _points_v[ip0],_points_v[ip1],_points_v[ip2],_points_v[ip3],t);
}