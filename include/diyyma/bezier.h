
/** \file bezier.h
  * \author Peter Wagener
  * \brief Bezier path utility class
  *
  */
  
#ifndef DIYYMA_BEZIER_H
#define DIYYMA_BEZIER_H
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "diyyma/util.h"
#include "diyyma/math.h"

#define BEZIER_FORWARD_MASK 0x03

#define BEZIER_FORWARD_TARGET 0x01
#define BEZIER_FORWARD_DIRECTION 0x02

struct BezierPoint {
  Vector3f anchor_left;
  Vector3f position;
  Vector3f anchor_right;
  Vector3f forward;
  int flags;
};

struct BezierTimePoint {
  float t_in;
  float t_out;
  float a_in;
  float a_out;
  void set(float tin, float tout, float ain, float aout) {
    t_in=tin;
    t_out=tout;
    a_in=ain;
    a_out=aout;
  }
};

class BezierPath : public RCObject {
  private:
    ARRAY(BezierPoint,_points);
    ARRAY(BezierTimePoint,_timePoints);
    
  public:
    BezierPath();
    ~BezierPath();
    
    Vector3f up;
    
    void clear();
    void load(LineScanner *scanner);
    void load(const char *fn_in, int repositoryMask);
    int  segmentCount();
    void setSegmentCount(int n);
    BezierPoint *points();
    
    int  correctionCount();
    void setCorrectionCount(int n);
    BezierTimePoint *timePoints();
    
    double temporalCorrection(double t, int loop);
    
    Matrixf transformation(double t, int loop);
    Vector3f position(double t, int loop);
};

#endif