
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
#include "diyyma/scenecontext.h"

class LightSceneNode;

class ILightController : public virtual RCObject {
  public:
    virtual ~ILightController() { }
    virtual void activate(Shader *shd)=0;
    
};

class ILightControllerReferrer {
  protected:
    ILightController *_lightController;
  
  public:
    ILightControllerReferrer();
    ~ILightControllerReferrer();
    
    ILightController *lightController();
    void setLightController(ILightController *l);
    
};

/** \brief Controller for one or more light sources to be applied on a shader.
  *
  * The controller requires a context source to perform world->view
  * translation of camera coordinates.
  * This is done so that coordinate transformation does not have to be done
  * in the shader.
  */
class SimpleLightController : 
  public ILightController,
  public ISceneContextReferrer {
  
  private:
    ARRAY(LightSceneNode*, _nodes);
  public:
    SimpleLightController();
    ~SimpleLightController();
    
    /** \brief Transmits all the lighting information handled by us
      * to a shader by setting a bunch of uniforms.
      */
    virtual void activate(Shader *shd);
    
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
class ISceneNode : public virtual RCObject {
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
    
    /** \brief Just sends the geometry without applying any transformation
      * or setting any parameters.*/
    virtual void sendGeometry() =0;
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

/** \brief Bezier path animation. Transforms children to follow a sequence of
  * cubic bezier curves.
  *
  * The curve is defined by an array of control points consisting of the
  * starting point, followed by any number of triplets.
  * 
  * The first bezier segment starts with the starting point, uses the first
  * two points of the first triplet as control points and the final point
  * as the target.
  * Each triplet's target point will then serve as the starting point for
  * the following bezier segment.
  *
  * 
  */
#ifdef DEBUG_DIYYMA_SPLINES
class CubicBezierSceneNode : 
  public IIterator,
  public IRenderableSceneNode {
#else
class CubicBezierSceneNode : 
  public IIterator,
  public ISceneNode {
#endif
  private:
    Matrixf _transform;
    
    ARRAY(Vector3f,_points);
    
  public:
    CubicBezierSceneNode(ISceneNode *parent);
    virtual ~CubicBezierSceneNode();
    
    
    /** \brief Appends a single point to our list of control points.*/
    void operator+=(const Vector3f &p);
    
    void clear();
    
    /** \brief Causes the transformation to "roll" on turns, like jets do.
      *
      * Defaults to 0.
      */
    float rollFactor;
    
    /** \brief The up direction, used for computing the roll value of
      * the spline's rotation.
      *
      * Defaults to the positive z axis.
      */
    Vector3f up;
    
    /** \brief Point in frame time at which animation is set to start. 
      *
      * Defaults to 0.
      */
    float timeOffset;
    
    /** \brief Animation speed.
      *
      * Defaults to 1.
      */
    float timeScale;
    
    virtual Matrixf transform();
    
    /** \brief Iterates the animation. Without this no animation is performed.*/
    virtual void iterate(double dt, double t);
    
    
    
    #ifdef DEBUG_DIYYMA_SPLINES
    
    virtual void render(SceneContext ctx);
    virtual void sendGeometry();
    
    #endif
    
};

/** \brief A camera riding piggyback on a scene node. Useful for animation.
  * 
  */
class CameraSceneNode : 
  public STSceneNode,
  public ISceneContextSource,
  public IIterator {
  
  public:
    Matrixf P;
    double time;
  
  public:
    CameraSceneNode(ISceneNode *parent);
    virtual ~CameraSceneNode();
    
    virtual SceneContext context();
    
    virtual void iterate(double dt, double t);
    
};


#define MAX_STSTM_TEXTURES 8

/** \brief Static transform, shader, textures and mesh scene node.
  * 
  * Combines all of the above into one renderable.
  * Optionally, a LightController can be assigned to be queried for assigning
  * light source information to the underlying shader.
  */
class STSTMSceneNode : 
  public IRenderableSceneNode, 
  public IShaderReferrer,
  public IStaticMeshReferrer,
  public ITextureReferrer<MAX_STSTM_TEXTURES>,
  public ILightControllerReferrer {
  private:
    
    GLuint _u_MVP;
    GLuint _u_MV;
    GLuint _u_M;
    GLuint _u_V;
    GLuint _u_P;
    GLuint _u_time;
  
  public:
    STSTMSceneNode(ISceneNode *parent);
    ~STSTMSceneNode();
    
    Matrixf staticTransform;
    
    virtual void updateUniforms();
    
    virtual void render(SceneContext ctx);
    virtual void sendGeometry();
    
    virtual Matrixf transform();
    
    virtual void applyUniforms(SceneContext ctx);
  
};

/** \brief Static transform multi-material mesh.
  * 
  * In contrast to the STSTMSceneNode, this node will not apply any shaders or
  * textures when rendering a mesh.
  * Instead, the mesh's own materials are used in rendering.
  */
class STMMSceneNode : 
  public IRenderableSceneNode,
  public IStaticMeshReferrer,
  public ILightControllerReferrer {
  
  public:
    STMMSceneNode(ISceneNode *parent);
    ~STMMSceneNode();
    
    Matrixf staticTransform;
    
    virtual void render(SceneContext ctx);
    virtual void sendGeometry();
    
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