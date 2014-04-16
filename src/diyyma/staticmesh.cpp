
/** \file staticmesh.cpp
  * \author Peter Wagener
  * \brief Implementation of static mesh objects.
  *
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "GL/glew.h"

#include "diyyma/staticmesh.h"


#include "diyyma/util.h"
#include "diyyma/xco.h"

StaticMesh::StaticMesh() {
  memset(_buffers,0,sizeof(_buffers));
  _vertexCount=0;
}

StaticMesh::~StaticMesh() {
  int i;
  for(i=0;i<MAX_ARRAY_BUFFERS;i++) if (_buffers[i].handle)
    glDeleteBuffers(1,&_buffers[i].handle);
}

/** \brief Crude loader for wavefront (OBJ) meshes.
  * 
  * \todo implement groups and material loading
  */

class OBJLoader {
  private:
    char *p;
    
    char *q;
    size_t ql;
    double qf;
    long int qi;
    
    float *vertices;
    float *normals;
    float *texCoords;
    int   *triangles;
    
    size_t nVertices;
    size_t nNormals;
    size_t nTexCoords;
    size_t nTriangles;
    
    int i;
    float buf[6];
    int bufi[9];
    
    int newline() {
      while((*p) && ((*p!='\r')&&(*p!='\n'))) p++;
      while((*p) && ((*p=='\r')||(*p=='\n'))) p++;
      return *p!=0;
    }
    
    int lnstring() {
      while((*p)&&((*p!='\r')&&(*p!='\n'))&&(*p<=' ')) p++;
      if (!*p || (*p=='\r') ||(*p=='\n')) return 0;
      q=p;
      while(*p>' ') p++;
      ql=(size_t)p-(size_t)q;
      return 1;
    }
    
    int lnfloat(float *v) {
      char *e;
      char t;
      
      while((*p)&&((*p!='\r')&&(*p!='\n'))&&
      !(((*p>='0')&&(*p<='9')) || (*p=='.') || (*p=='-'))) p++;
      if (!*p || (*p=='\r') ||(*p=='\n')) return 0;
      
      q=p;
      
      while(*p 
        && (((*p>='0')&&(*p<='9')) || (*p=='.') || (*p=='-')|| (((*p)|0x20)=='e')))
        p++;
      t=*p;
      *p=0;
      
      qf=strtod(q,&e);
      *p=t;
      if (e>q) {
        p=e;
        if (v) *v=qf;
        return 1;
      }
      
      return 0;
    }
    
    int lnint(int *v) {
      char *e;
      char t;
      
      while((*p)&&((*p!='\r')&&(*p!='\n'))&&
      !(((*p>='0')&&(*p<='9')) || (*p=='-'))) p++;
      if (!*p || (*p=='\r') ||(*p=='\n')) return 0;
      
      q=p;
      
      while(*p 
        && ((*p>='0')&&(*p<='9')))
        p++;
      t=*p;
      *p=0;
      
      qi=strtol(q,&e,10);
      *p=t;
      if (e>q) {
        p=e;
        if (v) *v=qi;
        return 1;
      }
      
      return 0;
    }
    
    int lnfacecorner(int idx) {
      int i;
      idx*=3;
      if (!lnint(&bufi[idx])) return 0;
      if ((bufi[idx]<1) ||(bufi[idx]>nVertices)) return 0;
      bufi[idx+1]=0;
      bufi[idx+2]=0;
      
      for(int i=0;i<2;i++) {
        if (*p!='/') break;
        p++;
        if (*p!='/')
          if (!lnint(&bufi[idx+i+1])) return 0;
      }
      
      
      if ((bufi[idx+1]<0) || (bufi[idx+1]>nTexCoords)) return 0;
      if ((bufi[idx+2]<0) || (bufi[idx+2]>nNormals)) return 0;
      
      return 1;
    }
  
  public:
    OBJLoader() : 
      nVertices(0), nNormals(0), nTexCoords(0), nTriangles(0),
      vertices(0), normals(0), texCoords(0), triangles(0)
       { }
    ~OBJLoader() {
      if (nVertices) free((void*)vertices);
      if (nNormals) free((void*)normals);
      if (nTexCoords) free((void*)texCoords);
      if (nTriangles) free((void*)triangles);
    }
    void parse(char *code) {
      p=code;
      while(lnstring()) {
        if (strncmp(q,"v",ql)==0) {
          if (!(lnfloat(buf)&&lnfloat(buf+1)&&lnfloat(buf+2))) goto next;
          
          vertices=(float*)realloc((void*)vertices,3*sizeof(float)*(nVertices+1));
          memcpy(vertices+nVertices*3,buf,3*sizeof(float));
          nVertices++;
        } else if (strncmp(q,"n",ql)==0 || strncmp(q,"vn",ql)==0) {
          if (!(lnfloat(buf)&&lnfloat(buf+1)&&lnfloat(buf+2))) goto next;
          
          normals=(float*)realloc((void*)normals,3*sizeof(float)*(nNormals+1));
          memcpy(normals+nNormals*3,buf,3*sizeof(float));
          nNormals++;
        } else if (strncmp(q,"n",ql)==0) {
          if (!(lnfloat(buf)&&lnfloat(buf+1)&&lnfloat(buf+2))) goto next;
          
          normals=(float*)realloc((void*)normals,3*sizeof(float)*(nNormals+1));
          memcpy(normals+nNormals*3,buf,3*sizeof(float));
          nNormals++;
        } else if (strncmp(q,"vt",ql)==0) {
          if (!(lnfloat(buf)&&lnfloat(buf+1))) goto next;
          
          texCoords=(float*)realloc((void*)texCoords,2*sizeof(float)*(nTexCoords+1));
          memcpy(texCoords+nTexCoords*2,buf,2*sizeof(float));
          nTexCoords++;
        } else if (strncmp(q,"f",ql)==0) {
          if (!(lnfacecorner(0)&&lnfacecorner(1))) goto next;
          
          while(lnfacecorner(2)) {
            triangles=(int*)realloc((void*)triangles,9*sizeof(int)*(nTriangles+1));
            memcpy(triangles+nTriangles*9,bufi,9*sizeof(int));
            nTriangles++;
            
            memcpy(bufi+3,bufi+6,3*sizeof(int));
            
          }
		}
        
        next:
        newline();
      }
      
    }
    
