
/** \file component.cpp
  * \author Peter Wagener
  * \brief Several component implementations
  *
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "SDL/SDL.h"
#include "GL/glew.h"

#include "diyyma/component.h"
#include "diyyma/ext/framedumper.h"

#include "IL/il.h"


FrameDumperComponent::FrameDumperComponent(
  const char* fn_pattern, int x, int y, int w, int h):
  _x(x), _y(y), _w(w), _h(h),
  _t_capture(0), _frame_index(0),
  interval(1.0/30.0) {
  _fn_pattern=strdup(fn_pattern);
  
  _pixel_buffer=malloc(w*h*3);
  
}

FrameDumperComponent::~FrameDumperComponent() { 
  free((void*)_fn_pattern);
  free((void*)_pixel_buffer);
}

void FrameDumperComponent::render() {
  
}

int FrameDumperComponent::event(const SDL_Event *ev) {
  return 0;
}

void FrameDumperComponent::iterate(double dt, double time) {
  ILuint img=0;
  char fn[512];
  if (time<_t_capture) return;
  
  ilEnable(IL_ORIGIN_SET);
  ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
  ilGenImages(1,&img);
  ilBindImage(img);
  
  glReadPixels(_x,_y,_w,_h,GL_RGB,GL_UNSIGNED_BYTE,_pixel_buffer);
  ilTexImage(_w,_h,1,3,IL_RGB,IL_UNSIGNED_BYTE,_pixel_buffer);
  
  _snprintf(fn,512,_fn_pattern,_frame_index);
  
  ilSaveImage(fn);
  
  _frame_index++;
  _t_capture+=interval;
}