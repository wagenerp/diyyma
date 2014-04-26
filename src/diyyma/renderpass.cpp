
#include "diyyma/renderpass.h"
#include "GL/glew.h"

IRenderPass::IRenderPass(): 
  _frameBufferObject(0),
  flags(0) {
  ARRAY_INIT(_drawBuffers);
}

IRenderPass::~IRenderPass() {
  ARRAY_DESTROY(_drawBuffers);
}


void IRenderPass::setFBO(GLuint fbo) {
  _frameBufferObject=fbo;
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
  transformLeft.setIdentity();
  transformRight.setIdentity();
  ARRAY_INIT(_nodes);
}

SceneNodeRenderPass::~SceneNodeRenderPass() {
  size_t idx;
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
  size_t idx;
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
  size_t idx;
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
  if (flags&RP_TRANSFORM_LEFT) {
    ctx.MV=transformLeft*ctx.MV;
    ctx.MVP=ctx.P*ctx.MV;
  }
  
  if (flags&RP_TRANSFORM_RIGHT) {
    ctx.MV*=transformRight;
    ctx.MVP*=transformRight;
  }
  
  if (flags&RP_SORT_NODES) 
    sortByDistance(Vector3f(ctx.MV.a14,ctx.MV.a24,ctx.MV.a34));
  beginPass();
  
  if (flags&RP_CLEAR)
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
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