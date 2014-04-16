
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

Matrixf MVP;

class PlaneComponent : public IComponent {
  private:
    Shader *shd;
    GLuint u_MVP;
    GLuint s_diffuse;
    
    Texture *t_diffuse;
    
    StaticMesh *mesh;
  
  public:
    PlaneComponent() {
      shd=new Shader("demo01");
      shd->grab();
      
      u_MVP=shd->locate("u_MVP");
      s_diffuse=shd->locate("s_diffuse");
      
      t_diffuse=new Texture("logo.tga");
      t_diffuse->grab();
      
      
      mesh=new StaticMesh();
      mesh->grab();
      mesh->loadOBJFile("plane.obj");
      
    }
    
    ~PlaneComponent() {
      shd->drop();
      t_diffuse->drop();
      mesh->drop();
    }
    
    virtual void render() {
      shd->bind();

      glUniformMatrix4fv(u_MVP,1,0,&MVP.a11);
      glUniform1i(s_diffuse,0);
      
      t_diffuse->bind(0);
      
      mesh->bind();
      mesh->send();
      mesh->unbind();
      
      t_diffuse->unbind();
      
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
  
  registerComponent(new PlaneComponent());
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
  MVP=
    Matrixf::Perspective(80,1.0/0.75,0.1,1000)
    //Matrixf::OrthogonalProjection(-10,10,-10,10,-1000,1000)
    *Matrixf::RotationY(-0.4)
    *Matrixf::Translation(20,0,-8)
    *Matrixf::RotationZ(time*0.2);
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
