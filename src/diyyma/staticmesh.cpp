
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

StaticMesh::StaticMesh(): _filename(0) {
  memset(_buffers,0,sizeof(_buffers));
  _vertexCount=0;
  ARRAY_INIT(_materials);
}

StaticMesh::~StaticMesh() {
  clear();
  /*
  int i;
  for(i=0;i<MAX_ARRAY_BUFFERS;i++) if (_buffers[i].handle)
    glDeleteBuffers(1,&_buffers[i].handle);
  */
}

size_t StaticMesh::materialCount() { 
  return _materials_n;
}
const MaterialSlice &StaticMesh::material(int idx) {
  return _materials_v[idx];
}

union face_t {
  struct {
    long v0[3];
    long v1[3];
    long v2[3];
    long imat;
    long omat;
  };
  long v[11];
  
  long &operator[](int idx) { return v[idx]; }
};

void StaticMesh::loadOBJ(char *code, const char *outputDOF) {
  LineScanner *scanner;
  SubString str;
  Vector3f bv;
  MaterialLibrary *mtlib=0;
  int imat=-1;
  SubString smat;
  MaterialSlice mat;
  face_t face;
  AssetRegistry<MaterialLibrary> *mtl;
  void *buffer=0;
  Vector3f *pvec3;
  Vector2f *pvec2;
  size_t idx, idxo;
  int i;
  ArrayBuffer *bufv;
  
  XCOWriterContext *xco=0;
  DOFArray          dof_array_head;
  FILE             *fDOF;
  
  smat.ptr=0;
  ARRAY(Vector3f,vertices);
  ARRAY(Vector3f,normals);
  ARRAY(Vector2f,texcoords);
  ARRAY(Vector3f,tangents);
  ARRAY(Vector3f,binormals);
  ARRAY(face_t,faces);
  
  // cleanup previously loaded data
  clear();
  
  // init temporary structures
  ARRAY_INIT(vertices);
  ARRAY_INIT(normals);
  ARRAY_INIT(texcoords);
  ARRAY_INIT(tangents);
  ARRAY_INIT(binormals);
  ARRAY_INIT(faces);  
  mtl=reg_mtl();
  
  // scan the obj file
  scanner=new LineScanner();
  scanner->grab();
  *scanner=code;
  
  while(scanner->getLnFirstString(&str)) {
    if (str=="v") {
      if (!scanner->getFloat(&bv.x,1)) continue;
      if (!scanner->getFloat(&bv.y,1)) continue;
      if (!scanner->getFloat(&bv.z,1)) continue;
      APPEND(vertices,bv);
    } else if ((str=="n")||(str=="vn")) {
      if (!scanner->getFloat(&bv.x,1)) continue;
      if (!scanner->getFloat(&bv.y,1)) continue;
      if (!scanner->getFloat(&bv.z,1)) continue;
      APPEND(normals,bv);
    } else if ((str=="#b")||(str=="#vb")) {
      if (!scanner->getFloat(&bv.x,1)) continue;
      if (!scanner->getFloat(&bv.y,1)) continue;
      if (!scanner->getFloat(&bv.z,1)) continue;
      APPEND(binormals,bv);
    } else if ((str=="#t")||(str=="#vt")) {
      if (!scanner->getFloat(&bv.x,1)) continue;
      if (!scanner->getFloat(&bv.y,1)) continue;
      if (!scanner->getFloat(&bv.z,1)) continue;
      APPEND(tangents,bv);
    } else if (str=="vt") {
      if (!scanner->getFloat(&bv.x,1)) continue;
      if (!scanner->getFloat(&bv.y,1)) continue;
      APPEND(texcoords,*(Vector2f*)&bv);
    } else if (str=="mtllib") {
      if (!scanner->getLnString(&str)) continue;
      if (mtlib) mtlib->drop();
      mtlib=mtl->get(str);
      if (mtlib) mtlib->grab();
    } else if (str=="usemtl") {
      scanner->getLnString(&smat);
    } else if (str=="f") {
      if (vertices_n<1) continue;
      if (!scanner->getOBJFaceCorner(face.v0,1)) continue;
      if (!scanner->getOBJFaceCorner(face.v1,1)) continue;
      if (!scanner->getOBJFaceCorner(face.v2,1)) continue;
      if ((face[0]<0)||(face[0]>=vertices_n )) face[0]= 0;
      if ((face[3]<0)||(face[3]>=vertices_n )) face[3]= 0;
      if ((face[1]<0)||(face[1]>=texcoords_n)) face[1]=-1;
      if ((face[4]<0)||(face[4]>=texcoords_n)) face[4]=-1;
      if ((face[2]<0)||(face[2]>=normals_n  )) face[2]=-1;
      if ((face[5]<0)||(face[5]>=normals_n  )) face[5]=-1;
      if ((imat<0) || (smat.ptr)) {
        if (smat.ptr&&mtlib) {
          mat.mat=mtlib->get(smat,0);
          if (!mat.mat) {
            LOG_WARNING(
              "WARNING: Material '%.*s' not found\n",
              smat.length,smat.ptr);
            mat.mat=new Material();
          }
        } else {
          mat.mat=new Material();
        }
        mat.mat->grab();
        mat.vertexCount=0;
        APPEND(_materials,mat);
        imat=_materials_n-1;
        smat.ptr=0;
        smat.length=0;
      }
      
      face[9]=imat;
      
      do {
        if ((face[6]<0)||(face[6]>=vertices_n )) face[6]= 0;
        if ((face[7]<0)||(face[7]>=texcoords_n)) face[7]=-1;
        if ((face[8]<0)||(face[8]>=normals_n  )) face[8]=-1;
        face[10]=_materials_v[imat].vertexCount;
        APPEND(faces,face);
        _materials_v[imat].vertexCount+=3;
        memcpy(face.v1,face.v2,sizeof(long)*3);
      } while(scanner->getOBJFaceCorner(face.v2,1));
      
    }
  }
  
  if (!_materials_n) goto cleanup;
  
  // compute per-material vertex offsets.
  _materials_v[0].vertexOffset=0;
  for(idx=1;idx<_materials_n;idx++)
    _materials_v[idx].vertexOffset=
      _materials_v[idx-1].vertexOffset+
      _materials_v[idx-1].vertexCount;
  // allocate enough buffer space to assemble final vertex buffers.
  // That is, three vectors per face
  _vertexCount=faces_n*3;
  buffer=malloc(sizeof(Vector3f)*_vertexCount);
  
  // assemble and send array buffers
  bufv=_buffers;
  
  if (outputDOF) {
    if (!xcow_create(&xco)) {
      xco=0;
      LOG_WARNING(
        "WARNING: unable to create XCO writer context!");
      goto vertices;
    }
    
    xcow_chunk_new(xco,XCO_DIYYMA_OBJECT);
    
  }
  
  
  
  // vertices
  vertices:
  for(idx=0;idx<faces_n;idx++) {
    idxo=faces_v[idx].omat+_materials_v[faces_v[idx].imat].vertexOffset;
    for(i=0;i<3;i++)
      ((Vector3f*)buffer)[idxo+i]=vertices_v[faces_v[idx][i*3]];
  }
  if (!bufv->handle) glGenBuffers(1,&bufv->handle);
  bufv->index    =BUFIDX_VERTICES;
  bufv->type     =GL_FLOAT;
  bufv->dimension=3;
  glBindBuffer(GL_ARRAY_BUFFER,bufv->handle);
  glBufferData(GL_ARRAY_BUFFER,_vertexCount*3*sizeof(float),buffer,GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  
  if (xco) {
    xcow_chunk_new(xco,XCO_DIYYMA_OBJECT_ARRAY);
    
    dof_array_head.index=bufv->index;
    dof_array_head.type =bufv->type;
    dof_array_head.dimension=bufv->dimension;
    dof_array_head.cbData=sizeof(float)*_vertexCount*bufv->dimension;
    
    xcow_data_write(xco,dof_array_head);
    xcow_data_writearr(xco,buffer,dof_array_head.cbData);
    
    xcow_chunk_close(xco);
  
  }
  
  bufv++;
  
  
  normals:
  if (!normals_n) goto texcoords;
  for(idx=0;idx<faces_n;idx++) {
    idxo=faces_v[idx].omat+_materials_v[faces_v[idx].imat].vertexOffset;
    for(i=0;i<3;i++) if (faces_v[idx][i*3+2]>-1)
      ((Vector3f*)buffer)[idxo+i]=normals_v[faces_v[idx][i*3+2]];
  }
  if (!bufv->handle) glGenBuffers(1,&bufv->handle);
  bufv->index    =BUFIDX_NORMALS;
  bufv->type     =GL_FLOAT;
  bufv->dimension=3;
  glBindBuffer(GL_ARRAY_BUFFER,bufv->handle);
  glBufferData(GL_ARRAY_BUFFER,_vertexCount*3*sizeof(float),buffer,GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  
  if (xco) {
    xcow_chunk_new(xco,XCO_DIYYMA_OBJECT_ARRAY);
    
    dof_array_head.index=bufv->index;
    dof_array_head.type =bufv->type;
    dof_array_head.dimension=bufv->dimension;
    dof_array_head.cbData=sizeof(float)*_vertexCount*bufv->dimension;
    
    xcow_data_write(xco,dof_array_head);
    xcow_data_writearr(xco,buffer,dof_array_head.cbData);
    
    xcow_chunk_close(xco);
  
  }
  
  bufv++;
  
  binormals:
  if (binormals_n<normals_n) goto tangents;
  for(idx=0;idx<faces_n;idx++) {
    idxo=faces_v[idx].omat+_materials_v[faces_v[idx].imat].vertexOffset;
    for(i=0;i<3;i++) if (faces_v[idx][i*3+2]>-1)
      ((Vector3f*)buffer)[idxo+i]=binormals_v[faces_v[idx][i*3+2]];
  }
  if (!bufv->handle) glGenBuffers(1,&bufv->handle);
  bufv->index    =BUFIDX_BINORMALS;
  bufv->type     =GL_FLOAT;
  bufv->dimension=3;
  glBindBuffer(GL_ARRAY_BUFFER,bufv->handle);
  glBufferData(GL_ARRAY_BUFFER,_vertexCount*3*sizeof(float),buffer,GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  
  if (xco) {
    xcow_chunk_new(xco,XCO_DIYYMA_OBJECT_ARRAY);
    
    dof_array_head.index=bufv->index;
    dof_array_head.type =bufv->type;
    dof_array_head.dimension=bufv->dimension;
    dof_array_head.cbData=sizeof(float)*_vertexCount*bufv->dimension;
    
    xcow_data_write(xco,dof_array_head);
    xcow_data_writearr(xco,buffer,dof_array_head.cbData);
    
    xcow_chunk_close(xco);
  
  }
  
  bufv++;
  
  tangents:
  if (tangents_n<normals_n) goto texcoords;
  for(idx=0;idx<faces_n;idx++) {
    idxo=faces_v[idx].omat+_materials_v[faces_v[idx].imat].vertexOffset;
    for(i=0;i<3;i++) if (faces_v[idx][i*3+2]>-1)
      ((Vector3f*)buffer)[idxo+i]=tangents_v[faces_v[idx][i*3+2]];
  }
  if (!bufv->handle) glGenBuffers(1,&bufv->handle);
  bufv->index    =BUFIDX_TANGENTS;
  bufv->type     =GL_FLOAT;
  bufv->dimension=3;
  glBindBuffer(GL_ARRAY_BUFFER,bufv->handle);
  glBufferData(GL_ARRAY_BUFFER,_vertexCount*3*sizeof(float),buffer,GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  
  if (xco) {
    xcow_chunk_new(xco,XCO_DIYYMA_OBJECT_ARRAY);
    
    dof_array_head.index=bufv->index;
    dof_array_head.type =bufv->type;
    dof_array_head.dimension=bufv->dimension;
    dof_array_head.cbData=sizeof(float)*_vertexCount*bufv->dimension;
    
    xcow_data_write(xco,dof_array_head);
    xcow_data_writearr(xco,buffer,dof_array_head.cbData);
    
    xcow_chunk_close(xco);
  
  }
  
  bufv++;
  
  
  texcoords:
  if (!texcoords_n) goto done_buffers;
  for(idx=0;idx<faces_n;idx++) {
    idxo=faces_v[idx].omat+_materials_v[faces_v[idx].imat].vertexOffset;
    for(i=0;i<3;i++) if (faces_v[idx][i*3+1]>-1)
      ((Vector2f*)buffer)[idxo+i]=texcoords_v[faces_v[idx][i*3+1]];
  }
  if (!bufv->handle) glGenBuffers(1,&bufv->handle);
  bufv->index    =BUFIDX_TEXCOORDS;
  bufv->type     =GL_FLOAT;
  bufv->dimension=2;
  glBindBuffer(GL_ARRAY_BUFFER,bufv->handle);
  glBufferData(GL_ARRAY_BUFFER,_vertexCount*2*sizeof(float),buffer,GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  
  if (xco) {
    xcow_chunk_new(xco,XCO_DIYYMA_OBJECT_ARRAY);
    
    dof_array_head.index=bufv->index;
    dof_array_head.type =bufv->type;
    dof_array_head.dimension=bufv->dimension;
    dof_array_head.cbData=sizeof(float)*_vertexCount*bufv->dimension;
    
    xcow_data_write(xco,dof_array_head);
    xcow_data_writearr(xco,buffer,dof_array_head.cbData);
    
    xcow_chunk_close(xco);
    
  }
  
  bufv++;
  
  done_buffers:
  free(buffer);
  
  
  if (xco) {
    for(idx=0;idx<_materials_n;idx++) {
      xcow_chunk_new(xco,XCO_DIYYMA_OBJECT_MATERIAL_SLICE);
      xcow_chunk_new(xco,0x0001);
      xcow_data_write(xco,(int32_t)_materials_v[idx].vertexCount);
      xcow_data_write(xco,(int32_t)_materials_v[idx].vertexOffset);
      xcow_chunk_close(xco);
      
      _materials_v[idx].mat->saveXCO(xco);
      
      xcow_chunk_close(xco);
    }
    
    xcow_finalize(xco);
    
    fDOF=fopen(outputDOF,"wb");
    
    if (!fDOF) {
      LOG_WARNING(
        "WARNING: unable to open output DOF file '%s'!",
        outputDOF);
      goto cleanup;
    }
    
    fwrite(xco->data,1,(size_t)xco->p-(size_t)xco->data,fDOF);
    
    fclose(fDOF);
    
  }
  
  
  // cleanup
  
  cleanup:
  
  scanner->drop();
  ARRAY_DESTROY(vertices);
  ARRAY_DESTROY(normals);
  ARRAY_DESTROY(texcoords);
  ARRAY_DESTROY(tangents);
  ARRAY_DESTROY(binormals);
  ARRAY_DESTROY(faces);
  if (mtlib) mtlib->drop();
  
}

int StaticMesh::loadDOFFile(const char *fn_in) {
  
  XCOReaderContext *xco=0;
  DOFArray array_head;
  size_t array_cb_record;
  int array_length;
  
  int idx_array=0;
  
  void *data=0;
  size_t cb;
  char *fn;
  int r=0;
  
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
  clear();
  
  
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
  
  
  _filename=strdup(fn);
  _fileFormat=1;
  
  r=1;
  
  finalize:
  
  if (data) free(data);
  if (xco) xcor_close(&xco);
  if (fn) free((void*)fn);
  
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
  
  clear();
  
  loadOBJ((char*)data);
  
  _filename=strdup(fn);
  _fileFormat=0;
  free(data);
  
  free((void*)fn);
  
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

void StaticMesh::send(int idx) {
  if ((idx<0)||(idx>=_materials_n)) return;
  glDrawArrays(
    GL_TRIANGLES,
    _materials_v[idx].vertexOffset,
    _materials_v[idx].vertexCount);
}

void StaticMesh::render(SceneContext ctx) {
  size_t idx;
  MaterialSlice *pmat;
  
  bind();
  FOREACH(idx,pmat,_materials) {
    if (pmat->mat->shader()) {
      pmat->mat->bind(ctx);
      glDrawArrays(
        GL_TRIANGLES,
        pmat->vertexOffset,
        pmat->vertexCount);
      pmat->mat->unbind();
    }
  }
  
  unbind();
}

void StaticMesh::clear() {
  int i;
  size_t idx;
  MaterialSlice *pmat;
  
  _vertexCount=0;
  for(i=0;i<MAX_ARRAY_BUFFERS;i++) if (_buffers[i].handle) {
    glDeleteBuffers(1,&_buffers[i].handle);
  }
  memset(_buffers,0,sizeof(_buffers));
  FOREACH(idx,pmat,_materials) 
    pmat->mat->drop();
  ARRAY_DESTROY(_materials);
  
  
  if (_filename) free((void*)_filename);
  _filename=0;
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


int StaticMesh::load(const char *fn, int flags) {
  
  size_t len=strlen(fn);
  clear();
  
  if (len<4) return 0;
  if (strcmp_ic(fn+len-4,".obj")==0) {
    loadOBJFile(fn);
  } else if (strcmp_ic(fn+len-4,".dof")==0) {
    loadDOFFile(fn);
  }
  
  return 1;
}


IStaticMeshReferrer::IStaticMeshReferrer() : _mesh(0) { }
IStaticMeshReferrer::~IStaticMeshReferrer() {
  if (_mesh) _mesh->drop();
}

StaticMesh *IStaticMeshReferrer::mesh() { return _mesh; }
void IStaticMeshReferrer::setMesh(StaticMesh *m) {
  if (m==_mesh) return;
  if (_mesh) _mesh->drop();
  _mesh=m;
  if (_mesh) {
    _mesh->grab();
  }
}


AssetRegistry<StaticMesh> *_reg_mesh=0;
AssetRegistry<StaticMesh> *reg_mesh() {
  if (!_reg_mesh)
    _reg_mesh=new AssetRegistry<StaticMesh>(REPOSITORY_MASK_MESH);
  return _reg_mesh;
}


void reg_mesh_free() {
  if (!_reg_mesh) return;
  delete _reg_mesh;
  _reg_mesh=0;
}
