
#include "diyyma/renderpass.h"
#include "GL/glew.h"

IRenderPass::IRenderPass(): 
  _frameBufferObject(0),
  flags(0),
  blend_src(GL_SRC_ALPHA),
  blend_trg(GL_ONE_MINUS_SRC_ALPHA),
  _viewport{0,0,0,0},
	_clearColor{0,0,0,0}{
  ARRAY_INIT(_drawBuffers);
}

IRenderPass::~IRenderPass() {
  ARRAY_DESTROY(_drawBuffers);
}

void IRenderPass::setClearColor(float r, float g, float b, float a){
	_clearColor[0] = r;
	_clearColor[1] = g;
	_clearColor[2] = b;
	_clearColor[3] = a;
}

void IRenderPass::setFBO(GLuint fbo) {
  _frameBufferObject=fbo;
  flags|=RP_SET_FBO;
}
void IRenderPass::assignDrawBuffer(GLenum buf) {
  APPEND(_drawBuffers,buf);
  flags|=RP_SET_DRAW_BUFFERS;
}

void IRenderPass::setViewport(GLint x, GLint y, GLint w, GLint h) {
  _viewport[0]=x;
  _viewport[1]=y;
  _viewport[2]=w;
  _viewport[3]=h;
  flags|=RP_SET_VIEWPORT;
}

