
/** \file diyyma.h
  * \author Peter Wagener
  * \brief DIYYMA main header
  *
  * Includes all neccessary headers, particularily all DIYYMA modules.
  * Also by defining DIYYMA_MAIN to non-zero, a main method is added
  * which loads up all libraries, creates an OpenGL context and calls
  * appropriate entry points.
  */

/*
#ifndef _DIYYMA_DIYYMA_H
#define _DIYYMA_DIYYMA_H
#define IL_STATIC_LIB
#define GLEW_NO_GLU
#define GLEW_STATIC
*/

#include "diyyma/config.h"
#include "diyyma/component.h"
#include "diyyma/math.h"
#include "diyyma/shader.h"
#include "diyyma/staticmesh.h"
#include "diyyma/texture.h"
#include "diyyma/util.h"
#include "diyyma/scenegraph.h"
#include "diyyma/renderpass.h"


#include "SDL/SDL.h"
#include "GL/glew.h"


#if DIYYMA_TEXTURE_IL
#include "IL/il.h"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


#if DIYYMA_MAIN

#ifndef TITLE
#define TITLE "diyyma"
#endif

#ifndef WINDOW_WIDTH
/** \brief Width, in pixels, of the output window being created */
#define WINDOW_WIDTH  800
#endif

#ifndef WINDOW_HEIGHT
/** \brief Height, in pixels, of the output window being created */
#define WINDOW_HEIGHT 600
#endif

#ifndef GL_MAJOR
/** \brief Major OpenGL version to use */
#define GL_MAJOR 3
#endif

#ifndef GL_MINOR
/** \brief Minor OpenGL version to use */
#define GL_MINOR 1
#endif

#ifndef ITERATE_TIMEOUT
/** \brief Iteration mode to use.
  *
  * If negative, no automatic iteration is performed: Iteration and rendering
  * occurs only as response to an event.
  * If zero, events will not be waited for, iteration is performed as rapidly
  * as the CPU/OS allow it.
  * If positive, we will wait for an event after each iteration, but no longer
  * than the specified amount of time, in milliseconds.
  */
#define ITERATE_TIMEOUT 100
#endif

#ifndef AUTORENDER
/** \brief Rerendering mode to use.
  * 
  * Set to 0 to disable rendering in every iteration step. To render the scene,
  * a call to rerender() has to be made then.*/
#define AUTORENDER 1
#endif

#ifndef ZBITS
#define ZBITS 24
#endif

////////////////////////////////////////////////////////////// timey-wimey stuff
/* 
  SDL_main.h defines a makro called main for some reason, so just undef it here. 
  */
#ifdef main
#undef main
#endif

int init(int argn, char **argv);
void render_pre();
void render_post();
void event(const SDL_Event *ev);
void iterate(double dt, double time);
void cleanup();

int _running=0;
int _doRender=0;

ARRAY(IComponent*,component);
ARRAY(IIterator*,iterator);

void registerComponent(IComponent *c) {
  c->grab();
  APPEND(component,c);
}

void registerIterator(IIterator *iter) {
  //iter->grab();
  APPEND(iterator,iter);
}

void quit() {
  _running=0;
}

void rerender() {
  _doRender=1;
}

void rand_init() {
  srand(time(0));
}

