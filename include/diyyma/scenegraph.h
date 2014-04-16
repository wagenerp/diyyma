
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

class ISceneNode : public RCObject  {
  public:
    ISceneNode() { }
    virtual ~ISceneNode();
    
    virtual void render(SceneContext ctx) =0;
};

class IPositionalSceneNode {
  public:
    virtual ~IPositionalSceneNode();
    
    virtual Vector3f position()=0;
};

class IGroupSceneNode {
  public:
    virtual ~IGroupSceneNode();
    
    virtual size_t linkCount() =0;
    virtual void appendLink(ISceneNode *link) =0;
    virtual void insertLink(ISceneNode *link, int before) =0;
    virtual void removeLink(ISceneNode *link) =0;
    virtual void removeLinkIndex(int idx) =0;
    virtual void clearLinks() =0;
};

class GroupSceneNode : public RCObject, public IGroupSceneNode {
  private:
    ISceneNode **_link_v;
    size_t       _link_n;
    double      *_distance_v;
  public:
    GroupSceneNode();
    ~GroupSceneNode();
    
    virtual void render(SceneContext ctx);
    
    void sortByDistance(const Vector3f &origin, int descending);
    
    virtual size_t linkCount();
    virtual void appendLink(ISceneNode *link);
    virtual void insertLink(ISceneNode *link, int before);
    virtual void removeLink(ISceneNode *link);
    virtual void removeLinkIndex(int idx);
    virtual void clearLinks();
};

class OffsetSceneNode : public GroupSceneNode, public IPositionalSceneNode {
  private:
    Matrixf _transform;
    Matrixf _transformInv;
  public:
    OffsetSceneNode();
    ~OffsetSceneNode();
    
    
    Matrixf transform();
    Matrixf transformInv();
    void    setTransform(const Matrixf &t);
    
    virtual void render(SceneContext ctx);
    
    virtual Vector3f position();
};

#define MAX_MESHSCENENODE_TEXTURES 8

class MeshSceneNode : public ISceneNode {
  private:
    StaticMesh *_mesh;
    Shader     *_shader;
    Texture    *_textures[MAX_MESHSCENENODE_TEXTURES];
    GLuint      _texture_locs[MAX_MESHSCENENODE_TEXTURES];
    
    GLuint _u_MVP;
    GLuint _u_MV;
  
  public:
    MeshSceneNode();
    ~MeshSceneNode();
    
    StaticMesh *mesh();
    void setMesh(StaticMesh *m);
    
    Shader *shader();
    void setShader(Shader *s);
    
    Texture *texture(size_t index);
    size_t textureCount();
    void addTexture(Texture *t, const char *loc);
    
    virtual void render(SceneContext ctx);
};

#endif