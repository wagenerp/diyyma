
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
  _transformFeedbackMode=GL_SEPARATE_ATTRIBS;
  ARRAY_INIT(_transformFeedbackVaryings);
}

Shader::Shader(const char *basename): _linked(0) {
  int i;
  for(i=0;i<SHADER_PROGRAM_COUNT;i++) {
    _shader[i]=0;
    _sourceFiles[i]=0;
  }
  _program=glCreateProgram();
  _transformFeedbackMode=GL_SEPARATE_ATTRIBS;
  ARRAY_INIT(_transformFeedbackVaryings);
  load(basename,0);
}

Shader::Shader(const char *vsd, const char *fsd, const char *gsd): _linked(0) {
  int i;
  for(i=0;i<SHADER_PROGRAM_COUNT;i++) {
    _shader[i]=0;
    _sourceFiles[i]=0;
  }
  _program=glCreateProgram();
  _transformFeedbackMode=GL_SEPARATE_ATTRIBS;
  ARRAY_INIT(_transformFeedbackVaryings);
  
  if (vsd) attach(vsd,0,GL_VERTEX_SHADER);
  if (fsd) attach(fsd,0,GL_FRAGMENT_SHADER);
  if (gsd) attach(gsd,0,GL_GEOMETRY_SHADER);
  
  link();

}

Shader::~Shader() {
  int i;
  size_t idx;
  char **pstr;
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
  
  FOREACH(idx,pstr,_transformFeedbackVaryings)
    free((void*)pstr);
  ARRAY_DESTROY(_transformFeedbackVaryings);
}

GLuint Shader::program() {
  return _program;
}

GLint Shader::locate(const char *id) {
  return glGetUniformLocation(_program,id);
}


