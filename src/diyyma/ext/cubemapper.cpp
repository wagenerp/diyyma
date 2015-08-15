
/** \file cubemapper.cpp
  * \author Peter Wagener
  * \brief Camera component capturing cubemap images
  *
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include <SDL2/SDL.h>
#include "GL/glew.h"

#include "diyyma/ext/cubemapper.h"

#include "IL/il.h"


Cubemapper::Cubemapper(unsigned int max_resolution) {
  _max_resolution=max_resolution;
  flags=0;
  _context.setIdentity();
  
  setProjectionParameters(1,1000);
  
  ARRAY_INIT(_cubemap);
  ARRAY_INIT(_renderpass);
  ARRAY_INIT(_colorAttachment);
  
  
  
  glGenFramebuffers(1,&_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER,_framebuffer);
  
  // our default color attachment. If a cubemap has no designated render target,
  // we use the default one. (It must be exported then)
  addColorAttachment();
  
  // Assign a depth buffer. We do not need to access it for any effects (yet)
  // so a Renderbuffer object will suffice (It's said to be faster than a 
  // texture.)
  glGenRenderbuffers(1,&_b_depth);
  glBindRenderbuffer(GL_RENDERBUFFER,_b_depth);
  glRenderbufferStorage(
    GL_RENDERBUFFER,GL_DEPTH_COMPONENT,
    max_resolution,max_resolution);
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,_framebuffer); 
  
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  
  // Allocate enough memory to capture a cubemap face's pixels.
  // This is needed to export a cubemap face to a file.
  // We use an upper bound for the required memory, which is three (RGB)
  // color components per texel, encoded as a 32 bit floating-point number
  // to support both HDR and LDR images.
  _pixel_buffer=malloc(max_resolution*max_resolution*3*sizeof(float));
}

Cubemapper::~Cubemapper() {
  size_t idx;
  Cubemap *pcube;
  IRenderPass **prenderpass;
  Texture **ptexture;
  
  FOREACH(idx,pcube,_cubemap) {
    if (pcube->fn_pattern) free((void*)pcube->fn_pattern);
    if (pcube->target    ) pcube->target->drop();
  }
  ARRAY_DESTROY(_cubemap);
  
  FOREACH(idx,prenderpass,_renderpass) {
    (*prenderpass)->drop();
  }
  ARRAY_DESTROY(_renderpass);
  
  FOREACH(idx,ptexture,_colorAttachment) {
    (*ptexture)->drop();
  }
  ARRAY_DESTROY(_colorAttachment);
  
  glDeleteFramebuffers(1,&_framebuffer);
  glDeleteRenderbuffers(1,&_b_depth);
  
  free(_pixel_buffer);
}

void Cubemapper::_initColorAttachment(
  Texture **ptex,GLenum attachment,
  int width, int height) {
  
  *ptex=new Texture();
  (*ptex)->grab();
  glBindTexture(GL_TEXTURE_2D,(*ptex)->name());
  glTexImage2D(
    GL_TEXTURE_2D,0,
    GL_R11F_G11F_B10F,
    width,height,
    0, GL_RGBA, GL_FLOAT,0);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D,0);
  glFramebufferTexture(GL_FRAMEBUFFER,attachment,(*ptex)->name(),0);
}

void Cubemapper::setProjectionParameters(float znear, float zfar) {
  _context.P.setPerspective(90,1,znear,zfar);
}

GLenum Cubemapper::addColorAttachment() {
  Texture *tex;
  
  glBindFramebuffer(GL_FRAMEBUFFER,_framebuffer);
  
  // create a new texture, initialize it for HDR rendering and assign it.
  tex=new Texture();
  tex->grab();
  glBindTexture(GL_TEXTURE_2D,tex->name());
  glTexImage2D(
    GL_TEXTURE_2D,0,
    GL_R11F_G11F_B10F,
    _max_resolution,_max_resolution,
    0, GL_RGBA, GL_FLOAT,0);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D,0);
  glFramebufferTexture(
    GL_FRAMEBUFFER,
    GL_COLOR_ATTACHMENT0+_colorAttachment_n,tex->name(),0);
  
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  
  // also enlist it so we can free it later.
  APPEND(_colorAttachment,tex);
  
  return GL_COLOR_ATTACHMENT0+_colorAttachment_n-1;
}

Texture *Cubemapper::getColorTexture(unsigned int index) {
  if (index>=_colorAttachment_n) return 0;
  return _colorAttachment_v[index];
}

void Cubemapper::addRenderpass(IRenderPass *p) {
  p->grab();
  APPEND(_renderpass,p);
}

void Cubemapper::addCubemap(
  const Vector3f &origin, unsigned int resolution,
  const char *fn_pattern, Texture *target) {
  
  Cubemap cube;
  
  if ((!fn_pattern && !target) 
  || !resolution 
  || (resolution>_max_resolution)) return;
  
  if (target) target->grab();
  
  cube.origin    =origin;
  cube.resolution=resolution;
  cube.fn_pattern =fn_pattern?strdup(fn_pattern):0;
  cube.target    =target;
  
  APPEND(_cubemap,cube);
}


void Cubemapper::setFrameTime(double time) {
  _context.time=time;
}

void Cubemapper::computeCubemaps() {
  size_t idx_cubemap;
  int    idx_face;
  size_t idx_renderpass;
  Cubemap *pcube;
  IRenderPass **prenderpass;
  
  GLenum drawBuffer=GL_COLOR_ATTACHMENT0;
  int    renderTargetChanged=1;
  char   fn_buf[512];
  
  int    viewport[4];
  
  ILuint img=0;
  
  if (!_cubemap_n || !_renderpass_n) return;
  
  // initiate exporting textures to files if desired
  if (flags&CUBEMAP_EXPORT) {
    ilEnable(IL_FILE_OVERWRITE);
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    ilGenImages(1,&img);
    ilBindImage(img);
  }
  
  glGetIntegerv(GL_VIEWPORT,viewport);
  
  glBindFramebuffer(GL_FRAMEBUFFER,_framebuffer);
  glDrawBuffers(1,&drawBuffer);
  
  FOREACH(idx_cubemap,pcube,_cubemap) {
    // if the previously rendered cubemap was rendered to a texture and 
    // the current one is not, we have to reset the render target to our
    // default texture
    if (!pcube->target && renderTargetChanged) {
      glFramebufferTexture(
        GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,
        _colorAttachment_v[0]->name(),0);
      renderTargetChanged=0;
    } else if (pcube->target) {
      renderTargetChanged=1;
    }
    glViewport(0,0,pcube->resolution,pcube->resolution);
    
    
    for(idx_face=0;idx_face<6;idx_face++) {
      // assign the cubemap's individual render target, if any.
      if (pcube->target) {
        glFramebufferTexture2D(
          GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,
          CUBEMAP_DATA[idx_face].textureTarget,
          pcube->target->name(),0);
      }
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
      
      // update the context transformation matrices
      // note that the projection matrix is set elsewhere.
      _context.V=
        CUBEMAP_DATA[idx_face].V
        *Matrixf::Translation(
          -pcube->origin.x,
          -pcube->origin.y,
          -pcube->origin.z);
      _context.MV=_context.V;
      _context.MVP=_context.P*_context.MV;
      
      // execute render passes
      FOREACH(idx_renderpass,prenderpass,_renderpass)
        (*prenderpass)->render();
      
      // export rendered face, if desired
      if ((flags&CUBEMAP_EXPORT)&&(pcube->fn_pattern)) {
        _snprintf(
          fn_buf,sizeof(fn_buf),
          pcube->fn_pattern,
          CUBEMAP_DATA[idx_face].name);
        
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(
          0,0,pcube->resolution,pcube->resolution,
          GL_RGB,(flags&CUBEMAP_HDR)?GL_FLOAT:GL_UNSIGNED_BYTE,
          _pixel_buffer);
        
        ilTexImage(
          pcube->resolution,pcube->resolution,1,
          3,IL_RGB,(flags&CUBEMAP_HDR)?IL_FLOAT:IL_UNSIGNED_BYTE,
          _pixel_buffer);
        
        ilSaveImage(fn_buf);
      }
      
    }
  }
  
  
  // cleanup
  
  glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
  
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  
  if (flags&CUBEMAP_EXPORT) {
    ilDeleteImages(1,&img);
  }
  
}


SceneContext Cubemapper::context() { return _context; }
