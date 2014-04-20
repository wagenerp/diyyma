
#include "diyyma/renderpass.h"

SceneNodeRenderPass::SceneNodeRenderPass(): _contextSource(0) {
  ARRAY_INIT(_nodes);
}

SceneNodeRenderPass::~SceneNodeRenderPass() {
  int idx;
  IRenderableSceneNode **pnode;
  FOREACH(idx,pnode,_nodes) {
    (*pnode)->drop();
  }
  ARRAY_DESTROY(_nodes);
  if (_contextSource) _contextSource->drop();
}

ISceneContextSource *SceneNodeRenderPass::contextSource() { return _contextSource; }
void SceneNodeRenderPass::setContextSource(ISceneContextSource *s) {
  if (_contextSource==s) return;
  if (_contextSource) _contextSource->drop();
  _contextSource=s;
  if (_contextSource) _contextSource->grab();
}

void SceneNodeRenderPass::operator+=(IRenderableSceneNode *node) {
  node->grab();
  APPEND(_nodes,node);
}

void SceneNodeRenderPass::sortByDistance(const Vector3f &origin) {
  int idx;
  IRenderableSceneNode **pnode;
  int sorted=1;
  Matrixf transform;
  
  if (_distance_n<_nodes_n) 
    ARRAY_SETSIZE(_distance,_nodes_n);
  
  FOREACH(idx,pnode,_nodes) {
    transform=(*pnode)->absTransform();
    _distance_v[idx]=(
      Vector3f(transform.a14,transform.a24,transform.a34)
      -origin).sqr();
    
    if (sorted && (idx>0)&&(_distance_v[idx-1]>_distance_v[idx]))
      sorted=0;
  }
  
  if (!sorted) 
    sortByKeys<IRenderableSceneNode*,double>(_nodes_v,_distance_v,_nodes_n);
}

void SceneNodeRenderPass::sortByDistance() {
  SceneContext ctx;
  
  if (!_contextSource) return;
  ctx=_contextSource->context();
  
  sortByDistance(Vector3f(ctx.MV.a14,ctx.MV.a24,ctx.MV.a34));
}


void SceneNodeRenderPass::render() {
  int idx;
  IRenderableSceneNode **pnode;
  SceneContext ctx;
  
  if (!_contextSource) return;
  
  ctx=_contextSource->context();
  
  FOREACH(idx,pnode,_nodes) {
    (*pnode)->render(ctx);
  }
}
int SceneNodeRenderPass::event(const SDL_Event *ev) {
  return 0;
}

void SceneNodeRenderPass::iterate(double dt, double time) {

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


LightController::LightController(): _contextSource(0) {
  ARRAY_INIT(_nodes);
}

LightController::~LightController() {
  int idx;
  LightSceneNode **pnode;
  FOREACH(idx,pnode,_nodes) {
    (*pnode)->drop();
  }
  ARRAY_DESTROY(_nodes);
  if (_contextSource) _contextSource->drop();
}

ISceneContextSource *LightController::contextSource() { return _contextSource; }
void LightController::setContextSource(ISceneContextSource *s) {
  if (_contextSource==s) return;
  if (_contextSource) _contextSource->drop();
  _contextSource=s;
  if (_contextSource) _contextSource->grab();
}

void LightController::operator+=(LightSceneNode *node) {
  if (_nodes_n>=MAX_LIGHTS) return;
  node->grab();
  APPEND(_nodes,node);
}


void LightController::activate(Shader *shd) {
  GLuint loc;
  int i;
  Matrixf V, MV;
  Vector3f p;
  
  loc=shd->locate("u_lightCount");
  
  if (!loc) return;
  
  glUniform1i(loc,_nodes_n);
  
  if (_contextSource)
    V=_contextSource->context().V;
  else
    V.setIdentity();
  
  
  for(i=0;i<_nodes_n;i++) {
    MV=_nodes_v[i]->absTransform()*V;
    p.set(MV.a14,MV.a24,MV.a34);
    glUniform3fv(shd->locate(light_uniforms[i][0]),1,&_nodes_v[i]->param.ambient_c.x);
    glUniform3fv(shd->locate(light_uniforms[i][1]),1,&_nodes_v[i]->param.diffuse_c.x);
    glUniform3fv(shd->locate(light_uniforms[i][2]),1,&_nodes_v[i]->param.specular_c.x);
    glUniform3fv(shd->locate(light_uniforms[i][3]),1,&p.x);
    glUniform1f (shd->locate(light_uniforms[i][4]),_nodes_v[i]->param.falloff_factor);
    glUniform1ui(shd->locate(light_uniforms[i][5]),_nodes_v[i]->param.flags);
  }
  
}
