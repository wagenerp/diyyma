
#include "diyyma/config.h"
#include "diyyma/scenegraph.h"
#include "diyyma/util.h"


ILightControllerReferrer::ILightControllerReferrer() :
  _lightController(0) {
}

ILightControllerReferrer::~ILightControllerReferrer() {
  if (_lightController) _lightController->drop();
}


ILightController *ILightControllerReferrer::lightController() { 
  return _lightController; 
}
void ILightControllerReferrer::setLightController(ILightController *l) {
  if (l==_lightController) return;
  if (_lightController) _lightController->drop();
  _lightController=l;
  if (_lightController) _lightController->grab();
}


const char *light_uniforms[][MAX_LIGHTS]={
  { 
    "u_light[0].ambient_c",
    "u_light[0].diffuse_c",
    "u_light[0].specular_c",
    "u_light[0].position_v",
    "u_light[0].falloff_factor",
    "u_light[0].flags"
  },
  { 
    "u_light[1].ambient_c",
    "u_light[1].diffuse_c",
    "u_light[1].specular_c",
    "u_light[1].position_v",
    "u_light[1].falloff_factor",
    "u_light[1].flags"
  },
  { 
    "u_light[2].ambient_c",
    "u_light[2].diffuse_c",
    "u_light[2].specular_c",
    "u_light[2].position_v",
    "u_light[2].falloff_factor",
    "u_light[2].flags"
  },
  { 
    "u_light[3].ambient_c",
    "u_light[3].diffuse_c",
    "u_light[3].specular_c",
    "u_light[3].position_v",
    "u_light[3].falloff_factor",
    "u_light[3].flags"
  },
  { 
    "u_light[4].ambient_c",
    "u_light[4].diffuse_c",
    "u_light[4].specular_c",
    "u_light[4].position_v",
    "u_light[4].falloff_factor",
    "u_light[4].flags"
  },
  { 
    "u_light[5].ambient_c",
    "u_light[5].diffuse_c",
    "u_light[5].specular_c",
    "u_light[5].position_v",
    "u_light[5].falloff_factor",
    "u_light[5].flags"
  },
  { 
    "u_light[6].ambient_c",
    "u_light[6].diffuse_c",
    "u_light[6].specular_c",
    "u_light[6].position_v",
    "u_light[6].falloff_factor",
    "u_light[6].flags"
  },
  { 
    "u_light[7].ambient_c",
    "u_light[7].diffuse_c",
    "u_light[7].specular_c",
    "u_light[7].position_v",
    "u_light[7].falloff_factor",
    "u_light[7].flags"
  }
  
};


SimpleLightController::SimpleLightController() {
  ARRAY_INIT(_nodes);
}

SimpleLightController::~SimpleLightController() {
  size_t idx;
  LightSceneNode **pnode;
  FOREACH(idx,pnode,_nodes) {
    (*pnode)->drop();
  }
  ARRAY_DESTROY(_nodes);
}

void SimpleLightController::operator+=(LightSceneNode *node) {
  if (_nodes_n>=MAX_LIGHTS) return;
  node->grab();
  APPEND(_nodes,node);
}


void SimpleLightController::activate(Shader *shd, SceneContext ctx) {
  GLint loc;
  size_t idx;
  LightSceneNode **pnode;
  Matrixf V, MV;
  Vector3f p;
  
  loc=shd->locate("u_lightCount");
  
  if (-1==loc) return;
  
  glUniform1i(loc,_nodes_n);
  
  if (_contextSource)
    V=_contextSource->context().V;
  else
    V.setIdentity();
  
  FOREACH(idx,pnode,_nodes) {
    MV=(*pnode)->absTransform()*V;
    p.set(MV.a14,MV.a24,MV.a34);
    glUniform3fv(shd->locate(light_uniforms[idx][0]),1,&(*pnode)->param.ambient_c.x);
    glUniform3fv(shd->locate(light_uniforms[idx][1]),1,&(*pnode)->param.diffuse_c.x);
    glUniform3fv(shd->locate(light_uniforms[idx][2]),1,&(*pnode)->param.specular_c.x);
    glUniform3fv(shd->locate(light_uniforms[idx][3]),1,&p.x);
    glUniform1f (shd->locate(light_uniforms[idx][4]),(*pnode)->param.falloff_factor);
    glUniform1ui(shd->locate(light_uniforms[idx][5]),(*pnode)->param.flags);
  }
  
}

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
  size_t idx;
  ISceneNode **pchild;
  FOREACH(idx,pchild,_children) {
    (*pchild)->drop();
  }
  ARRAY_DESTROY(_children);
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


