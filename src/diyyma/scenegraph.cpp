
#include "diyyma/scenegraph.h"
#include "diyyma/util.h"

ISceneNode::ISceneNode(ISceneNode *parent) {
  ARRAY_INIT(_children);
  if (parent) {
    grab();
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
    // this fucking line
    //_parent->drop();
  }
}

ISceneNode *ISceneNode::parent() { return _parent; }

Matrixf ISceneNode::absTransform() {
  Matrixf r=transform();
  
  for(ISceneNode *n=_parent;n;n=n->_parent) 
    r=n->transform()*r;

  return r;
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



LissajousSceneNode::LissajousSceneNode(ISceneNode *parent) :
  ISceneNode(parent)
  {
  amplitude.set(1,1,1);
  phase.set(0,0,0);
  frequency.set(1,1,1);
  _transform.setIdentity();
}
LissajousSceneNode::~LissajousSceneNode() {
}

Matrixf LissajousSceneNode::transform() {
  return _transform;
}

void LissajousSceneNode::iterate(double dt, double t) {
  
  Vector3f p;
  
  p=phase+frequency*t;
  p.set(cos(p.x),cos(p.y),cos(p.z));
  p^=amplitude;
  
  _transform.a14=p.x;
  _transform.a24=p.y;
  _transform.a34=p.z;
}






STSTMSceneNode::STSTMSceneNode(ISceneNode *parent) : 
  IRenderableSceneNode(parent),
  _lightController(0),
  IShaderReferrer(),
  IStaticMeshReferrer(),
  ITextureReferrer<MAX_STSTM_TEXTURES>()
  {
  staticTransform.setIdentity();
  _shaderReferrer=this;
}

STSTMSceneNode::~STSTMSceneNode() {
  if (_lightController) _lightController->drop();
  
}


void STSTMSceneNode::updateUniforms() {
  _u_MVP =_shader->locate("u_MVP");
  _u_MV  =_shader->locate("u_MV");
  _u_V   =_shader->locate("u_V");
  _u_time=_shader->locate("u_time");
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
    if (_u_V   ) glUniformMatrix4fv(_u_V  ,1,0,&ctx.V.a11);
    if (_u_MV  ) glUniformMatrix4fv(_u_MV ,1,0,&ctx.MV.a11);
    if (_u_MVP ) glUniformMatrix4fv(_u_MVP,1,0,&ctx.MVP.a11);
    if (_u_time) glUniform1f(_u_time,ctx.time);
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


LightController *STSTMSceneNode::lightController() { return _lightController; }
void STSTMSceneNode::setLightController(LightController *l) {
  if (l==_lightController) return;
  if (_lightController) _lightController->drop();
  _lightController=l;
  if (_lightController) _lightController->grab();
}