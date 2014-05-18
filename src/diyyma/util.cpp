
/** \file shader.cpp
  * \author Peter Wagener
  * \brief Miscellaneous utility method implementations.
  *
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diyyma/config.h"
#include "diyyma/util.h"

int _logMask=LOG_MASK_ERROR|LOG_MASK_WARNING|LOG_MASK_INFO;
int logMask() { return _logMask; }
void setLogMask(int v) { _logMask=v; }


int readFile(const char *fn,void **data, size_t *cb) {
  FILE    *f;
  int      cbFile, cbRead;
  
  f=fopen(fn,"rb");
  if (!f) return 0;
  
  fseek(f,0,SEEK_END);
  cbFile=ftell(f);
  fseek(f,0,SEEK_SET);
  
  *data=malloc(cbFile+1);
  
  cbRead=fread(*data,1,cbFile,f);
  
  fclose(f);
  
  if (cbRead<cbFile) {
    free(*data);
    *data=0;
    return 0;
  }
  
  ((char*)*data)[cbFile]=0;
  
  *cb=cbRead;
  
  return 1;
}

#ifdef _WIN32
#include <windows.h>

int file_exists(const char *fn) {
  int attr=GetFileAttributesA(fn);
  
  return (attr!=-1);
}

timestamp_t file_timestamp(const char *fn) {
  HANDLE h=INVALID_HANDLE_VALUE;
  FILETIME tModify;
  timestamp_t r=0;
  
  if (INVALID_HANDLE_VALUE==(h=CreateFileA(
    fn,GENERIC_READ,FILE_SHARE_READ,
    0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)))
    goto finalize;
  
  if (GetFileTime(h,0,0,&tModify)==0)
    goto finalize;
  
  r=((timestamp_t)tModify.dwHighDateTime<<32)|(tModify.dwLowDateTime);
  
  finalize:
  if (h!=INVALID_HANDLE_VALUE) {
    CloseHandle(h);
  }
  
  return r;
}


#else

#include <unistd.h>

int file_exists(const char *str) {
  return access(str,F_OK);
}

timestamp_t file_timestamp(const char *fn) {
#pragma message "WARNING: file_timestamp not yet implemented for this environment!"
  return 0;
}

#endif


struct Repository {
  char **path_v;
  size_t path_n;
};

Repository _repositories[REPOSITORY_COUNT]= {
  {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {0,0}, {0,0}, {0,0}
};


void vfs_registerPath(const char *path, int mask) {
  int i;
  
  for(i=0;i<REPOSITORY_COUNT;i++) if (mask&(1<<i)) {
    _repositories[i].path_v=(char**)realloc(
      (void*)_repositories[i].path_v,
      sizeof(char*)*(_repositories[i].path_n+1));
    _repositories[i].path_v[_repositories[i].path_n++]=strdup(path);
  }
  
}

char *vfs_locate(const char *fn, int mask) {
  char buf[512];
  char *p;
  int cc_fn, cc_path;
  
  if (file_exists(fn)) return strdup(fn);
  
  cc_fn=strlen(fn);
  memcpy(buf+512-cc_fn-2,fn,cc_fn+1);
  
  
  int i;
  size_t j;
  
  for(i=0;i<REPOSITORY_COUNT;i++) if (mask&(1<<i))
    for (j=0;j<_repositories[i].path_n;j++) {
      cc_path=strlen(_repositories[i].path_v[j]);
      p=buf+512-cc_fn-cc_path-2;
      memcpy(p,_repositories[i].path_v[j],cc_path);
      
      if (file_exists(p)) return strdup(p);
    }
  
  
  return 0;
}

size_t vfs_locate_all(const char *fn, int mask, char ***files) {
  char buf[512];
  char *p;
  int cc_fn, cc_path;
  size_t files_n=0;
  
  *files=0;
  
  
  if (file_exists(fn)) {
    *files=(char**)realloc((void*)*files,sizeof(char*)*(files_n+1));
    (*files)[files_n++]=strdup(fn);
  }
  
  cc_fn=strlen(fn);
  memcpy(buf+512-cc_fn-2,fn,cc_fn+1);
  
  int i;
  size_t j;
  
  for(i=0;i<REPOSITORY_COUNT;i++) if (mask&(1<<i))
    for (j=0;j<_repositories[i].path_n;j++) {
      cc_path=strlen(_repositories[i].path_v[j]);
      p=buf+512-cc_fn-cc_path-2;
      memcpy(p,_repositories[i].path_v[j],cc_path);
      
      if (file_exists(p)) {
        *files=(char**)realloc((void*)*files,sizeof(char*)*(files_n+1));
        (*files)[files_n++]=strdup(p); 
      }
    }
  
  if (!files_n) {
    free((void*)*files);
    *files=0;
  }
  
  return files_n;
}


#if DIYYMA_RC_NO_AUTODELETE||DIYYMA_RC_GLOBAL_LIST
RCObject **__rcobjects_v=0;
size_t     __rcobjects_n=0;
#endif
RCObject::RCObject() {
  #if DIYYMA_RC_NO_AUTODELETE||DIYYMA_RC_GLOBAL_LIST
    APPEND(__rcobjects,this);
    _refcount=1;
  #else
    _refcount=0;
  #endif
}
void RCObject::grab() { _refcount++; }
void RCObject::drop() { 
  #if (!DIYYMA_RC_NO_AUTODELETE)&&DIYYMA_RC_GLOBAL_LIST
    size_t idx;
    RCObject **pobj;
    if (_refcount==2) {
      goto delthis;
    } else if (_refcount<2) {
      LOG_WARNING(
        "WARNING: RCObject %.8p dropped without being grabbed!\n",
        this);
      goto delthis;
    } else _refcount--;
    
    return;
    
    delthis:
    FOREACH(idx,pobj,__rcobjects) if ((*pobj)==this) {
      __rcobjects_v[idx]=__rcobjects_v[--__rcobjects_n];
      break;
    }
    delete this;
    
  #else
    if (_refcount==1) delete this;
    else if (_refcount<1) {
      LOG_WARNING(
        "WARNING: RCObject %.8p dropped without being grabbed!\n",
        this);
      delete this;
    } else _refcount--;
  #endif
}
int RCObject::refcount() { return _refcount; }

#if DIYYMA_RC_NO_AUTODELETE

int RCObject::ClearDeprecated() {
  int idx;
  size_t n=0,n0=__rcobjects_n;
  
  while(n!=__rcobjects_n) {
    n=__rcobjects_n;
    
    for(idx=0;idx<__rcobjects_n;idx++) if (__rcobjects_v[idx]->refcount()==1) {
      #ifdef DIYYMA_DEBUG
        printf("dropping RCObject #%.4i (%.8p)\n",idx,__rcobjects_v[idx]);
      #endif
      fflush(stdout);
      __rcobjects_v[idx]->drop();
      __rcobjects_v[idx]=__rcobjects_v[--__rcobjects_n];
      idx--;
    }
  }
  #ifdef DIYYMA_DEBUG
    printf("dropped %i of %i deprecated RCObjects.\n",
      n0-__rcobjects_n,n0);
    fflush(stdout);
  #endif
  return n0-__rcobjects_n;
}

#endif

#if DIYYMA_RC_NO_AUTODELETE||DIYYMA_RC_GLOBAL_LIST

void RCObject::ListObjects() {
  size_t idx;
  RCObject **pobj;
  
  FOREACH(idx,pobj,__rcobjects) {
    printf("RCObject #%.4i (%.8p): ",idx,*pobj);
    fflush(stdout);
    printf("%4i\n",(*pobj)->refcount());
    fflush(stdout);
  }
}

#endif

double randf() {
  return (double)rand()/(double)RAND_MAX;
}


char *strdup(const char *str) {
  int l;
  char *res;
  
  l=strlen(str);
  
  res=(char*)malloc(l+1);
  strcpy(res,str);
  
  return res;
  
}

int strcmp_ic(const char *a, const char *b) {
  
  do {
    if (
      (*a!=*b)
      && ( 
        ((*a|0x20) != (*b|0x20))
        || ((*a|0x20)<'a') || ((*a|0x20)>'z')
        || ((*b|0x20)<'a') || ((*b|0x20)>'z')
        )) return (int)*a-(int)*b;
    a++,b++;
    
  } while(*a || *b);
  
  return 0;
}

LineScanner::LineScanner() :
  _data(0), _p(0), _end(0), _newLine(1) {
  
}

LineScanner::~LineScanner() { 
  
}

LineScanner *LineScanner::_temp=0;

LineScanner *LineScanner::GetTemporary() {
  LineScanner *res;
  if (!_temp) {
    _temp=new LineScanner();
    _temp->grab();
  }
  
  if (_temp->refcount()>1) {
    res=new LineScanner();
  } else {
    res=_temp;
  }
  res->grab();
  
  return res;
}

void LineScanner::operator=(const char *data) {
  _data=data;
  _p=data;
  _end=_data+strlen(data);
}
void LineScanner::assign(const char *data, size_t cb) {
  _data=data;
  _p=data;
  _end=_data+cb;
}
void LineScanner::assign(const char *data, const char *e) {
  _data=data;
  _p=data;
  _end=e;
}

size_t LineScanner::tell() {
  return (size_t)_p-(size_t)_data;
}

size_t LineScanner::seek(int offset, int origin) {
  int p;
  switch(origin) {
    default:
    case 0: p=offset; break;
    case 1: p=(int)_p-(int)_data+offset; break;
    case 2: p=(int)_end-(int)_data+offset; break;
  }
  if (p<0) p=0; else if (p>(int)_end-(int)_data) p=(int)_end-(int)_data;
  
  _p=_data+p;
  
  return (size_t)p;
}

int LineScanner::seekNewLine() {
  _newLine=1;
  
  while((_p<_end)&&(*_p!='\n')&&(*_p!='\r')) _p++;
  if (_p>=_end) return 0;
  
  while((_p<_end)&&((*_p=='\n')||(*_p=='\r'))) _p++;
  if (_p>=_end) return 0;
  return 1;
}

int LineScanner::getString(SubString *res) {
  int longstring=0, esc=0, doesc=flags&LINESCANNER_USE_ESCAPE;
  char quote;
  _newLine=0;
  
  res->ptr=0;
  res->length=0;
  
  while (_p<_end) {
    if (longstring) {
      if (esc) esc=0;
      else if (doesc && (*_p=='\\')) {
        esc=true;
      } else if (*_p==quote) { 
        res->length=(size_t)_p-(size_t)res->ptr;
        _p++;
        return 1;
      }
    } else if (*_p > ' ') { 
      if (!res->ptr) {
        res->ptr=_p;
        if ((*_p=='"')||(*_p=='\'')) {
          longstring=1;
          quote=*_p;
        }
      }
    } else if (res->ptr) {
      res->length=(size_t)_p-(size_t)res->ptr;
      while((_p<_end)&&((*_p=='\n')||(*_p=='\r'))) {
        _newLine=1;
        _p++;
      }
      return 1;
    }
    _p++;
  }
  if (res->ptr) { 
    res->length=(size_t)_end-(size_t)res->ptr;
    return 1;
  }
  return 0;
}

int LineScanner::getLnString(SubString *res) {
  if (_newLine) return 0;
  while((_p<_end)&&(*_p<=' ')) {
    if ((*_p=='\n') || (*_p=='\r')) {
      res->ptr=0;
      res->length=0;
      while((_p<_end)&&((*_p=='\n') || (*_p=='\r'))) _p++;
      _newLine=1;
      return 0;
    }
    _p++;
  }
  
  return getString(res);
}

int LineScanner::getLnFirstString(SubString *res) {
  if (!_newLine && !seekNewLine()) {
    res->ptr=0;
    res->length=0;
    return 0;
  }
  
  return getString(res);
}

int LineScanner::getLnRemainder(SubString *res) {
  if (_p>=_end) {
    res->ptr=0;
    res->length=0;
    return 0;
  }
  res->ptr=_p;
  while((_p<_end)&&(*_p!='\r')&&(*_p!='\n')) _p++;
  res->length=(size_t)_p-(size_t)res->ptr;
  return 1;
}

int LineScanner::getLong(long *pres, int nonl) {
  char buf[32];
  long res;
  char *endptr=0;
  
  SubString str;
  if (nonl) {
    if (!getLnString(&str)) return 0;
  } else {
    if (!getString(&str)) return 0;
  }
  if (str.length>31) {
    memcpy(buf,str.ptr,31);
    buf[31]=0;
  } else {
    memcpy(buf,str.ptr,str.length);
    buf[str.length]=0;
  }
  
  res=strtol(buf,&endptr,0);
  
  if (endptr<=buf) 
    return 0;
  
  *pres=res;
  return 1;
}

int LineScanner::getULong(unsigned long *pres, int nonl) {
  char buf[32];
  unsigned long res;
  char *endptr=0;
  
  SubString str;
  if (nonl) {
    if (!getLnString(&str)) return 0;
  } else {
    if (!getString(&str)) return 0;
  }
  if (str.length>31) {
    memcpy(buf,str.ptr,31);
    buf[31]=0;
  } else {
    memcpy(buf,str.ptr,str.length);
    buf[str.length]=0;
  }
  
  res=strtoul(buf,&endptr,0);
  
  if (endptr<=buf) 
    return 0;
  
  *pres=res;
  return 1;
}

int LineScanner::getDouble(double *pres, int nonl) {
  char buf[32];
  double res;
  char *endptr=0;
  
  SubString str;
  if (nonl) {
    if (!getLnString(&str)) return 0;
  } else {
    if (!getString(&str)) return 0;
  }
  if (str.length>31) {
    memcpy(buf,str.ptr,31);
    buf[31]=0;
  } else {
    memcpy(buf,str.ptr,str.length);
    buf[str.length]=0;
  }
  
  res=strtod(buf,&endptr);
  
  if (endptr<=buf) 
    return 0;
  
  *pres=res;
  return 1;
}


int LineScanner::getFloat(float *pres, int nonl) {
  char buf[32];
  double res;
  char *endptr=0;
  
  SubString str;
  if (nonl) {
    if (!getLnString(&str)) return 0;
  } else {
    if (!getString(&str)) return 0;
  }
  if (str.length>31) {
    memcpy(buf,str.ptr,31);
    buf[31]=0;
  } else {
    memcpy(buf,str.ptr,str.length);
    buf[str.length]=0;
  }
  
  res=strtod(buf,&endptr);
  
  if (endptr<=buf) 
    return 0;
  
  *pres=(float)res;
  return 1;
}

int LineScanner::getOBJFaceCorner(long *pres, int nonl) {
  int i;
  const char *p, *e;
  
  SubString str;
  if (nonl) {
    if (!getLnString(&str)) return 0;
  } else {
    if (!getString(&str)) return 0;
  }
  
  p=str.ptr;
  e=str.ptr+str.length;
  for(i=0;i<3;i++) {
    pres[i]=0;
    while (p<e) {
      if ((*p>='0') && (*p<='9')) {
        pres[i]=pres[i]*10+(*p-'0');
      } else if (*p=='/') {
        p++;
        break;
      } else {
        return 0;
      }
      p++;
    }
    pres[i]--;
  }
  
  if (p<e)
    return 0;
  
  return 1;
  
}