int Shader::_preprocess(
  int subject_idx, 
  void ***code_v, size_t **code_cb_v, char ***files_v, size_t *code_n,
  int repositoryMask) {
  
  LineScanner *ln;
  SubString str;
  
  char *fn_new=0;
  size_t cb_new;
  void *data_new;
  
  char *p,*e, *fn_start, *q, *fn;
  char *subject, *include_start, *include_end;
  size_t cc_fn;
  size_t i;
  size_t cc=(*code_cb_v)[subject_idx];
  
  int n_added=0;
  
  subject=(char*)(*code_v)[subject_idx];
  p=subject; 
  e=subject+cc;
  
  char *loop_start=0;
  char *loop_pragma=0;
  long loop_count=0;
  
  while(p+11<e) {
    if ((*p<' ')&&(*p!='\t')&&(p[1]=='#')) {
      if (strncmp(p+2,"include ",8)==0) {
        // remember begin of #include and skip ahead
        include_start=p+1;
        p+=9;
        
        
        while((p<e)&&((*p==' ')||(*p=='\t'))) p++;
        
        // read file name
        fn_start=p;
        while((p<e)&&(*p>' ')) p++;
        if ((p>=e) || (p==fn_start)) {
          LOG_WARNING("WARNING: file name expected in #include directive\n");
          goto next;
        }
        include_end=p;
        cc_fn=(size_t)include_end-(size_t)fn_start;
        
        fn=(char*)malloc(cc_fn+1);
        
        memcpy(fn,fn_start,cc_fn);
        fn[cc_fn]=0;
        
        // include files only once
        for(i=0;i<*code_n;i++) if ((*files_v)[i]&&strcmp((*files_v)[i],fn)==0) {
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
        
        
        // split the code apart:
        // code_v[ 0 .. subject_idx-1 ]
        // code before #include
        // included code
        // code after #include
        // code_v [ subject_idx+1 .. $ ]
        //
        // this adds two more pieces of code
        (*code_n)+=2;
        n_added+=2;
        
        
        // allocate new arrays
        *code_v   =(void** )realloc((void*)*code_v,   (*code_n)*sizeof(void*));
        *code_cb_v=(size_t*)realloc((void*)*code_cb_v,(*code_n)*sizeof(size_t*));
        *files_v  =(char** )realloc((void*)*files_v,  (*code_n)*sizeof(char*));
        
        // shift following files down
        for(i=*code_n-1;i>subject_idx+1;i--) {
          (*code_v   )[i]=(*code_v   )[i-2];
          (*code_cb_v)[i]=(*code_cb_v)[i-2];
          (*files_v  )[i]=(*files_v  )[i-2];
        }
        
        (*code_cb_v)[subject_idx]=(size_t)include_start-(size_t)subject;
        
        (*code_v   )[subject_idx+1]=data_new;
        (*code_cb_v)[subject_idx+1]=cb_new;
        (*files_v  )[subject_idx+1]=fn_new;
        
        (*code_v   )[subject_idx+2]=include_end;
        (*code_cb_v)[subject_idx+2]=(size_t)e-(size_t)include_end;
        // set the file name only if it is the beginning and thus can be freed.
        (*files_v  )[subject_idx+2]=0;
        
        subject=include_end;
        subject_idx+=2;
        
        subject_idx+=_preprocess(
          subject_idx-1,
          code_v,code_cb_v,files_v,code_n,
          repositoryMask);
        
        finalize_stmt:
        
        memset(include_start,' ',(size_t)include_end-(size_t)include_start);
        
        free((void*)fn);
        
        p--;
        
      } else if (strncmp(p+2,"pragma ",7)==0) {
        ln=LineScanner::GetTemporary();
        ln->assign(p+9,e);
        
        if (!ln->getLnFirstString(&str)) goto finalize_pragma;
        
        if (str=="TFB") {
          if (!ln->getLnString(&str)) goto finalize_pragma;
          if (str=="mode") {
            if (!ln->getLnString(&str)) goto finalize_pragma;
            if (str=="separate") 
              _transformFeedbackMode=GL_SEPARATE_ATTRIBS;
            else if (str=="interleaved")
              _transformFeedbackMode=GL_INTERLEAVED_ATTRIBS;
            else
              LOG_WARNING(
                "WARNING: unrecognized TFB mode '%.*s' in shader source %s.\n",
                str.length,str.ptr,
                (*files_v)[subject_idx]);
                
          } else if (str=="varying") {
            if (!ln->getLnString(&str)) goto finalize_pragma;
            APPEND(_transformFeedbackVaryings,str.dup());
          }
          
        } else if (str=="loop-begin") {
          if (loop_start) {
            LOG_WARNING(
              "WARNING: Nested pragma loops not supported (source: %s)\n",
              (*files_v)[subject_idx]);
            goto finalize_pragma;
          }
          
          if (!ln->getLong(&loop_count,1) || (loop_count<1)) {
            LOG_WARNING(
              "WARNING: loop-begin without valid loop count (source: %s)\n",
              (*files_v)[subject_idx]);
            goto finalize_pragma;
          }
          ln->seekNewLine(0);
          loop_pragma=p;
          loop_start=p+ln->tell()+8;
          
          
        } else if (str=="loop-end") {
          if (!loop_start) {
            LOG_WARNING(
              "WARNING: loop-end without loop-begin (source: %s)\n",
              (*files_v)[subject_idx]);
            goto finalize_pragma;
          }
          
          ln->seekNewLine(9);
          
          // insert the looped code loop_count times and append the remainder
          (*code_n)+=loop_count+1;
          n_added+=loop_count+1;
          
          // allocate new arrays
          *code_v   =(void** )realloc((void*)*code_v,   (*code_n)*sizeof(void*));
          *code_cb_v=(size_t*)realloc((void*)*code_cb_v,(*code_n)*sizeof(size_t*));
          *files_v  =(char** )realloc((void*)*files_v,  (*code_n)*sizeof(char*));
          
          // shift following files down
          for(i=*code_n-1;i>subject_idx+loop_count;i--) {
            (*code_v   )[i]=(*code_v   )[i-loop_count-1];
            (*code_cb_v)[i]=(*code_cb_v)[i-loop_count-1];
            (*files_v  )[i]=(*files_v  )[i-loop_count-1];
          }
          
          (*code_cb_v)[subject_idx]=(size_t)loop_pragma-(size_t)subject;
          
          subject_idx++;
          
          for(i=0;i<loop_count;i++) {
            (*code_v   )[subject_idx]=loop_start;
            (*code_cb_v)[subject_idx]=(size_t)p-(size_t)loop_start+1;
            (*files_v  )[subject_idx]=0;
            
            subject_idx++;
          }
          
          subject=p+ln->tell()+8;
          
          (*code_v   )[subject_idx]=subject;
          (*code_cb_v)[subject_idx]=(size_t)e-(size_t)subject;
          (*files_v  )[subject_idx]=0;
          
          loop_start=0;
          loop_pragma=0;
          loop_count=0;
          
        }
        
        finalize_pragma:
        ln->seekNewLine(0);
        p=p+ln->tell()+7;
        
        ln->drop();
      }
      next: ;
    }
    p++;
    
  }
  
  return n_added;
}

void Shader::_preprocess(
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
  
  _preprocess(
    0,
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
  
  _preprocess(
    code,cc,
    &code_v,&code_cb_v,&files_v,&code_n,
    REPOSITORY_MASK_SHADER);
    
  
  glShaderSource(shd,code_n,(const char**)code_v,(const int*)code_cb_v);
  
  for(i=0;i<code_n;i++) if (files_v[i]) {
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
      "WARNING: shader %s program %i compilation error:\n  %s\n",
      _sourceFiles[idx],idx, log);
    
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
  
  if (_sourceFiles[idx]) {
    free((void*)_sourceFiles[idx]);
  }
  _sourceFiles[idx]=fn;
  
  r=attach((char*)data,cb,mode);
  
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
  
  if (_transformFeedbackVaryings_n) {
    glTransformFeedbackVaryings(
      _program,
      _transformFeedbackVaryings_n,
      (const GLchar**)_transformFeedbackVaryings_v,
      _transformFeedbackMode);
  }
  
  
  glLinkProgram(_program);
  
  glGetShaderiv(_program,GL_INFO_LOG_LENGTH,&ccLog);
  if (ccLog) {
    log=(char*)malloc(ccLog+1);
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
  size_t idx;
  char **pstr;
  
  FOREACH(idx,pstr,_transformFeedbackVaryings)
    free((void*)pstr);
  ARRAY_DESTROY(_transformFeedbackVaryings);
  
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

int Shader::load(const char *fn, int flags) {
  char buf[512];
  size_t idx;
  char **pstr;
  
  FOREACH(idx,pstr,_transformFeedbackVaryings)
    free((void*)pstr);
  ARRAY_DESTROY(_transformFeedbackVaryings);
  
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


IShaderReferrer::IShaderReferrer() : _shader(0), _id_shader(0) { }
IShaderReferrer::~IShaderReferrer() {
  if (_shader) _shader->drop();
  //if (_id_shader) free((void*)_id_shader);
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
void IShaderReferrer::setShader(const char *id) {
  Shader *shd;
  shd=reg_shd()->get(id);
  
  if (shd==_shader) return;
  if (_id_shader) free((void*)_id_shader);
  if (shd) {
    setShader(shd);
    _id_shader=strdup(id);
  } else {
    setShader((Shader*)0);
    _id_shader=0;
  }
  
}



AssetRegistry<Shader> *_reg_shd=0;
AssetRegistry<Shader> *reg_shd() {
  if (!_reg_shd)
    _reg_shd=new AssetRegistry<Shader>(REPOSITORY_MASK_SHADER);
  return _reg_shd;
}



void reg_shd_free() {
  if (!_reg_shd) return;
  delete _reg_shd;
  _reg_shd=0;
}
