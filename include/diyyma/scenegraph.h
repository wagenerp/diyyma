
/** \file scenegraph.h
  * \author Peter Wagener
  * \brief Scene Graph implementation
  *
  * There are three different kinds of scene nodes:
  * - transformation / animation nodes:
  *   they perform some form of transformation, without any rendering
  * - renderable scene nodes:
  *   they perform only a static transformation (matrix) but are actually
  *   rendered
  * - computation nodes:
  *   these nodes carry some form of data for rendering, other than 
  *   actual visible geometry (e.g. lights)
  *
  */

#ifndef _DIYYMA_SCENENODE_H
#define _DIYYMA_SCENENODE_H

#include "diyyma/shader.h"
#include "diyyma/staticmesh.h"
#include "diyyma/texture.h"
#include "diyyma/util.h"
#include "diyyma/math.h"

/** \brief Data to be passed to scene nodes for processing,
  * containing information such as the current projection matrix.
  *
  * Also, this is used for sharing a common view/projection among 
  * multiple components, e.g. an FPSCameraComponent usually generates
  * a SceneContext, which is then fed into a SceneNodeRenderPass, etc.
  */
struct SceneContext {
  Matrixf MVP; ///<\brief The current model-view-projection matrix.
  Matrixf MV;  ///<\brief The current model-view matrix.
  Matrixf V;   ///<\brief The current view matrix.
  
  double time; ///<\brief The current frame time.
  
  void setIdentity() {
    V.setIdentity();
    MV.setIdentity();
    MVP.setIdentity();
    time=0;
  }
  
};

/** \brief Interface for a source of a scene context, usually something
  * like a camera or an offscreen render pass.
  *
  * \todo This should be an RCObject, however i cannot figure out how to
  * actually make this happen as implementing classes are RCObjects already
  * and virtual inheritance seems to produce segfaults.
  */
class ISceneContextSource : public virtual IRCObject {
  
  public:
    virtual ~ISceneContextSource() { }
    virtual SceneContext context() =0;
};

class LightSceneNode;

/** \brief Controller for one or more light sources to be applied on a shader.
  *
  * The controller requires a context source to perform world->view
  * translation of camera coordinates.
  * This is done so that coordinate transformation does not have to be done
  * in the shader.
  */
class LightController : public RCObject {
  private:
    ARRAY(LightSceneNode*, _nodes);
    ISceneContextSource *_contextSource;
  public:
    LightController();
    ~LightController();
    
    
    ISceneContextSource *contextSource();
    void setContextSource(ISceneContextSource *s);
    
    /** \brief Transmits all the lighting information handled by us
      * to a shader by setting a bunch of uniforms.
      */
    void activate(Shader *shd);
    
    void operator+=(LightSceneNode *node);
    
};

class ISceneNode;
/** \brief Abstract base class of all scene nodes.
  *
  * All scene nodes have at most one parent and any number of children.
  * Also, each node must implement a getter for a transformation matrix.
  * This class already provides a method for obtaining the absolute 
  * transformation for the node.
  *
  * Note that children are not explicitly added but a parent-child relation
  * is established through the constructor.
  */
class ISceneNode : public RCObject {
  private:
    ISceneNode *_parent;
    ARRAY(ISceneNode*,_children);
  public:
    ISceneNode(ISceneNode *parent);
    virtual ~ISceneNode();
    
    ISceneNode *parent();
    
    Matrixf absTransform();
    
    virtual Matrixf transform() =0;
    
};

/** \brief Abstract class for renderable scene nodes.
  *
  * Besides the ISceneNode methods, this one also has to implement a 
  * rendering method.
  */
class IRenderableSceneNode : public ISceneNode {
  public:
    IRenderableSceneNode(ISceneNode *parent);
    virtual ~IRenderableSceneNode();
    
    /** \brief Renders whatever the implementing scene node represents.
      *
      * \param ctx SceneContext representing the current scene information,
      * such as the view and projection matrices.
      */
    virtual void render(SceneContext ctx) =0;
};


/** \brief Static transformation scene node. Always transforms children
  * according to a publicly accessible matrix.
  */
class STSceneNode : public ISceneNode {
  public:
    STSceneNode(ISceneNode *parent);
    virtual ~STSceneNode();
    
    Matrixf staticTransform;
    
    virtual Matrixf transform();
  
};


/** \brief Lissajous path animation. Transforms children to follow a 
  * 3d lissajous curve.
  *
  * Currently only the position is animated, orientation remains unaltered.
  */
class LissajousSceneNode : public ISceneNode, public IIterator {
  private:
    Matrixf _transform;
    
  public:
    LissajousSceneNode(ISceneNode *parent);
    virtual ~LissajousSceneNode();
    
    /** \brief amplitude of each component's cosine function. */
    Vector3f amplitude;
    /** \brief phase of each component's cosine function.
      * In radians.
      */
    Vector3f phase;
    /** \brief frequency of each component's cosine function.
      * In radians per second.
      */
    Vector3f frequency;
    
    virtual Matrixf transform();
    
    /** \brief Iterates the animation. Without this no animation is performed.*/
    virtual void iterate(double dt, double t);
  
};


#define MAX_STSTM_TEXTURES 8

/** \brief Static transform, shader, textures and mesh scene node.
  * 
  * Combines all of the above into one renderable.
  * Optionally, a LightController can be assigned to be queried for assigning
  * light source information to the underlying shader.
  */
class STSTMSceneNode : public IRenderableSceneNode {
  private:
    StaticMesh *_mesh;
    Shader     *_shader;
    Texture    *_textures[MAX_STSTM_TEXTURES];
    GLuint      _texture_locs[MAX_STSTM_TEXTURES];
    
    LightController *_lightController;
    
    GLuint _u_MVP;
    GLuint _u_MV;
    GLuint _u_V;
    GLuint _u_time;
  
  public:
    STSTMSceneNode(ISceneNode *parent);
    ~STSTMSceneNode();
    
    Matrixf staticTransform;
    
    
    StaticMesh *mesh();
    void setMesh(StaticMesh *m);
    
    Shader *shader();
    void setShader(Shader *s);
    
    Texture *texture(size_t index);
    size_t textureCount();
    void addTexture(Texture *t, const char *loc);
    
    LightController *lightController();
    void setLightController(LightController *l);
    
    virtual void render(SceneContext ctx);
    
    virtual Matrixf transform();
  
};


#define LIGHT_CAST_SHADOW 0x01
#define LIGHT_USE_FALLOFF 0x02
#define LIGHT_DIRECTIONAL 0x04


#define MAX_LIGHTS 8

/** \brief Parameters for a single source of light.
  *
  * Note that in a shader, the commented-out field position_v is populated 
  * with the view-space coordinates of the light source.
  */

struct LightParams {
  Vector3f ambient_c;
  Vector3f diffuse_c;
  Vector3f specular_c;
  
  // Vector3f positionv;
  
  float    falloff_factor;
  
  u_int32_t flags;
  
};

/** \brief Light source to be put into a scene graph.
  */

class LightSceneNode : public STSceneNode {
  public:
    LightSceneNode(ISceneNode *parent);
    virtual ~LightSceneNode();
    
    LightParams param;
};



#endif