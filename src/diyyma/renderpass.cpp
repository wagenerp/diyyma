
#include "diyyma/renderpass.h"
#include "GL/glew.h"

IRenderPass::IRenderPass(): 
  _setFBO(0),
  _frameBufferObject(0),
  flags(0),
  _contextSource(0) {
  ARRAY_INIT(_drawBuffers);
}

IRenderPass::~IRenderPass() {
  ARRAY_DESTROY(_drawBuffers);
}


ISceneContextSource *IRenderPass::contextSource() { return _contextSource; }
void IRenderPass::setContextSource(ISceneContextSource *s) {
  if (_contextSource==s) return;
  if (_contextSource) _contextSource->drop();
  _contextSource=s;
  if (_contextSource) _contextSource->grab();
}

void IRenderPass::setFBO(GLuint fbo) {
  _setFBO=1;
  flags|=RP_SET_FBO;
}
void IRenderPass::assignDrawBuffer(GLenum buf) {
  APPEND(_drawBuffers,buf);
  flags|=RP_SET_DRAW_BUFFERS;
}

void IRenderPass::beginPass() {
  if (flags&RP_SET_FBO) 
    glBindFramebuffer(GL_FRAMEBUFFER,_frameBufferObject);
  
  if ((flags&RP_SET_DRAW_BUFFERS)&&_drawBuffers_n) 
    glDrawBuffers(_drawBuffers_n,_drawBuffers_v);
  
  if (flags&RP_DEPTH_TEST)
    glEnable(GL_DEPTH_TEST);
  if (flags&RP_CULL_FACE) {
    glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
  }
}

void IRenderPass::endPass() {
  if (flags&RP_DEPTH_TEST)
    glDisable(GL_DEPTH_TEST);
  if (flags&RP_CULL_FACE)
    glDisable(GL_CULL_FACE);
}

SceneNodeRenderPass::SceneNodeRenderPass() {
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
  Matrixf m;
  
  if (!_contextSource) return;
  
  
  ctx=_contextSource->context();
  if (flags&RP_SKYMODE) {
    m=Matrixf(
      1,0,0, -ctx.V.a14,
      0,1,0, -ctx.V.a24,
      0,0,1, -ctx.V.a34,
      0,0,0, 1);
    ctx.V*=m;
    ctx.MV*=m;
    ctx.MVP*=m;
  }
  
  if (flags&RP_SORT_NODES) 
    sortByDistance(Vector3f(ctx.MV.a14,ctx.MV.a24,ctx.MV.a34));
  beginPass();
  
  
  FOREACH(idx,pnode,_nodes) {
    (*pnode)->render(ctx);
  }
  
  endPass();
  
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


const float SCREEN_QUAD_VERTICES[18]={ 
  -1,-1,0, 1,-1,0, 1,1,0,
  1,1,0, -1,1,0, -1,-1,0
};

ScreenQuadRenderPass::ScreenQuadRenderPass(): 
  IShaderReferrer(),
  ITextureReferrer<MAX_SCREENQUAD_TEXTURES>() {
  
  glGenBuffers(1,&_b_vertices);
  
  glBindBuffer(GL_ARRAY_BUFFER,_b_vertices);
  glBufferData(GL_ARRAY_BUFFER,
    sizeof(SCREEN_QUAD_VERTICES),SCREEN_QUAD_VERTICES,GL_STATIC_DRAW);
  glVertexAttribPointer(BUFIDX_VERTICES,3,GL_FLOAT,0,0,0);
  glEnableVertexAttribArray(BUFIDX_VERTICES);
  glBindBuffer(GL_ARRAY_BUFFER,0);
}

ScreenQuadRenderPass::~ScreenQuadRenderPass() {
  glDeleteBuffers(1,&_b_vertices);
}

void ScreenQuadRenderPass::updateUniforms() {
  _u_time=_shader->locate("u_time");
}

void ScreenQuadRenderPass::render() {
  int i;
  
  float time;
  
  if (_contextSource)
    time=_contextSource->context().time;
  
  beginPass();
  if (_shader) {
    _shader->bind();
    for(i=0;i<MAX_STSTM_TEXTURES;i++)
      if (_texture_locs[i]) 
        glUniform1i(_texture_locs[i],i);
    if (_u_time) glUniform1f(_u_time,time);
  }
  
  for(i=0;i<MAX_STSTM_TEXTURES;i++)
    if (_textures[i]) _textures[i]->bind(i);
  
  glBindBuffer(GL_ARRAY_BUFFER,_b_vertices);
  glVertexAttribPointer(BUFIDX_VERTICES,3,GL_FLOAT,0,0,0);
  glEnableVertexAttribArray(BUFIDX_VERTICES);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  glDrawArrays(GL_TRIANGLES,0,sizeof(SCREEN_QUAD_VERTICES)/sizeof(float));
  glDisableVertexAttribArray(BUFIDX_VERTICES);
  
  for(i=0;i<MAX_STSTM_TEXTURES;i++)
    if (_textures[i]) _textures[i]->unbind();
  
  if (_shader) _shader->unbind();
  
  endPass();
  
}
int ScreenQuadRenderPass::event(const SDL_Event *ev) {
  return 0;
}

void ScreenQuadRenderPass::iterate(double dt, double time) {

}