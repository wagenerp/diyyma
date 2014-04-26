
/** \file renderpass.h
  * \author Peter Wagener
  * \brief Render passes
  *
  *
  */

#ifndef _DIYYMA_RENDERPASS_H
#define _DIYYMA_RENDERPASS_H

#include "diyyma/scenegraph.h"
#include "diyyma/component.h"
#include "diyyma/shader.h"
#include "diyyma/staticmesh.h"
#include "diyyma/texture.h"
#include "diyyma/util.h"
#include "diyyma/math.h"

#include "SDL/SDL.h"

/** \brief Causes the render pass to bind a designated frame buffer object
  * when rendering.
  */
#define RP_SET_FBO 0x01

/** \brief Causes the render pass to set a list of draw buffers when
  * rendering.
  */
#define RP_SET_DRAW_BUFFERS 0x02
/** \brief Causes the render pass to nullify view translation, used for
  * the rendering of skies (or any object considered located in infinity)
  */
#define RP_SKYMODE 0x04

/** \brief Enables depth testing in the render pass.
  *
  * Depth testing is considered to be *disabled* by default, so it will be 
  * activated *and* deactivated in a renderpass assigned with this flag,
  * while other render passes will not touch it.
  */
#define RP_DEPTH_TEST 0x10

/** \brief Enables backface culling in the render pass.
  *
  * Like depth testing, backface culling is considered to be *disabled* by 
  * default, so it will be activated *and* deactivated in a renderpass 
  * assigned with this flag,
  * while other render passes will not touch it.
  */
#define RP_CULL_FACE 0x20

/** \brief Forces the render pass to sort all nodes according to distance to
  * the view origin when rendering.
  */
#define RP_SORT_NODES 0x40


/** \brief Clears both depth and color component prior to running the actual
  * render pass.
  */
#define RP_CLEAR 0x80


/** \brief Causes the left-transformation matrix of a renderpass to be used.
  *
  * The new Model-View-Projection matrix will be 
  *       
  *      Projection * TransformLeft * ModelView
  */
#define RP_TRANSFORM_LEFT 0x100

/** \brief Causes the left-transformation matrix of a renderpass to be used.
  *
  * The new Model-View-Projection matrix will be 
  *       
  *      Projection * ModelView * TransformRight
  */
#define RP_TRANSFORM_RIGHT 0x200



class IRenderPass : public IComponent {
  protected:
    
    ARRAY(GLenum,_drawBuffers);
    GLuint _frameBufferObject;
  
  public:
    IRenderPass();
    ~IRenderPass();
    
    /** \brief Any compbination of RP_* flags 
      */
    int flags;
    
    /** \brief Assigns a frame buffer object to be activated on rendering.
      *
      * If the whole scene uses a single frame buffer, it would be better
      * to just activate it in render_pre and keep it out of all the
      * render passes.
      *
      * Calling this method also sets the RP_SET_FBO flag.
      */
    void setFBO(GLuint fbo);
    
    /** \brief Assigns a draw buffer (GL_COLOR_ATTACHMENTi enum) to the render
      * pass.
      *
      * All the assigned color attachments are set when rendering and will
      * *not* be unset afterwards.
      *
      * Calling this method also sets the RP_SET_DRAW_BUFFERS flag.
      */
    void assignDrawBuffer(GLenum buf);
    
    virtual void beginPass();
    virtual void endPass();
};

class SceneNodeRenderPass : 
  public IRenderPass,
  public ISceneContextReferrer {
  private:
    ARRAY(IRenderableSceneNode*,_nodes);
    ARRAY(double,_distance);
    
    ARRAY(GLenum,_drawBuffers);
    GLuint _frameBufferObject;
    int _setFBO;
    
  public:
    SceneNodeRenderPass();
    ~SceneNodeRenderPass();
    
    Matrixf transformLeft;
    Matrixf transformRight;
    
    void sortByDistance(const Vector3f &center);
    /** \brief sorts by distance to the camera, taken from context()->MV.
      */
    void sortByDistance();
    
    void operator+=(IRenderableSceneNode *node);
    
    virtual void render();
    virtual int event(const SDL_Event *ev);
    virtual void iterate(double dt, double time);
  
};

#define MAX_SCREENQUAD_TEXTURES 8

class ScreenQuadRenderPass : 
  public IRenderPass,
  public IShaderReferrer,
  public ITextureReferrer<MAX_SCREENQUAD_TEXTURES>,
  public ISceneContextReferrer {
  
  private:
    GLuint _b_vertices;
    GLuint _u_time;
  
  public:
    ScreenQuadRenderPass();
    ~ScreenQuadRenderPass();
    
    virtual void updateUniforms();
    
    virtual void render();
    virtual int event(const SDL_Event *ev);
    virtual void iterate(double dt, double time);
    
};



#endif