    int build(ArrayBuffer *bufv) {
      int cr;
      float *buf;
      float *pbuf;
      int i;
      int nbuf=0;
      
      if (nTriangles<1) return nbuf;
      
      cr=nTriangles*3;
      buf=(float*)malloc(sizeof(float)*cr*3);
      
      for(pbuf=buf,i=0;i<cr;i++,pbuf+=3) {
        memcpy(pbuf,vertices+(triangles[i*3]-1)*3,3*sizeof(float));
      }
      
      if (!bufv->handle) {
        glGenBuffers(1,&bufv->handle);
      }
      bufv->index    =BUFIDX_VERTICES;
      bufv->type     =GL_FLOAT;
      bufv->dimension=3;
      glBindBuffer(GL_ARRAY_BUFFER,bufv->handle);
      glBufferData(GL_ARRAY_BUFFER,cr*3*sizeof(float),buf,GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER,0);
      
      nbuf++;
      if (nNormals) {
        bufv++;
        
        for(pbuf=buf,i=0;i<cr;i++,pbuf+=3)
          memcpy(pbuf,normals+(triangles[i*3+2]-1)*3,3*sizeof(float));
          
        if (!bufv->handle) {
          glGenBuffers(1,&bufv->handle);
        }
        bufv->index    =BUFIDX_NORMALS;
        bufv->type     =GL_FLOAT;
        bufv->dimension=3;
        
        glBindBuffer(GL_ARRAY_BUFFER,bufv->handle);
        glBufferData(GL_ARRAY_BUFFER,cr*3*sizeof(float),buf,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        
        nbuf++;
      }
      
      if (nTexCoords) {
        bufv++;
        
        for(pbuf=buf,i=0;i<cr;i++,pbuf+=2)
          memcpy(pbuf,texCoords+(triangles[i*3+1]-1)*2,2*sizeof(float));
          
        if (!bufv->handle) {
          glGenBuffers(1,&bufv->handle);
        }
        bufv->index    =BUFIDX_TEXCOORDS;
        bufv->type     =GL_FLOAT;
        bufv->dimension=2;
        
        glBindBuffer(GL_ARRAY_BUFFER,bufv->handle);
        glBufferData(GL_ARRAY_BUFFER,cr*2*sizeof(float),buf,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        
        
        nbuf++;
      }
      
      free((void*)buf);
      
      return nbuf;
      
    }
    
    int vertexCount() {
      return nTriangles*3;
    }
    
};

void StaticMesh::loadOBJ(char *code) {
  clear();
  
  OBJLoader *loader=new OBJLoader();
  
  loader->parse(code);
  
  _vertexCount=loader->vertexCount();
  loader->build(_buffers);
  
  delete loader;
}

int StaticMesh::loadDOFFile(const char *fn_in) {
  
  XCOReaderContext *xco=0;
  DOFArray array_head;
  void *array_data;
  size_t array_cb_record;
  size_t array_length;
  
  int idx_array=0;
  
  void *data=0;
  size_t cb;
  char *fn;
  int r=0;
  
  clear();
  
  if (!(fn=vfs_locate(fn_in,REPOSITORY_MASK_MESH))) {
    LOG_WARNING(
      "WARNING: unable to find static mesh file '%s'\n",
      fn_in)
    goto finalize;
  }
  
  if (!readFile(fn,&data,&cb)) {
    LOG_WARNING(
      "WARNING: unable to load static mesh file '%s'\n",
      fn)
    goto finalize;
  }
  
  if (!xcor_create(&xco,data,cb)) {
    LOG_WARNING(
      "WARNING: unable to open XCO file '%s'\n",
      fn)
    goto finalize;
  }
  
  if (!xcor_chunk_sub(xco) || (xco->head->id!=XCO_DIYYMA_OBJECT) 
  || !xcor_chunk_sub(xco)) {
    LOG_WARNING(
      "WARNING: XCO file '%s' does not contain a diyyma object\n",
      fn)
    goto finalize;
  }
  
  do { switch (xco->head->id) {
    case XCO_DIYYMA_OBJECT_ARRAY:
      if (xcor_data_remain(xco)<sizeof(array_head)) {
        LOG_WARNING(
          "WARNING: XCO file '%s' contains invalid array head\n",
          fn)
        goto next;
      }
      xcor_data_read(xco,array_head);
      
      if (idx_array>=MAX_ARRAY_BUFFERS) {
        LOG_WARNING(
          "WARNING: XCO file '%s' contains too many arrays (max: %i)\n",
          fn, MAX_ARRAY_BUFFERS)
        goto next;
      }
      
      if (xcor_data_remain(xco)!=array_head.cbData) {
        LOG_WARNING(
          "WARNING: XCO file '%s' contains invalid array: byte count mismatch\n",
          fn)
        goto next;
      }
      
      switch(array_head.type) {
        case GL_BYTE: array_cb_record=1; break;
        case GL_UNSIGNED_BYTE: array_cb_record=1; break;
        case GL_SHORT: array_cb_record=2; break;
        case GL_UNSIGNED_SHORT: array_cb_record=2; break;
        case GL_INT: array_cb_record=4; break;
        case GL_UNSIGNED_INT: array_cb_record=4; break;
        case GL_FLOAT: array_cb_record=4; break;
        case GL_DOUBLE: array_cb_record=8; break;
        default:
          LOG_WARNING(
            "WARNING: XCO file '%s' contains invalid array: "
            " invalid array type: %lu\n",
            fn,array_head.type)
          goto next;
      }
      
      array_cb_record*=array_head.dimension;
      array_length    =array_head.cbData/array_cb_record;
      
      if (_vertexCount && (array_length!=_vertexCount)) {
        LOG_WARNING(
          "WARNING: XCO file '%s' contains invalid array: "
          " array length mismatch\n",
          fn)
        goto next;
      }
      
      if (array_head.cbData!=array_length*array_cb_record) {
        
        LOG_WARNING(
          "WARNING: XCO file '%s' contains invalid array: "
          " badly aligned data block\n",
          fn)
        goto next;
      }
      
      if (array_head.cbData<1) goto next;
      
      glGenBuffers(1,&_buffers[idx_array].handle);
      _buffers[idx_array].index=array_head.index;
      _buffers[idx_array].type=array_head.type;
      _buffers[idx_array].dimension=array_head.dimension;
      
      glBindBuffer(GL_ARRAY_BUFFER,_buffers[idx_array].handle);
      glBufferData(GL_ARRAY_BUFFER,array_head.cbData,xco->p,GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER,0);
      
      idx_array++;
      
      break;
    default:
      break;
  } next: ;} while(xcor_chunk_next(xco));
  
  
  r=1;
  
  finalize:
  
  if (data) free(data);
  if (xco) xcor_close(&xco);
  
  return r;
}

int StaticMesh::loadOBJFile(const char *fn_in) {
  
  void *data;
  size_t cb;
  char *fn;
  
  if (!(fn=vfs_locate(fn_in,REPOSITORY_MASK_MESH))) {
    LOG_WARNING(
      "WARNING: unable to find static mesh file '%s'\n",
      fn_in)
    return 0;
  }
  
  if (!readFile(fn,&data,&cb)) {
    LOG_WARNING(
      "WARNING: unable to load static mesh file '%s'\n",
      fn)
    return 0;
  }
  
  loadOBJ((char*)data);
  free(data);
  
  return 1;
  
}

void StaticMesh::bind() {
  int i;
  for(i=0;i<MAX_ARRAY_BUFFERS;i++) if (_buffers[i].handle) {
    glBindBuffer(GL_ARRAY_BUFFER,_buffers[i].handle);
    glVertexAttribPointer(
      _buffers[i].index,_buffers[i].dimension,_buffers[i].type,
      0,0,0);
    glEnableVertexAttribArray(_buffers[i].index);
    glBindBuffer(GL_ARRAY_BUFFER,0);
  }
}

void StaticMesh::unbind() {
  int i;
  for(i=0;i<MAX_ARRAY_BUFFERS;i++) if (_buffers[i].handle) {
    glDisableVertexAttribArray(_buffers[i].index);
  }
}

void StaticMesh::send() {
  glDrawArrays(GL_TRIANGLES,0,_vertexCount);
}

void StaticMesh::clear() {
  int i;
  _vertexCount=0;
  for(i=0;i<MAX_ARRAY_BUFFERS;i++) if (_buffers[i].handle) {
    glDeleteBuffers(1,&_buffers[i].handle);
  }
  memset(_buffers,0,sizeof(_buffers));
}

void StaticMesh::reload() {
  if (_filename) {
    switch(_fileFormat) {
      case 0: loadOBJFile(_filename); break;
      case 1: loadDOFFile(_filename); break;
    }
  }
}

timestamp_t StaticMesh::filesTimestamp() {
  if (!_filename) return 0;
  return file_timestamp(_filename);
}