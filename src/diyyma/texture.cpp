
/** \file shader.cpp
  * \author Peter Wagener
  * \brief Texture loading wrapper implementation.
  *
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "IL/il.h"

#include "GL/glew.h"
#include "diyyma/config.h"
#include "diyyma/util.h"
#include "diyyma/texture.h"

#if DIYYMA_TEXTURE_IL
GLuint loadTextureFile(const char *fn_in, GLuint tex_in, 
  GLenum target_texture, GLenum target_image,
  int HDR) {
  ILuint img=0;
  GLuint tex=tex_in;
  char *fn=0;
  
  
  if (!(fn=vfs_locate(fn_in,REPOSITORY_MASK_TEXTURE))) {
    LOG_WARNING(
      "WARNING: unable to find texture file '%s'\n",
      fn_in);
    goto finalize;
  }
  
  ilEnable(IL_ORIGIN_SET);
  ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
  ilGenImages(1,&img);
  ilBindImage(img);
  
  if (ilLoadImage(fn)!=1) {
    LOG_WARNING(
      "WARNING: unable to load texture file '%s'\n",
      fn);
    goto finalize;
  }
  
  ilConvertImage(HDR?IL_RGB:IL_RGBA,HDR?IL_FLOAT:IL_UNSIGNED_BYTE);
  
  if (!tex) glGenTextures(1,&tex);
  glBindTexture(target_texture,tex);
  glTexImage2D(
    target_image,0,HDR?GL_R11F_G11F_B10F:GL_RGBA,
    ilGetInteger(IL_IMAGE_WIDTH),ilGetInteger(IL_IMAGE_HEIGHT),
    0,HDR?GL_RGB:GL_RGBA,HDR?GL_FLOAT:GL_UNSIGNED_BYTE,
    ilGetData());
  glTexParameteri(target_texture,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(target_texture,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  finalize:
  
  if (fn) free((void*)fn);
  if (img) ilDeleteImages(1,&img);
  
  return tex;
}
#else
GLuint loadTextureFile(const char *fn_in, GLuint tex_in, 
  GLenum target_texture, GLenum target_image,
  int HDR) {
  LOG_WARNING("WARNING: No texture loading backend implemented\n");
  return 0;
}
#endif

#ifdef _MSC_VER
Texture::Texture(): 
  _filename(0), _slot(-1),
  _target(GL_TEXTURE_2D), _loadHDR(0) {
  _filename_cube[0] = 0; _filename_cube[1] = 0; _filename_cube[2] = 0;
  _filename_cube[3] = 0; _filename_cube[4] = 0; _filename_cube[5] = 0; 
  glGenTextures(1,&_name);
}
Texture::Texture(GLenum target): 
  _filename(0), _slot(-1),
  _target(target), _loadHDR(0) {
  _filename_cube[0] = 0; _filename_cube[1] = 0; _filename_cube[2] = 0;
  _filename_cube[3] = 0; _filename_cube[4] = 0; _filename_cube[5] = 0; 
  glGenTextures(1,&_name);
}
#else
Texture::Texture(): 
  _filename(0), _filename_cube{0,0,0,0,0,0},
   _slot(-1), _target(GL_TEXTURE_2D),
  _loadHDR(0) {
  glGenTextures(1,&_name);
}
Texture::Texture(GLenum target): 
  _filename(0), _filename_cube{0,0,0,0,0,0},
   _slot(-1), _target(target),
  _loadHDR(0) {
  glGenTextures(1,&_name);
}

#endif

#ifdef _MSC_VER
Texture::Texture(const char *fn_in): 
  _filename(0), _slot(-1), 
  _target(GL_TEXTURE_2D), _loadHDR(0) {
  _filename_cube[0] = 0; _filename_cube[1] = 0; _filename_cube[2] = 0;
  _filename_cube[3] = 0; _filename_cube[4] = 0; _filename_cube[5] = 0; 
  glGenTextures(1,&_name);
  load(fn_in,0);
}
#else
Texture::Texture(const char *fn_in): 
  _filename(0), _filename_cube{0,0,0,0,0,0},
   _slot(-1), _target(GL_TEXTURE_2D),
  _loadHDR(0) {
  
  glGenTextures(1,&_name);
  load(fn_in,0);
}

#endif

Texture::~Texture() {
  if (_filename) free((void*)_filename);
  
  if (_name) glDeleteTextures(1,&_name);
}

GLuint Texture::name() {
  return _name;
}
GLenum Texture::target() {
  return _target;
}

void Texture::reload() {
  int i;
  switch(_target) {
    case GL_TEXTURE_CUBE_MAP:
      for(i=0;i<6;i++) if (_filename_cube[i]) {
        loadTextureFile(
          _filename_cube[i],_name,
          GL_TEXTURE_CUBE_MAP,CUBEMAP_DATA[i].textureTarget,
          _loadHDR);
      }
      break;
    case GL_TEXTURE_2D:
      if (_filename)
        loadTextureFile(_filename,_name,_target,_target,_loadHDR);
      break;
  }
}

timestamp_t Texture::filesTimestamp() {
  int i;
  timestamp_t res=0, t;
  switch(_target) {
    case GL_TEXTURE_CUBE_MAP:
      for(i=0;i<6;i++) if (_filename_cube[i]) {
        t=file_timestamp(_filename_cube[i]);
        if (t>res) res=t;
      }
      break;
    case GL_TEXTURE_2D:
      if (_filename)
        res=file_timestamp(_filename);
      break;
  }
  return res;
}

void Texture::initCubemap(unsigned int resolution, int hdr) {
  _target=GL_TEXTURE_CUBE_MAP;
  glBindTexture(GL_TEXTURE_CUBE_MAP,_name);
  glTexImage2D(
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,0,
    hdr?GL_R11F_G11F_B10F:GL_RGB,
    resolution,resolution,0,
    GL_RGB,GL_FLOAT,0);
  glTexImage2D(
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,0,
    hdr?GL_R11F_G11F_B10F:GL_RGB,
    resolution,resolution,0,
    GL_RGB,GL_FLOAT,0);
  glTexImage2D(
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,0,
    hdr?GL_R11F_G11F_B10F:GL_RGB,
    resolution,resolution,0,
    GL_RGB,GL_FLOAT,0);
  glTexImage2D(
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,0,
    hdr?GL_R11F_G11F_B10F:GL_RGB,
    resolution,resolution,0,
    GL_RGB,GL_FLOAT,0);
  glTexImage2D(
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,0,
    hdr?GL_R11F_G11F_B10F:GL_RGB,
    resolution,resolution,0,
    GL_RGB,GL_FLOAT,0);
  glTexImage2D(
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,0,
    hdr?GL_R11F_G11F_B10F:GL_RGB,
    resolution,resolution,0,
    GL_RGB,GL_FLOAT,0);
  glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
}

void Texture::bind(int slot) {
  if ((slot<0)||(slot>=TEXTURE_SLOTS)) return;
  if (__boundTextures[slot]==this) return;
  if (__boundTextures[slot]) {
    __boundTextures[slot]->_slot=-1;
    __boundTextures[slot]->drop();
  }
  __boundTextures[slot]=this;
  grab();
  _slot=slot;
  
  glEnable(GL_TEXTURE0+slot);
  glActiveTexture(GL_TEXTURE0+slot);
  glEnable(_target);
  glBindTexture(_target,_name);
}

int Texture::bind() {
  int slot;
  if (_slot>-1) return _slot;
  for(slot=0;slot<TEXTURE_SLOTS;slot++) if (!__boundTextures[slot]) {
    __boundTextures[slot]=this;
    grab();
    _slot=slot;
    
    glEnable(GL_TEXTURE0+slot);
    glActiveTexture(GL_TEXTURE0+slot);
    glEnable(_target);
    glBindTexture(_target,_name);
    return slot;
  }
  return -1;
}

void Texture::unbind() {
  if ((_slot<0)||(_slot>=TEXTURE_SLOTS)) return;
  if (__boundTextures[_slot]==this) {
    __boundTextures[_slot]=0;
    glActiveTexture(GL_TEXTURE0+_slot);
    glBindTexture(_target,0);
    _slot=-1;
    drop();
  } else {
    _slot=-1;
  }
}

Texture *Texture::__boundTextures[8]={0,0,0,0,0,0,0,0};

void Texture::Unbind() {
  int i;
  for(i=0;i<TEXTURE_SLOTS;i++) if (__boundTextures[i]) {
    glActiveTexture(GL_TEXTURE0+i);
    glBindTexture(__boundTextures[i]->_target,0);
    __boundTextures[i]->_slot=-1;
    __boundTextures[i]->drop();
    __boundTextures[i]=0;
  }
}
void Texture::Unbind(int slot) {
  if ((slot<0)||(slot>=TEXTURE_SLOTS)) return;
  if (__boundTextures[slot]) {
    glActiveTexture(GL_TEXTURE0+slot);
    glBindTexture(__boundTextures[slot]->_target,0);
    __boundTextures[slot]->_slot=-1;
    __boundTextures[slot]->drop();
    __boundTextures[slot]=0;
  }
  
}

int Texture::load(const char *fn, int flags) {
  char fn_buf[512];
  char *fn_cube[6];
  int i;
  
  _loadHDR=flags&TEXTURE_LOAD_HDR;
  
  if (flags&TEXTURE_LOAD_CUBEMAP) {
    _target=GL_TEXTURE_CUBE_MAP;
    printf("loading cubemap texture.. %s\n",fn);
    for(i=0;i<6;i++) {
      if (_filename_cube[i]) free((void*)_filename_cube[i]);
      _snprintf(fn_buf,sizeof(fn_buf),fn,CUBEMAP_DATA[i].name);
      printf("  %s (%s): ",CUBEMAP_DATA[i].name,fn_buf);
      if (_filename_cube[i]=vfs_locate(fn_buf,REPOSITORY_MASK_TEXTURE)) {
        printf("%s\n",_filename_cube[i]);
        loadTextureFile(
          _filename_cube[i],_name,
          GL_TEXTURE_CUBE_MAP,CUBEMAP_DATA[i].textureTarget,
          _loadHDR);
      } else {
        printf(" <not found> %s\n",_filename_cube[i]);
      }
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP,_name);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
  } else {
    if (_filename) free((void*)_filename);
    _filename=vfs_locate(fn,REPOSITORY_MASK_TEXTURE);
    if (_filename) {
      loadTextureFile(_filename,_name,_target,_target,
      _loadHDR);
    } else {
      LOG_WARNING(
        "WARNING: unable to locate texture file '%s'\n",fn);
    }
  }
  return 1;
}


AssetRegistry<Texture> *_reg_tex=0;
AssetRegistry<Texture> *reg_tex() {
  if (!_reg_tex)
    _reg_tex=new AssetRegistry<Texture>(REPOSITORY_MASK_TEXTURE);
  return _reg_tex;
}


void reg_tex_free() {
  if (!_reg_tex) return;
  delete _reg_tex;
  _reg_tex=0;
}
