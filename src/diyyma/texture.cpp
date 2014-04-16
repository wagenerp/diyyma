
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
GLuint loadTextureFile(const char *fn_in, GLuint tex_in) {
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
  
  ilConvertImage(IL_RGBA,IL_UNSIGNED_BYTE);
  
  if (!tex) glGenTextures(1,&tex);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,tex);
  
  glTexImage2D(
    GL_TEXTURE_2D,0,GL_RGBA,
    ilGetInteger(IL_IMAGE_WIDTH),ilGetInteger(IL_IMAGE_HEIGHT),
    0,GL_RGBA,GL_UNSIGNED_BYTE,
    ilGetData());
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  
  finalize:
  
  if (fn) free((void*)fn);
  if (img) ilDeleteImages(1,&img);
  
  return tex;
}
#else
GLuint loadTextureFile(const char *fn, GLuint tex_in=0) {
  LOG_WARNING("WARNING: No texture loading backend implemented\n");
  return 0;
}
#endif

Texture::Texture(): _filename(0), _slot(-1) { 
  glGenTextures(1,&_name);
}

Texture::Texture(const char *fn_in): _slot(-1) {
  _filename=vfs_locate(fn_in,REPOSITORY_MASK_TEXTURE);
  if (!_filename) {
    glGenTextures(1,&_name);
    _filename=strdup(fn_in);
  } else {
    _name=loadTextureFile(_filename);
  }
}

Texture::~Texture() {
  if (_filename) free((void*)_filename);
  
  if (_name) glDeleteTextures(1,&_name);
}

void Texture::reload() {
  if (!_filename) return;
  loadTextureFile(_filename,_name);
}

timestamp_t Texture::filesTimestamp() {
  if (!_filename) return 0;
  return file_timestamp(_filename);
}

void Texture::bind(int slot) {
  if ((slot<0)||(slot>=TEXTURE_SLOTS)) return;
  if (__boundTextures[slot]==_name) return;
  __boundTextures[slot]=_name;
  glActiveTexture(GL_TEXTURE0+slot);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,_name);
  _slot=slot;
}

void Texture::unbind() {
  if ((_slot<0)||(_slot>=TEXTURE_SLOTS)) return;
  if (__boundTextures[_slot]==_name) {
    __boundTextures[_slot]=0;
    glActiveTexture(GL_TEXTURE0+_slot);
    glBindTexture(GL_TEXTURE_2D,0);
  }
  _slot=-1;
}

GLuint Texture::__boundTextures[8]={0,0,0,0,0,0,0,0};

void Texture::Unbind() {
  int i;
  for(i=0;i<TEXTURE_SLOTS;i++) if (__boundTextures[i]) {
    glActiveTexture(GL_TEXTURE0+i);
    glBindTexture(GL_TEXTURE_2D,0);
    __boundTextures[i]=0;
  }
}
void Texture::Unbind(int slot) {
  if ((slot<0)||(slot>=TEXTURE_SLOTS)) return;
  if (__boundTextures[slot]) {
    glActiveTexture(GL_TEXTURE0+slot);
    glBindTexture(GL_TEXTURE_2D,0);
    __boundTextures[slot]=0;
  }
  
}