CameraFollowerSceneNode::CameraFollowerSceneNode(ISceneNode *parent) :
  ISceneNode(parent)
  {
}
CameraFollowerSceneNode::~ CameraFollowerSceneNode () {
}

Matrixf CameraFollowerSceneNode::transform() {
  Matrixf M;
  
  if (_contextSource) M=_contextSource->context().V.inverse();
  else M.setIdentity();
  
  return M;
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

#ifdef DEBUG_DIYYMA_SPLINES
CubicBezierSceneNode::CubicBezierSceneNode(ISceneNode *parent) :
  IRenderableSceneNode(parent)
  {
#else
CubicBezierSceneNode::CubicBezierSceneNode(ISceneNode *parent) :
  ISceneNode(parent)
  {
#endif
  _transform.setIdentity();
  _path=new BezierPath();
  _path->grab();
  timeOffset=0;
  timeScale=1;
  loop=0;
}
CubicBezierSceneNode::~CubicBezierSceneNode() {
  if (_path) _path->drop();
}

BezierPath *CubicBezierSceneNode::path() { return _path; }

void CubicBezierSceneNode::setPath(BezierPath *p) {
  if (p==_path) return;
  if (_path) _path->drop();
  _path=p;
  if (_path) _path->grab();
}


Matrixf CubicBezierSceneNode::transform() {
  return _transform;
}

void CubicBezierSceneNode::iterate(double dt, double t) {
  if (_path) {
    _transform=_path->transformation((t-timeOffset)*timeScale,loop);
  }
}

#ifdef DEBUG_DIYYMA_SPLINES
void CubicBezierSceneNode::render(SceneContext ctx) {
  int i;
  float t;
  int nSegments;
  Vector3f p;
  Shader *shd;
  if (!_path) return;
  
  if (parent()) ctx.MVP*=parent()->absTransform();
  
  shd=reg_shd()->get("debug-linestrip");
  shd->bind();
  glUniformMatrix4fv(shd->locate("u_MVP"),1,0,&ctx.MVP.a11);
  
  nSegments=_path->segmentCount();
  
  glBegin(GL_LINE_STRIP);
    for(i=0;i<nSegments;i++) {
      for(t=0;t<1;t+=0.04) {
        p=_path->position(i+t,0);
        glVertex3fv(&p.x);
      }
    }
  glEnd();
  shd->unbind();
}

void CubicBezierSceneNode::sendGeometry() {

}
#endif


CameraSceneNode::CameraSceneNode(ISceneNode *parent) :
  STSceneNode(parent)
  {
  P.setPerspective(80,1.0/0.75,0.1,1000);
  time=0;
  pos_w = Vector3f(0,0,0);
}

CameraSceneNode::~CameraSceneNode() { }

SceneContext CameraSceneNode::context() {
  SceneContext ctx;
  Matrixf M=absTransform();
  ctx.P=P;
  ctx.V=M.inverse();
  ctx.M.setIdentity();
  ctx.MV=ctx.V;
  ctx.MVP=ctx.P*ctx.V;
  ctx.time=time;
  ctx.camPos_w = M*pos_w;
  return ctx;
}

void CameraSceneNode::iterate(double dt, double t) { time=t; }


STSTMSceneNode::STSTMSceneNode(ISceneNode *parent) : 
  IRenderableSceneNode(parent),
  IShaderReferrer(),
  IStaticMeshReferrer(),
  ITextureReferrer<MAX_STSTM_TEXTURES>(),
  _u_MVP(-1),
  _u_MV(-1),
  _u_M(-1),
  _u_V(-1),
  _u_P(-1),
  _u_time(-1),
  _u_camPos_w(-1)
  {
  staticTransform.setIdentity();
  _shaderReferrer=this;
}

STSTMSceneNode::~STSTMSceneNode() {
  
}


void STSTMSceneNode::updateUniforms() {
  _u_MVP =_shader->locate("u_MVP");
  _u_MV  =_shader->locate("u_MV");
  _u_M   =_shader->locate("u_M");
  _u_V   =_shader->locate("u_V");
  _u_P   =_shader->locate("u_P");
  _u_time=_shader->locate("u_time");
  _u_camPos_w=_shader->locate("u_camPos_w");
}

void STSTMSceneNode::render(SceneContext ctx) {
  int i;
  Matrixf M;
  if (!_mesh) return;
  
  M=absTransform();
  ctx.M=M;
  ctx.MV*=M;
  ctx.MVP*=M;
  if (_shader) {
    _shader->bind();
    for(i=0;i<MAX_STSTM_TEXTURES;i++)
      if (_textures[i] && (_texture_locs[i]!=-1))
        glUniform1i(_texture_locs[i],_textures[i]->bind());
    applyUniforms(ctx);
  }
  for(i=0;i<MAX_STSTM_TEXTURES;i++)
    if (_textures[i]) _textures[i]->bind();
  
  _mesh->bind();
  _mesh->send();
  _mesh->unbind();
  
  Texture::Unbind();
  
  
  if (_shader) {
    _shader->unbind();
  }
}
void STSTMSceneNode::sendGeometry() {
  if (!_mesh) return;
  _mesh->bind();
  _mesh->send();
  _mesh->unbind();
}

Matrixf STSTMSceneNode::transform() {
  return staticTransform;
}

void STSTMSceneNode::applyUniforms(SceneContext ctx) {
  if (-1!=_u_P   ) glUniformMatrix4fv(_u_P  ,1,0,&ctx.P.a11);
  if (-1!=_u_V   ) glUniformMatrix4fv(_u_V  ,1,0,&ctx.V.a11);
  if (-1!=_u_M   ) glUniformMatrix4fv(_u_M  ,1,0,&ctx.M.a11);
  if (-1!=_u_MV  ) glUniformMatrix4fv(_u_MV ,1,0,&ctx.MV.a11);
  if (-1!=_u_MVP ) glUniformMatrix4fv(_u_MVP,1,0,&ctx.MVP.a11);
  if (-1!=_u_time) glUniform1f(_u_time,ctx.time);
  if (-1!=_u_camPos_w) glUniform3fv(_u_camPos_w,1,&ctx.camPos_w.x);
  if (_lightController) _lightController->activate(_shader,ctx);
  
}

STMMSceneNode::STMMSceneNode(ISceneNode *parent) : 
  IRenderableSceneNode(parent),
  IStaticMeshReferrer()
  {
  staticTransform.setIdentity();
}

STMMSceneNode::~STMMSceneNode() {
}

void STMMSceneNode::render(SceneContext ctx) {
  size_t idx, nmat;
  Material *mat;
  Matrixf M;
  
  M=absTransform();
  ctx.M=M;
  ctx.MV*=M;
  ctx.MVP*=M;
  
  if (!_mesh) return;
  if (_lightController) {
    _mesh->bind();
    nmat=_mesh->materialCount();
    for(idx=0;idx<nmat;idx++) {
      mat=_mesh->material(idx).mat;
      if (!mat->shader()) continue;
      mat->bind(ctx);
      if (mat->shader()) _lightController->activate(mat->shader(),ctx);
      _mesh->send(idx);
      mat->unbind();
    }
    _mesh->unbind();
  } else {
    _mesh->render(ctx);
  }
  
}
void STMMSceneNode::sendGeometry() {
  if (!_mesh) return;
  _mesh->bind();
  _mesh->send();
  _mesh->unbind();
}

Matrixf STMMSceneNode::transform() {
  return staticTransform;
}
