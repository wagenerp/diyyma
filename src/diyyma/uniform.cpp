
/** \file manager.h
  * \author Peter Wagener
  * \brief Management of asset loading, scene composition and modification.
  *
  *
  */

#include "diyyma/uniform.h"

UniformBlock::UniformBlock() {
  ARRAY_INIT(_formats);
  _data=0;
  glGenBuffers(1,&_ubo);
}
UniformBlock::~UniformBlock() {
  glDeleteBuffers(1,&_ubo);
  if (_data) free(_data);
  ARRAY_DESTROY(_formats);
}

void UniformBlock::operator+(GLenum fmt) {
  size_t cbf;
  if (fmt>=UNIFORM_FORMAT_COUNT) return;
  
  cbf=UNIFORM_SIZE[fmt];
  
  _cb+=cbf;
  _data=realloc(_data,_cb);
  
  APPEND(_formats,fmt);
}

GLenum UniformBlock::operator[](unsigned int idx) const {
  return _formats_v[idx];
}
size_t UniformBlock::count() const { return _formats_n; }
size_t UniformBlock::cb() const { return _cb; }
void *UniformBlock::data() const { return _data; }

void UniformBlock::update() {
  glBindBuffer(GL_UNIFORM_BUFFER,_ubo);
  glBufferData(GL_UNIFORM_BUFFER,_cb,_data,GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER,0);
}

void UniformBlock::bind(GLuint idx) const {
  glBindBufferBase(GL_UNIFORM_BUFFER,idx,_ubo);
}