int main(int argn, char **argv) {
  SDL_Window    *window  =0;
  SDL_Renderer  *renderer=0;
  SDL_GLContext  context =0;
  SDL_Event      ev;
  int            r,i;
  int            vMinor, vMajor;
  double         timeScale,dt, time;
  Uint64         ticksLast=0, ticksBegin, ticks;
  
  IComponent   **pcomp;
  IIterator    **piter;
  
  rand_init();
  
  ARRAY_INIT(component);
  ARRAY_INIT(iterator);
  
  SDL_ASSERTJ(
    SDL_Init(SDL_INIT_VIDEO)==0,
    "SDL_Init",
    cleanup)
    
	ASSERT_WARN(
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,ZBITS)==0,
    "SDL_GL_SetAttribute")
  
  SDL_ASSERTJ(
    SDL_CreateWindowAndRenderer(
      WINDOW_WIDTH,WINDOW_HEIGHT,
      SDL_WINDOW_OPENGL,
      &window,&renderer)==0,
    "SDL_CreateWindowAndRenderer",
    cleanup)
  
  SDL_SetWindowTitle(window,TITLE);
    
  
  
  SDL_ASSERTJ(
    context=SDL_GL_CreateContext(window),
    "SDL_GL_CreateContext",
    cleanup)
  
  SDL_ASSERTJ(
  	SDL_GL_MakeCurrent(window,context)==0,
    "SDL_GL_MakeCurrent",
    cleanup)
  
	ASSERT_WARN(
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,GL_MAJOR)==0,
    "SDL_GL_SetAttribute")
    
	ASSERT_WARN(
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,GL_MINOR)==0,
    "SDL_GL_SetAttribute")
  
  
  
  glewExperimental = GL_TRUE;
  glewInit();
  
	ASSERT_WARN(
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,&vMajor)==0,
    "SDL_GL_GetAttribute")
	ASSERT_WARN(
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,&vMinor)==0,
    "SDL_GL_GetAttribute")
  
  printf("OpenGL version: %i.%i\n",vMajor,vMinor);
  
  ASSERTJ(
    init(argn,argv),
    "init()",
    cleanup)
  
  fflush(stdout);
  
  ticksLast=SDL_GetPerformanceFrequency();
  timeScale=(double)(1.0)/(double)(ticksLast);
  ticksLast=SDL_GetPerformanceCounter();
  ticksBegin=ticksLast;
  
  _running=1;
  _doRender=1;
  while(_running) {
    #if ITERATE_TIMEOUT==0
      r=SDL_PollEvent(&ev);
    #else
    #if ITERATE_TIMEOUT>-1 
      r=SDL_WaitEventTimeout(&ev,ITERATE_TIMEOUT);
    #else
      r=SDL_WaitEvent(&ev);
    #endif
    #endif
    while(r) {
      switch(ev.type) {
        case SDL_QUIT:
          _running=0;
          break;
      }
      FOREACH(i,pcomp,component) if ((*pcomp)->event(&ev)) goto ev_done;
      event(&ev);
      
      ev_done:
      r=SDL_PollEvent(&ev);
    }
    
    ticks=SDL_GetPerformanceCounter();
    time=(double)(ticks-ticksBegin)*timeScale;
    dt=(double)(ticks-ticksLast)*timeScale;
    ticksLast=ticks;
    
    FOREACH(i,pcomp,component) (*pcomp)->iterate(dt,time);
    FOREACH(i,piter,iterator) (*piter)->iterate(dt,time);
    iterate(dt,time);
    
    #if !AUTORENDER
      if(_doRender) {
    #endif
    if (SDL_GL_GetCurrentContext()!=context)
      SDL_GL_MakeCurrent(window,context);
    render_pre();
    FOREACH(i,pcomp,component) (*pcomp)->render();
    render_post();
    SDL_GL_SwapWindow(window);
    #if !AUTORENDER
        _doRender=0;
      }
    #endif
  }
  
  cleanup();
  
  cleanup:
  #if DIYYMA_RC_NO_AUTODELETE
    RCObject::ClearDeprecated();
  #endif
  
  reg_shd_free();
  reg_tex_free();
  reg_mesh_free();
  reg_mtl_free();
  
  FOREACH(i,pcomp,component) {
    #ifdef DIYYMA_DEBUG
      LOG_DEBUG("dropping component %i..\n",i);
      if (component_v[i]->refcount()>1) {
        LOG_WARNING(
          "WARNING: Component #%4i (%.8p) stil referred by something in cleanup!"
          " (refcount: %i)\n",
          i,component_v[i],component_v[i]->refcount());
      }
    #endif
    component_v[i]->drop();
  }
  //FOREACH(i,piter,iterator ) (*piter)->drop();
  ARRAY_DESTROY(component);
  ARRAY_DESTROY(iterator);
  
  #if DIYYMA_RC_NO_AUTODELETE
    RCObject::ClearDeprecated();
  #endif
  
  #if DIYYMA_RC_GLOBAL_LIST
    #ifdef DIYYMA_DEBUG
      printf("remaining RCObjects:\n");
      RCObject::ListObjects();
    #endif
  #endif
  
  if (context ) SDL_GL_DeleteContext(context);
  if (renderer) SDL_DestroyRenderer(renderer);
  if (window  ) SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}


#endif // DIYYMA_MAIN
/*
#endif
*/