void IRenderPass::beginPass() {
  if (flags&RP_SET_FBO) 
    glBindFramebuffer(GL_FRAMEBUFFER,_frameBufferObject);
    
  if (flags&RP_SET_VIEWPORT)
    glViewport(_viewport[0],_viewport[1],_viewport[2],_viewport[3]);
  
  if (flags&RP_NO_COLOR)
    glDrawBuffer(GL_NONE);
  else if ((flags&RP_SET_DRAW_BUFFERS)&&_drawBuffers_n) 
    glDrawBuffers(_drawBuffers_n,_drawBuffers_v);
  
  if (flags&RP_NO_DEPTH) 
    glDepthMask(0);
  if (flags&RP_DEPTH_TEST)
    glEnable(GL_DEPTH_TEST);
  if (flags&RP_CULL_FACE) {
    glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
  }
  
  if (flags&RP_TRANSLUCENT) {
    
    glEnable(GL_BLEND); 
    glBlendFunc(blend_src,blend_trg);
  }
  if (flags&RP_CLEAR_DEPTH) {
    glClear(GL_DEPTH_BUFFER_BIT);
  }
  if (flags&RP_CLEAR_COLOR) {
    glClearColor(_clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
  }
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void IRenderPass::endPass() {
  if (flags&RP_DEPTH_TEST)
    glDisable(GL_DEPTH_TEST);
  if (flags&RP_CULL_FACE)
    glDisable(GL_CULL_FACE);
  if (flags&RP_NO_DEPTH) 
    glDepthMask(GL_TRUE);
  
  if (flags&RP_TRANSLUCENT) {
    glDisable(GL_BLEND); 
  }
}

SceneNodeRenderPass::SceneNodeRenderPass() :
  _u_MVP(-1),
  _u_MV(-1),
  _u_M(-1),
  _u_V(-1),
  _u_P(-1),
  _u_time(-1),
  _u_camPos_w(-1)
  {
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

void SceneNodeRenderPass::updateUniforms() {
  _u_MVP =_shader->locate("u_MVP");
  _u_MV  =_shader->locate("u_MV");
  _u_M   =_shader->locate("u_M");
  _u_V   =_shader->locate("u_V");
  _u_P   =_shader->locate("u_P");
  _u_camPos_w=_shader->locate("u_camPos_w");
  _u_time=_shader->locate("u_time");
}

void SceneNodeRenderPass::applyUniforms(SceneContext ctx) {
  if (-1!=_u_P   ) glUniformMatrix4fv(_u_P  ,1,0,&ctx.P.a11);
  if (-1!=_u_V   ) glUniformMatrix4fv(_u_V  ,1,0,&ctx.V.a11);
  if (-1!=_u_MV  ) glUniformMatrix4fv(_u_MV ,1,0,&ctx.MV.a11);
  if (-1!=_u_MVP ) glUniformMatrix4fv(_u_MVP,1,0,&ctx.MVP.a11);
  if (-1!=_u_camPos_w) glUniform3fv(_u_camPos_w,1,&ctx.camPos_w.x);
  if (-1!=_u_time) glUniform1f(_u_time,ctx.time);
}


void SceneNodeRenderPass::render() {
  size_t idx;
  IRenderableSceneNode **pnode;
  SceneContext ctx;
  Matrixf m, MV, MVP;
  
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
  
  
  if (_shader) {
    MV=ctx.MV;
    MVP=ctx.MVP;
    _shader->bind();
    FOREACH(idx,pnode,_nodes) {
      m=(*pnode)->absTransform();
      ctx.MV=MV*m;
      ctx.MVP=MVP*m;
      applyUniforms(ctx);
      (*pnode)->sendGeometry();
    }
    _shader->unbind();
  } else {
    FOREACH(idx,pnode,_nodes) {
      (*pnode)->render(ctx);
    }
  
  }
  
  endPass();
  
}
int SceneNodeRenderPass::event(const SDL_Event *ev) {
  return 0;
}

void SceneNodeRenderPass::iterate(double dt, double time) {

}




InstanceRenderPass::InstanceRenderPass() :
  _u_MVP(-1),
  _u_MV(-1),
  _u_M(-1),
  _u_V(-1),
  _u_P(-1),
  _u_time(-1),
  _u_camPos_w(-1)
  {
  transformLeft.setIdentity();
  transformRight.setIdentity();
  ARRAY_INIT(_nodes);
  _shaderReferrer=this;
}

InstanceRenderPass::~InstanceRenderPass() {
  size_t idx;
  ISceneNode **pnode;
  FOREACH(idx,pnode,_nodes) {
    (*pnode)->drop();
  }
  ARRAY_DESTROY(_nodes);
}


void InstanceRenderPass::operator+=(ISceneNode *node) {
  node->grab();
  APPEND(_nodes,node);
}

void InstanceRenderPass::sortByDistance(const Vector3f &origin) {
  size_t idx;
  ISceneNode **pnode;
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
    sortByKeys<ISceneNode*,double>(_nodes_v,_distance_v,_nodes_n);
}

void InstanceRenderPass::sortByDistance() {
  SceneContext ctx;
  
  if (!_contextSource) return;
  ctx=_contextSource->context();
  
  sortByDistance(Vector3f(ctx.MV.a14,ctx.MV.a24,ctx.MV.a34));
}

void InstanceRenderPass::updateUniforms() {
  _u_MVP =_shader->locate("u_MVP");
  _u_MV  =_shader->locate("u_MV");
  _u_M   =_shader->locate("u_M");
  _u_V   =_shader->locate("u_V");
  _u_P   =_shader->locate("u_P");
  _u_camPos_w=_shader->locate("u_camPos_w");
  _u_time=_shader->locate("u_time");
}

void InstanceRenderPass::applyUniforms(SceneContext ctx) {
  if (-1!=_u_P   ) glUniformMatrix4fv(_u_P  ,1,0,&ctx.P.a11);
  if (-1!=_u_V   ) glUniformMatrix4fv(_u_V  ,1,0,&ctx.V.a11);
  if (-1!=_u_MV  ) glUniformMatrix4fv(_u_MV ,1,0,&ctx.MV.a11);
  if (-1!=_u_MVP ) glUniformMatrix4fv(_u_MVP,1,0,&ctx.MVP.a11);
  if (-1!=_u_camPos_w) glUniform3fv(_u_camPos_w,1,&ctx.camPos_w.x);
  if (-1!=_u_time) glUniform1f(_u_time,ctx.time);
}


void InstanceRenderPass::render() {
  size_t idx;
  ISceneNode **pnode;
  SceneContext ctx;
  Matrixf m, MV, MVP;
  int i;
  if (!_contextSource || !_shader || !_nodes_n || !_mesh) {
    return;
  }
  
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
  
  MV=ctx.MV;
  MVP=ctx.MVP;
  
  _shader->bind();
  for(i=0;i<8;i++)
    if (_textures[i] && (_texture_locs[i]!=-1))
      glUniform1i(_texture_locs[i],_textures[i]->bind());
  
  applyUniforms(ctx);
  if (_lightController) _lightController->activate(_shader,ctx);
  
  _mesh->bind();
  
  FOREACH(idx,pnode,_nodes) {
    m=(*pnode)->absTransform();
    if (-1!=_u_MV  ) { 
      ctx.MV=MV*m; 
      glUniformMatrix4fv(_u_MV ,1,0,&ctx.MV.a11); 
    }
    
    if (-1!=_u_MVP ) {
      ctx.MVP=MVP*m; 
      glUniformMatrix4fv(_u_MVP,1,0,&ctx.MVP.a11);
    }
    
    _mesh->send();
  }
  _mesh->unbind();
  
  Texture::Unbind();
  
  _shader->unbind();
  
  endPass();
  
}
int InstanceRenderPass::event(const SDL_Event *ev) {
  return 0;
}

void InstanceRenderPass::iterate(double dt, double time) {

}

const float SCREEN_QUAD_VERTICES[18]={ 
  -1,-1,0, 1,-1,0, 1,1,0,
  1,1,0, -1,1,0, -1,-1,0
};

ScreenQuadRenderPass::ScreenQuadRenderPass(): 
  IShaderReferrer(),
  ITextureReferrer<MAX_SCREENQUAD_TEXTURES>(),
  _u_time(-1) {
  
  _shaderReferrer=this;
  
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
  SceneContext ctx;
  
  
  if (_contextSource) ctx=_contextSource->context();
  else ctx.setIdentity();
  
  beginPass();
  if (_shader) {
    _shader->bind();
    for(i=0;i<MAX_SCREENQUAD_TEXTURES;i++)
      if (_textures[i] && (_texture_locs[i]!=-1)) {
        fflush(stdout);
        glUniform1i(_texture_locs[i],_textures[i]->bind());
      }
    applyUniforms(ctx);
  }
  
  for(i=0;i<MAX_SCREENQUAD_TEXTURES;i++)
    if (_textures[i]) _textures[i]->bind();
  
  glBindBuffer(GL_ARRAY_BUFFER,_b_vertices);
  glVertexAttribPointer(BUFIDX_VERTICES,3,GL_FLOAT,0,0,0);
  glEnableVertexAttribArray(BUFIDX_VERTICES);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  glDrawArrays(GL_TRIANGLES,0,sizeof(SCREEN_QUAD_VERTICES)/sizeof(float));
  glDisableVertexAttribArray(BUFIDX_VERTICES);
  
  Texture::Unbind();
  
  if (_shader) _shader->unbind();
  
  endPass();
  
}
int ScreenQuadRenderPass::event(const SDL_Event *ev) {
  return 0;
}

void ScreenQuadRenderPass::iterate(double dt, double time) {

}

void ScreenQuadRenderPass::applyUniforms(SceneContext ctx) {
  if (_u_time) glUniform1f(_u_time,ctx.time);
}

SkyBoxRenderPass::SkyBoxRenderPass() : _u_VPInv(-1) {

}

SkyBoxRenderPass::~SkyBoxRenderPass() {

}

void SkyBoxRenderPass::updateUniforms() {
  ScreenQuadRenderPass::updateUniforms();
  _u_VPInv=_shader->locate("u_VPInv");
}


void SkyBoxRenderPass::applyUniforms(SceneContext ctx) {
  Matrixf VPInv;
  ScreenQuadRenderPass::applyUniforms(ctx);
  if (-1!=_u_VPInv) {
    VPInv=ctx.V;
    VPInv.a14=VPInv.a24=VPInv.a34=0;
    VPInv=ctx.P*VPInv;
    VPInv.invert();
    glUniformMatrix4fv(_u_VPInv,1,0,&VPInv.a11);
  }
}

