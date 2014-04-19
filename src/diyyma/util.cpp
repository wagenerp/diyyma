
/** \file shader.cpp
  * \author Peter Wagener
  * \brief Miscellaneous utility method implementations.
  *
  */
#include <stdio.h>
#include <stdlib.h>
#include "diyyma/util.h"
#include <string.h>

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

char *strdup(const char *str) {
  int l;
  char *res;
  
  l=strlen(str);
  
  res=(char*)malloc(l+1);
  strcpy(res,str);
  
  return res;
  
}

#ifdef _WIN32
#include <windows.h>

int file_exists(const char *fn) {
  int attr=GetFileAttributesA(fn);
  
  return (attr!=0xffffffff);
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

Repository _repositories[REPOSITORY_COUNT]= {{0,0}, {0,0}, {0,0}};


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
  
  
  int i,j;
  
  for(i=0;i<REPOSITORY_COUNT;i++) if (mask&(1<<i))
    for (j=0;j<_repositories[i].path_n;j++) {
      cc_path=strlen(_repositories[i].path_v[j]);
      p=buf+512-cc_fn-cc_path-2;
      memcpy(p,_repositories[i].path_v[j],cc_path);
      
      if (file_exists(p)) return strdup(p);
    }
  
  
  return 0;
}

double randf() {
  return (double)rand()/(double)RAND_MAX;
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