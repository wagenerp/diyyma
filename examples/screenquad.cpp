
/** \file basic.cpp
  * \author Peter Wagener
  * \brief Basic template for creating a window to be drawn to using OpenGL.
  *
  * Window creation and event handling is performed with the help of 
  * Simple Directmedia Layer (SDL) 2.0 - http://www.libsdl.org/
  *
  */

////////////////////////////////////////////////////////////// environment setup

#define TITLE "diyyma"

/** \brief Width, in pixels, of the output window being created */
#define WINDOW_WIDTH  800
/** \brief Height, in pixels, of the output window being created */
#define WINDOW_HEIGHT 600

/** \brief Major OpenGL version to use */
#define GL_MAJOR 3
/** \brief Minor OpenGL version to use */
#define GL_MINOR 3

/** \brief Iteration mode to use.
  *
  * If negative, no automatic iteration is performed: Iteration and rendering
  * occurs only as response to an event.
  * If zero, events will not be waited for, iteration is performed as rapidly
  * as the CPU/OS allow it.
  * If positive, we will wait for an event after each iteration, but no longer
  * than the specified amount of time, in milliseconds.
  */
#define ITERATE_TIMEOUT 10

/** \brief Rerendering mode to use.
  * 
  * Set to 0 to disable rendering in every iteration step. To render the scene,
  * a call to rerender() has to be made then.*/
#define AUTORENDER 1

/** \brief This will cause a main method to be created that will run the game
  * loop and call the following six entry points when appropriate:
  *
  * - init
  * - render_pre
  * - render_post
  * - iterate
  * - event
  * - cleanup
  */
#define DIYYMA_MAIN 1
#include "diyyma/diyyma.h"

/////////////////////////////////////////////////////////////////// entry points

AssetReloader *reloader;

const float SCREEN_QUAD_VERTICES[18]={ 
  -1,-1,0, 1,-1,0, 1,1,0,
  1,1,0, -1,1,0, -1,-1,0
};
class ScreenQuad : public IComponent {
  private:
    Shader *shd;
    GLuint buf;
    
    void _init() {
      
    }
  
  public:
    ScreenQuad(Shader *s) {
      shd=s;
      shd->grab();
      
      glGenBuffers(1,&buf);
      
      
      glBindBuffer(GL_ARRAY_BUFFER,buf);
      glBufferData(GL_ARRAY_BUFFER,
        sizeof(SCREEN_QUAD_VERTICES),SCREEN_QUAD_VERTICES,GL_STATIC_DRAW);
      glVertexAttribPointer(BUFIDX_VERTICES,3,GL_FLOAT,0,0,0);
      glEnableVertexAttribArray(BUFIDX_VERTICES);
      glBindBuffer(GL_ARRAY_BUFFER,0);
      
    }
    
    ~ScreenQuad() {
      glDeleteBuffers(1,&buf);
      shd->drop();
    }
    
    virtual void render() {
      shd->bind();
      glBindBuffer(GL_ARRAY_BUFFER,buf);
      glVertexAttribPointer(BUFIDX_VERTICES,3,GL_FLOAT,0,0,0);
      glEnableVertexAttribArray(BUFIDX_VERTICES);
      glBindBuffer(GL_ARRAY_BUFFER,0);
      glDrawArrays(GL_TRIANGLES,0,sizeof(SCREEN_QUAD_VERTICES)/sizeof(float));
      glDisableVertexAttribArray(BUFIDX_VERTICES);
      
      shd->unbind();
    }
    
    virtual int event(const SDL_Event *ev) {
      return 0;
    }
    
    virtual void iterate(double dt, double time) {
      
    }

};

/** \brief Initialization for the rendering environment
  *
  * This method is invoked *after* the window, renderer and context were 
  * initialized and before the event queue is entered.
  * This is the place to load any (static) resources, prepare the rendering
  * environment and preprocessing.
  *
  * \param argn number of command-line arguments passed to the application,
  * including the command.
  * \param argv array of command-line arguments, including the command itself
  * as the first element.
  */
int init(int argn, char **argv) {
  ilInit();
  vfs_registerPath("shader/",REPOSITORY_MASK_SHADER);
  vfs_registerPath("meshes/",REPOSITORY_MASK_MESH);
  vfs_registerPath("textures/",REPOSITORY_MASK_TEXTURE);
  
  glClearColor(0.1,0.1,0.12,1.0);
  
  registerComponent(reloader=new AssetReloader());
  Shader *shd=new Shader("mandelbrot");
  *reloader+=shd;
  
  registerComponent(new ScreenQuad(shd));
  return 1;
}

/** \brief Rendering method (first).
  *
  * This method is called every time the display is rerendered, either 
  * in each iteration step if AUTORENDER is set to 1 or after a call to
  * rerender(). Called *before* any components are rendered
  * 
  */
void render_pre() {
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  
}

/** \brief Rendering method (first).
  *
  * This method is called every time the display is rerendered, either 
  * in each iteration step if AUTORENDER is set to 1 or after a call to
  * rerender(). Called *after* any components are rendered
  * 
  */
void render_post() {
  
}

/** \brief Iteration method.
  *
  * This method is called every iteration step, whether rendering occurs or not.
  *
  * \param dt Time, in seconds, that passed since the last invocation,
  * give or take a few microseconds.
  */
void iterate(double dt, double time) {
}

/** \brief Event handling callback.
  *
  * Whenever an (input) event occurs on our window, this method is invoked.
  * For detailed information on event handling, refer to the SDL reference
  * manual (http://wiki.libsdl.org/SDL_Event).
  *
  * \param ev Pointer to an SDL_Event structure representing the event
  * being handled.
  */

void event(const SDL_Event *ev) {
  switch(ev->type) {
    case SDL_KEYDOWN:
      switch(ev->key.keysym.sym) {
        case SDLK_F4:
          if (ev->key.keysym.mod&(KMOD_LALT|KMOD_RALT))
            quit();
          break;
      }
      break;
  }
}

/** \brief Cleanup callback.
  *
  * This method is called after the event loop was exited and before the 
  * window, context or renderer are destroyed.
  * This is the place to free up any resources allocated during 
  * initialization or iteration.
  */

void cleanup() {
}
