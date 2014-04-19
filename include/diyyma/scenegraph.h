
/** \file scenegraph.h
  * \author Peter Wagener
  * \brief Scene Graph implementation
  *
  * The scene graph will combine render passes and hierarchial scene 
  * rendering in one data structure.
  *
  * The root scene node should therefore be a list of render pass
  * implementations, e.g. [ SkyBoxSceneNode, GroupSceneNode, ... ].
  * 
  * todo: map project's render pass draft onto scene graph classes 
  * representation.
  *
  *
  */

#ifndef _DIYYMA_SCENENODE_H
#define _DIYYMA_SCENENODE_H

#include "diyyma/shader.h"
#include "diyyma/staticmesh.h"
#include "diyyma/texture.h"
#include "diyyma/util.h"
#include "diyyma/math.h"

struct SceneContext {
  Matrixf MVP;
  Matrixf MV;
};

class ISceneNode;
class GroupSceneNode;

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

class IRenderableSceneNode : public ISceneNode {
  public:
    IRenderableSceneNode(ISceneNode *parent);
    virtual ~IRenderableSceneNode();
    
    virtual void render(SceneContext ctx) =0;
};

class STSceneNode : public ISceneNode {
  public:
    STSceneNode(ISceneNode *parent);
    virtual ~STSceneNode();
    
    Matrixf staticTransform;
    
    virtual Matrixf transform();
  
};


#define MAX_STSTM_TEXTURES 8

class STSTMSceneNode : public IRenderableSceneNode {
  private:
    StaticMesh *_mesh;
    Shader     *_shader;
    Texture    *_textures[MAX_STSTM_TEXTURES];
    GLuint      _texture_locs[MAX_STSTM_TEXTURES];
    
    GLuint _u_MVP;
    GLuint _u_MV;
  
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
    
    virtual void render(SceneContext ctx);
    
    virtual Matrixf transform();
  
};

#endif