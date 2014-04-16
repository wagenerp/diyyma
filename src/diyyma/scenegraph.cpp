
#include "diyyma/scenegraph.h"

GroupSceneNode::GroupSceneNode():
  _link_v(0), _link_n(0), _distance_v(0) { 
}
GroupSceneNode::~GroupSceneNode() {
  int i;
  if (_link_n) {
    for(i=0;i<_link_n;i++)
      _link_v[i]->drop();
  }
  if (_link_v) {
    free((void*)_link_v);
    free((void*)_distance_v);
  }
}

void GroupSceneNode::render(SceneContext ctx) {
  int i;
  for(i=0;i<_link_n;i++) {
    _link_v[i]->render(ctx);
  }
}

size_t GroupSceneNode::linkCount() { return _link_n; }
void GroupSceneNode::appendLink(ISceneNode *link) {
  link->grab();
  _link_v=(ISceneNode**)realloc(
    (void*)_link_v,sizeof(ISceneNode*)*(_link_n+1));
  _distance_v=(double*)realloc(
    (void*)_distance_v,sizeof(double)*(_link_n+1));
  _link_v[_link_n++]=link;
}

void GroupSceneNode::insertLink(ISceneNode *link, int before) {
  int i;
  if (before>=_link_n) {
    appendLink(link); 
    return;
  }
  
  link->grab();
  _link_v=(ISceneNode**)realloc(
    (void*)_link_v,sizeof(ISceneNode*)*(_link_n+1));
  _distance_v=(double*)realloc(
    (void*)_distance_v,sizeof(double)*(_link_n+1));
  _link_n++;
  
  for(i=_link_n-1;i>before;i--)
    _link_v[i]=_link_v[i-1];
  
  _link_v[before]=link;
}
void GroupSceneNode::removeLink(ISceneNode *link) {
  int i;
  for(i=0;i<_link_n;i++)
    if (_link_v[i]==link)
      removeLinkIndex(i--);
}
void GroupSceneNode::removeLinkIndex(int idx) {
  if ((idx<0)||(idx>=_link_n)) return;
  
  _link_v[idx]->drop();
  
  while(idx<_link_n-1) {
    _link_v[idx]=_link_v[idx+1];
    idx++;
  }
  _link_n--;
}
void GroupSceneNode::clearLinks() {
  int i;
  if (_link_n) {
    for(i=0;i<_link_n;i++)
      _link_v[i]->drop();
    free((void*)_link_v);
    _link_n=0;
    _link_v=0;
  }
}

void GroupSceneNode::sortByDistance(const Vector3f &origin, int descending) {
  int i;
  int sorted=1;
  IPositionalSceneNode *node;
  
  for (i=0;i<_link_n;i++) {
    node=dynamic_cast<IPositionalSceneNode*>(_link_v[i]);
    if (node) {
      _distance_v[i]=(node->position()-origin).sqr();
      if (sorted && (i>0)&&(_distance_v[i]<_distance_v[i+1]))
        sorted=0;
    } else {
      if (i==0) _distance_v[i]=0;
      else _distance_v[i]=_distance_v[i-1];
    }
  }
  
  if (!sorted) 
    sortByKeys<ISceneNode*,double>(_link_v,_distance_v,_link_n);
}

OffsetSceneNode::OffsetSceneNode() {
  _transform.setIdentity();
  _transformInv.setIdentity();
}

Matrixf OffsetSceneNode::transform() { return _transform; }
Matrixf OffsetSceneNode::transformInv() { return _transformInv; }
void OffsetSceneNode::setTransform(const Matrixf &t) {
  _transform=t;
  _transformInv=t.inverse();
}

void OffsetSceneNode::render(SceneContext ctx) {
  ctx.MVP*=_transform;
  ctx.MV*=_transform;
  GroupSceneNode::render(ctx);
}

Vector3f OffsetSceneNode::position() {
  return Vector3f(_transform.a14,_transform.a24,_transform.a34);
}

MeshSceneNode::MeshSceneNode() : 
  _mesh(0), _shader(0) {
  memset(_textures,0,sizeof(_textures));
  memset(_texture_locs,0,sizeof(_texture_locs));
}

MeshSceneNode::~MeshSceneNode() {
  int i;
  if (_mesh) _mesh->drop();
  if (_shader) _shader->drop();
  
  for(i=0;i<MAX_MESHSCENENODE_TEXTURES;i++)
    if (_textures[i]) _textures[i]->drop();
}

StaticMesh *MeshSceneNode::mesh() { return _mesh; }
void MeshSceneNode::setMesh(StaticMesh *m) {
  if (m==_mesh) return;
  if (m) m->grab();
  if (_mesh) _mesh->drop();
  _mesh=m;
}

Shader *MeshSceneNode::shader() { return _shader; }
void MeshSceneNode::setShader(Shader *s) {
  if (s==_shader) return;
  if (s) {
    s->grab();
    _u_MVP=s->locate("u_MVP");
    _u_MV=s->locate("u_MV");
  }
  if (_shader) _shader->drop();
  _shader=s;
}

Texture *MeshSceneNode::texture(size_t index) {
  if (index>=MAX_MESHSCENENODE_TEXTURES) return 0;
  return _textures[index];
}
size_t MeshSceneNode::textureCount() {
  int i;
  
  for(i=0;i<MAX_MESHSCENENODE_TEXTURES;i++)
    if (!_textures[i]) break;
  return i;
}
void MeshSceneNode::addTexture(Texture *t, const char *loc) {
  int i;
  for(i=0;i<MAX_MESHSCENENODE_TEXTURES;i++)
    if (!_textures[i]) {
      t->grab();
      _textures[i]=t;
      if (_shader)
        _texture_locs[i]=_shader->locate(loc);
      else 
        _texture_locs[i]=0;
    }
  
}

void MeshSceneNode::render(SceneContext ctx) {
  int i;
  if (!_mesh) return;
  
  if (_shader) {
    _shader->bind();
    for(i=0;i<MAX_MESHSCENENODE_TEXTURES;i++)
      if (_texture_locs[i]) 
        glUniform1i(_texture_locs[i],i);
    
    glUniformMatrix4fv(_u_MV ,1,0,&ctx.MV.a11);
    glUniformMatrix4fv(_u_MVP,1,0,&ctx.MVP.a11);
  }
  
  for(i=0;i<MAX_MESHSCENENODE_TEXTURES;i++)
    if (_textures[i]) _textures[i]->bind(i);
  
  _mesh->bind();
  _mesh->send();
  _mesh->unbind();
  
  for(i=0;i<MAX_MESHSCENENODE_TEXTURES;i++)
    if (_textures[i]) _textures[i]->unbind();
  
  
  
  if (_shader) {
    _shader->unbind();
  }
}