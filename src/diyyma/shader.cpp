
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
  int i;
  for(i=0;i<SHADER_PROGRAM_COUNT;i++) {
    _shader[i]=0;
    _sourceFiles[i]=0;
  }
  _program=glCreateProgram();
  load(basename);
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

void handleShaderIncludes(
  char *subject, size_t cc, 
  void ***code_v, size_t **code_cb_v, char ***files_v, size_t *code_n,
  int repositoryMask) {
  
  char *fn_new=0;
  size_t cb_new;
  void *data_new;
  
  char *p,*e, *fn_start, *q, *fn;
  size_t cc_fn;
  size_t i;
  
  if (!cc) cc=strlen(subject);
  if (!cc) return;
  
  p=subject; e=subject+cc;
  
  while(p+10<e) {
    if ((*p<' ')&&(*p!='\t') && (strncmp(p+1,"#include",8)==0)) {
      // remember begin of #include and skip ahead
      q=p+1;
      p+=9;
      
      
      while((p<e)&&((*p==' ')||(*p=='\t'))) p++;
      
      // read file name
      fn_start=p;
      while((p<e)&&(*p>' ')) p++;
      if ((p>=e) || (p==fn_start)) {
        LOG_WARNING("WARNING: file name expected in #include directive\n");
        goto next;
      }
      cc_fn=(size_t)p-(size_t)fn_start;
      
      fn=(char*)malloc(cc_fn+1);
      
      memcpy(fn,fn_start,cc_fn);
      fn[cc_fn]=0;
      
      // the last file is expected to be the source, without a name.
      for(i=0;i<*code_n-1;i++) if (strcmp((*files_v)[i],fn)==0) {
        goto finalize_stmt;
      }
      
      
      if (!(fn_new=vfs_locate(fn,repositoryMask))) {
        LOG_WARNING(
          "WARNING: unable to located #include'd file '%s'\n",
          fn);
        goto finalize_stmt;
      }
      
      
      
      if (!readFile(fn_new,&data_new,&cb_new)) {
        free((void*)fn_new);
        LOG_WARNING(
          "WARNING: unable to load #include'd file '%s' ('%s')\n",
          fn,fn_new);
        goto finalize_stmt;
      }
      
      (*code_n)++;
      
      
      
      
      *code_v   =(void** )realloc((void*)*code_v,   (*code_n)*sizeof(void*));
      *code_cb_v=(size_t*)realloc((void*)*code_cb_v,(*code_n)*sizeof(size_t*));
      *files_v  =(char** )realloc((void*)*files_v,  (*code_n)*sizeof(char*));
      
      
      for(i=*code_n-1;i>0;i--) { 
        (*code_v   )[i]=(*code_v   )[i-1];
        (*code_cb_v)[i]=(*code_cb_v)[i-1];
        (*files_v  )[i]=(*files_v  )[i-1];
      }
      
      (*code_v   )[0]=data_new;
      (*code_cb_v)[0]=cb_new;
      (*files_v  )[0]=fn_new;
      
      handleShaderIncludes(
        (char*)data_new,cb_new,
        code_v,code_cb_v,files_v,code_n,
        repositoryMask);
      
      finalize_stmt:
      free((void*)fn);
      
      // erase #include statement so there will be no error
      memset(q,' ',(size_t)p-(size_t)q);
      
      
      next: ;
    }
    p++;
    
  }
  
}

void handleShaderIncludes(
  const char *subject, size_t cc, 
  void ***code_v, size_t **code_cb_v, char ***files_v, size_t *code_n,
  int repositoryMask) {
  
  if (!cc) cc=strlen(subject);
  if (!cc) return;
  
  (*code_n)=1;
  
  *code_v   =(void** )malloc((*code_n)*sizeof(void*));
  *code_cb_v=(size_t*)malloc((*code_n)*sizeof(size_t*));
  *files_v  =(char** )malloc((*code_n)*sizeof(char*));
  
  (*code_v   )[0]=(void*)strdup(subject);
  (*code_cb_v)[0]=cc;
  (*files_v  )[0]=0;
  
  handleShaderIncludes(
    (char*)(*code_v)[0],cc,
    code_v,code_cb_v,files_v,code_n,
    repositoryMask);
  
}

int Shader::attach(const char *code, size_t cc, int mode) {
  int idx;
  GLuint shd;
  GLint  r = 0;
  GLint  ccLog;
  char   *log;
  
  size_t i;
  void **code_v;
  size_t *code_cb_v;
  char **files_v;
  size_t code_n;
  
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
  
  handleShaderIncludes(
    code,cc,
    &code_v,&code_cb_v,&files_v,&code_n,
    REPOSITORY_MASK_SHADER);
  
  glShaderSource(shd,code_n,(const char**)code_v,(const int*)code_cb_v);
  
  for(i=0;i<code_n;i++) {
    free((void*)code_v[i]);
    if (files_v[i]) free((void*)files_v[i]);
  }
  
  free((void*)code_v);
  free((void*)code_cb_v);
  free((void*)files_v);
  
  
  //glShaderSource(shd,1,&code,&ccCode);
  glCompileShader(shd);
  
  glGetShaderiv(shd,GL_COMPILE_STATUS,&r);
  if (r!=GL_TRUE) {
    glGetShaderiv(shd,GL_INFO_LOG_LENGTH,&ccLog);
    log=(char*)malloc(ccLog+1);
    glGetShaderInfoLog(shd,ccLog+1,&ccLog,log);
    LOG_WARNING(
      "WARNING: shader program %i compilation error:\n  %s\n",
      idx, log);
    
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

int Shader::load(const char *fn) {
  char buf[512];
  
  int i;
  _linked=0;
  for(i=0;i<SHADER_PROGRAM_COUNT;i++) if (_shader[i]) {
    glDetachShader(_program,_shader[i]);
    glDeleteShader(_shader[i]);
    _shader[i]=0;
  }
  _program=glCreateProgram();
  
  _snprintf(buf,512,"%s.vsd",fn);
  attachFile(buf,GL_VERTEX_SHADER);
  
  _snprintf(buf,512,"%s.fsd",fn);
  attachFile(buf,GL_FRAGMENT_SHADER);
  
  _snprintf(buf,512,"%s.gsd",fn);
  attachFile(buf,GL_GEOMETRY_SHADER);
  
  link();
  
  return 1;
}


IShaderReferrer::IShaderReferrer() : _shader(0) { }
IShaderReferrer::~IShaderReferrer() {
  if (_shader) _shader->drop();
}

Shader *IShaderReferrer::shader() { return _shader; }
void IShaderReferrer::setShader(Shader *s) {
  if (s==_shader) return;
  if (_shader) _shader->drop();
  _shader=s;
  if (_shader) {
    _shader->grab();
    updateUniforms();
  }
}
