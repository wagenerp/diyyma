
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

class BezierPath : public RCObject {
  private:
    ARRAY(Vector3f,_points);
    
  public:
    BezierPath();
    ~BezierPath();
    
    Vector3f up;
    
    void clear();
    void load(LineScanner *scanner);
    void load(const char *fn_in, int repositoryMask);
    int  segmentCount();
    
    Matrixf transformation(double t, int loop);
    Vector3f position(double t, int loop);
};

#endif