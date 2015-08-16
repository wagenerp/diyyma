
/** \file framedumper.h
  * \author Peter Wagener
  * \brief Frame dumping component
  *
  * This component should be registered *after* the very last
  * rendering component.
  *
  */
#ifndef _DIYYMA_EXT_FRAMEDUMPER_H
#define _DIYYMA_EXT_FRAMEDUMPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "SDL2/SDL.h"
#include "GL/glew.h"

#include "diyyma/component.h"

/** \brief FPS-like camera motion using mouse and WSAD. 
  *
  * This module provides an ISceneContextSource implementation and can 
  * therefore be stuffed into a render pass, light controller, etc.
  *
  * While camera motion is just one part of the deal, a public field
  * P is used for the pure projection matrix.
  * 
  * If manual intervention is desired, the fields pos and angles can be set,
  * followed by a call of the compute method to update the matrices (V and VP).
  *
  * To control motion speed and mouse sensitivity, use the fields
  * speed and mouseSensitivity, defaulting to 1.
  *
  **/
class FrameDumperComponent : public IComponent {
  private:
    char *_fn_pattern;
    int   _frame_index;
    double _t_capture;
    int _x, _y, _w, _h;
    
    void *_pixel_buffer;
    
    
  public:
    FrameDumperComponent(const char *fn_pattern, int x, int y, int w, int h);
    ~FrameDumperComponent();
    
    double interval;
    
    virtual void render();
    virtual int event(const SDL_Event *ev);
    virtual void iterate(double dt, double time);
};
#endif