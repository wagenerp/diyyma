
#if DIYYMA_MAINasdf

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


#include "diyyma/diyyma.h"


////////////////////////////////////////////////////////////// timey-wimey stuff
/* 
  SDL_main.h defines a makro called main for some reason, so just undef it here. 
  */
#ifdef main
#undef main
#endif

int init(int argn, char **argv);
void render();
void event(const SDL_Event *ev);
void iterate(double dt, double time);
void cleanup();

int _running=0;
int _doRender=0;

IComponent **component_v=0;
size_t       component_n=0;

void registerComponent(IComponent *c) {
  c->grab();
  component_v=(IComponent**)realloc(
    (void*)component_v,
    sizeof(IComponent*)*(component_n+1));
  component_v[component_n++]=c;
}


void quit() {
  _running=0;
}

void rerender() {
  _doRender=1;
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
  
  SDL_ASSERTJ(
    SDL_Init(SDL_INIT_VIDEO)==0,
    "SDL_Init",
    cleanup)
  
  
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
  timeScale=1.0d/(double)ticksLast;
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
      for(i=0;i<component_n;i++) if (component_v[i]->event(&ev)) goto ev_done;
      event(&ev);
      
      ev_done:
      r=SDL_PollEvent(&ev);
    }
    
    ticks=SDL_GetPerformanceCounter();
    time=(double)(ticks-ticksBegin)*timeScale;
    dt=(double)(ticks-ticksLast)*timeScale;
    ticksLast=ticks;
    
    for(i=0;i<component_n;i++) component_v[i]->iterate(dt,time);
    iterate(dt,time);
    
    #if !AUTORENDER
      if(_doRender) {
    #endif
    if (SDL_GL_GetCurrentContext()!=context)
      SDL_GL_MakeCurrent(window,context);
    for(i=0;i<component_n;i++) component_v[i]->render();
    render();
    SDL_GL_SwapWindow(window);
    #if !AUTORENDER
        _doRender=0;
      }
    #endif
  }
  
  cleanup();
  
  cleanup:
  
  for(i=0;i<component_n;i++) component_v[i]->drop();
  if (component_v) {
    free((void*)component_v);
    component_v=0;
    component_n=0;
  }
  
  if (context ) SDL_GL_DeleteContext(context);
  if (renderer) SDL_DestroyRenderer(renderer);
  if (window  ) SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}


#endif