
#include "diyyma/scenegraph.h"
#include "diyyma/util.h"

ISceneNode::ISceneNode(ISceneNode *parent) {
  ARRAY_INIT(_children);
  if (parent) {
    parent->grab();
    _parent=parent;
    APPEND(_parent->_children,this);
  } else {
    _parent=0;
  }
}

ISceneNode::~ISceneNode() {
  int idx;
  ISceneNode **pchild;
  FOREACH(idx,pchild,_children) {
    (*pchild)->drop();
  }
  ARRAY_DESTROY(_children);
  if (_parent) {
    _parent->drop();
  }
}

ISceneNode *ISceneNode::parent() { return _parent; }

Matrixf ISceneNode::absTransform() {
  Matrixf r=transform();
  
  for(ISceneNode *n=_parent;n;n=n->_parent) 
    r=n->transform()*r;
}


IRenderableSceneNode::IRenderableSceneNode(ISceneNode *parent):
  ISceneNode(parent) {
  
}
IRenderableSceneNode::~IRenderableSceneNode() {
}


STSceneNode::STSceneNode(ISceneNode *parent) :
  ISceneNode(parent)
  {
  staticTransform.setIdentity();
}
STSceneNode::~STSceneNode() {
}

Matrixf STSceneNode::transform() {
  return staticTransform;
}


STSTMSceneNode::STSTMSceneNode(ISceneNode *parent) : 
  _mesh(0), _shader(0), 
  IRenderableSceneNode(parent)
  {
  memset(_textures,0,sizeof(_textures));
  memset(_texture_locs,0,sizeof(_texture_locs));
  staticTransform.setIdentity();
}

STSTMSceneNode::~STSTMSceneNode() {
  int i;
  if (_mesh) _mesh->drop();
  if (_shader) _shader->drop();
  
  for(i=0;i<MAX_STSTM_TEXTURES;i++)
    if (_textures[i]) _textures[i]->drop();
}

StaticMesh *STSTMSceneNode::mesh() { return _mesh; }
void STSTMSceneNode::setMesh(StaticMesh *m) {
  if (m==_mesh) return;
  if (m) m->grab();
  if (_mesh) _mesh->drop();
  _mesh=m;
}

Shader *STSTMSceneNode::shader() { return _shader; }
void STSTMSceneNode::setShader(Shader *s) {
  if (s==_shader) return;
  if (s) {
    s->grab();
    _u_MVP=s->locate("u_MVP");
    _u_MV=s->locate("u_MV");
  }
  if (_shader) _shader->drop();
  _shader=s;
}

Texture *STSTMSceneNode::texture(size_t index) {
  if (index>=MAX_STSTM_TEXTURES) return 0;
  return _textures[index];
}
size_t STSTMSceneNode::textureCount() {
  int i;
  
  for(i=0;i<MAX_STSTM_TEXTURES;i++)
    if (!_textures[i]) break;
  return i;
}
void STSTMSceneNode::addTexture(Texture *t, const char *loc) {
  int i;
  for(i=0;i<MAX_STSTM_TEXTURES;i++)
    if (!_textures[i]) {
      t->grab();
      _textures[i]=t;
      if (_shader)
        _texture_locs[i]=_shader->locate(loc);
      else 
        _texture_locs[i]=0;
    }
  
}

void STSTMSceneNode::render(SceneContext ctx) {
  int i;
  if (!_mesh) return;
  
  ctx.MV*=staticTransform;
  ctx.MVP*=staticTransform;
  
  if (_shader) {
    _shader->bind();
    for(i=0;i<MAX_STSTM_TEXTURES;i++)
      if (_texture_locs[i]) 
        glUniform1i(_texture_locs[i],i);
    
    glUniformMatrix4fv(_u_MV ,1,0,&ctx.MV.a11);
    glUniformMatrix4fv(_u_MVP,1,0,&ctx.MVP.a11);
  }
  
  for(i=0;i<MAX_STSTM_TEXTURES;i++)
    if (_textures[i]) _textures[i]->bind(i);
  
  _mesh->bind();
  _mesh->send();
  _mesh->unbind();
  
  for(i=0;i<MAX_STSTM_TEXTURES;i++)
    if (_textures[i]) _textures[i]->unbind();
  
  
  
  if (_shader) {
    _shader->unbind();
  }
}

Matrixf STSTMSceneNode::transform() {
  return staticTransform;
}