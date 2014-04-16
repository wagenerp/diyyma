
/** \file shader.cpp
  * \author Peter Wagener
  * \brief Shader class implementation used for GLSL.
  *
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "GL/glew.h"

#include "diyyma/shader.h"


#include "diyyma/util.h"


Shader::Shader(): _linked(0) {
  int i;
  
  for(i=0;i<SHADER_PROGRAM_COUNT;i++) {
    _shader[i]=0;
    _sourceFiles[i]=0;
  }
  _program=glCreateProgram();
}

Shader::Shader(const char *basename): _linked(0) {
  char buf[512];
  
  int i;
  for(i=0;i<SHADER_PROGRAM_COUNT;i++) {
    _shader[i]=0;
    _sourceFiles[i]=0;
  }
  _program=glCreateProgram();
  
  _snprintf(buf,512,"%s.vsd",basename);
  attachFile(buf,GL_VERTEX_SHADER);
  
  _snprintf(buf,512,"%s.fsd",basename);
  attachFile(buf,GL_FRAGMENT_SHADER);
  
  _snprintf(buf,512,"%s.gsd",basename);
  attachFile(buf,GL_GEOMETRY_SHADER);
  
  link();
}

Shader::Shader(const char *vsd, const char *fsd, const char *gsd): _linked(0) {
  int i;
  for(i=0;i<SHADER_PROGRAM_COUNT;i++) {
    _shader[i]=0;
    _sourceFiles[i]=0;
  }
  _program=glCreateProgram();
  
  if (vsd) attach(vsd,0,GL_VERTEX_SHADER);
  if (fsd) attach(fsd,0,GL_FRAGMENT_SHADER);
  if (gsd) attach(gsd,0,GL_GEOMETRY_SHADER);
  
  link();

}

Shader::~Shader() {
  int i;
  for(i=0;i<SHADER_PROGRAM_COUNT;i++) if (_shader[i]) {
    glDetachShader(_program,_shader[i]);
    glDeleteShader(_shader[i]);
    if (_sourceFiles[i]) {
      free((void*)_sourceFiles[i]);
    }
  }
  if (_program) {
    glDeleteProgram(_program);
  }
}

GLuint Shader::program() {
  return _program;
}

GLuint Shader::locate(const char *id) {
  return glGetUniformLocation(_program,id);
}

int Shader::attach(const char *code, size_t cc, int mode) {
  int idx;
  GLint  ccCode;
  GLuint shd;
  GLint  r = 0;
  GLint  ccLog;
  char   *log;
  
  if ((idx=SHADER_INDEX(mode))==-1) {
    LOG_WARNING("WARNING: invalid shader program mode: %i\n",mode);
    return 0;
  }
  
  if (_shader[idx]) {
    LOG_WARNING("WARNING: shader program %i already attached\n",mode);
    return 0;
  }
  
  if (_linked) {
    LOG_WARNING(
      "WARNING: shader already linked, cannot attach another program\n");
    return 0;
  }
  
  shd=glCreateShader(mode);
  
  if (cc) ccCode=cc;
  else    ccCode=strlen(code);
  glShaderSource(shd,1,&code,&ccCode);
  glCompileShader(shd);
  
  glGetShaderiv(shd,GL_COMPILE_STATUS,&r);
  if (r!=GL_TRUE) {
    glGetShaderiv(shd,GL_INFO_LOG_LENGTH,&ccLog);
    log=(char*)malloc(ccLog+1);
    glGetShaderInfoLog(shd,ccLog+1,&ccLog,log);
    LOG_WARNING(
      "WARNING: shader program %i compilation error:\n  %s\n",
      mode, log);
    
    free((void*)log);
    return 0;
  }
  
  glAttachShader(_program,shd);
  
  
  _shader[idx]=shd;
  
  return 1;
}

int Shader::attachFile(const char *fn_in, int mode) {
  void *data=0;
  size_t cb;
  int r = 0;
  int idx;
  char *fn=0;
  
  if ((idx=SHADER_INDEX(mode))==-1) {
    LOG_WARNING("WARNING: invalid shader program mode: %i\n",mode);
    goto finalize;
  }
  
  if (!(fn=vfs_locate(fn_in,REPOSITORY_MASK_SHADER))) {
    LOG_WARNING(
      "WARNING: unable to find shader source file %s\n",
      fn_in);
    goto finalize;
  }
  
  if (!(r=readFile(fn,&data,&cb))) {
    LOG_WARNING(
      "WARNING: unable to read shader source file %s\n",
      fn);
    goto finalize;
  }
  
  r=attach((char*)data,cb,mode);
  
  if (_sourceFiles[idx]) {
    free((void*)_sourceFiles[idx]);
  }
  _sourceFiles[idx]=fn;
  fn=0;
  
  finalize:
  
  if (fn) free((void*)fn);
  if (data) free(data);
  
  return r;
}

int Shader::link() {
  GLint  r=0;
  GLint  ccLog=0;
  char   *log;
  
  if (_linked) {
    LOG_WARNING(
      "WARNING: shader already linked!\n");
    return 1;
  }
  glLinkProgram(_program);
  
  glGetShaderiv(_program,GL_INFO_LOG_LENGTH,&ccLog);
  if (ccLog) {
    log=(char*)malloc(ccLog+1);
    printf("%i\n",ccLog);
    glGetShaderInfoLog(_program,ccLog,&ccLog,log);
    LOG_WARNING(
      "WARNING: shader program %i link error:\n %i %s %c\n",
      _program, ccLog, log, *log);
    
    free((void*)log);
    return 0;
  }
  
  _linked=1;
  
  return 1;
  
}

void Shader::bind() {
  glUseProgram(_program);
}

void Shader::unbind() {
  glUseProgram(0);
}

void Shader::Unbind() {
  glUseProgram(0);
}

void Shader::reload() {
  int i=0;
  _linked=0;
  for(i=0;i<SHADER_PROGRAM_COUNT;i++) if (_sourceFiles[i]) {
    if (_shader[i]) {
      glDetachShader(_program,_shader[i]);
      glDeleteShader(_shader[i]);
      _shader[i]=0;
    }
    attachFile(_sourceFiles[i],SHADER_MODE(i));
  }
  link();
}

timestamp_t Shader::filesTimestamp() {
  int i=0;
  timestamp_t r=0, t;
  for(i=0;i<SHADER_PROGRAM_COUNT;i++) if (_sourceFiles[i]) {
    t=file_timestamp(_sourceFiles[i]);
    if (t>r) r=t;
  }
  
  return r;
}