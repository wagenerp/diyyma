
#include "diyyma/material.h"
#include "GL/glew.h"

Material::Material() :
  _u_MVP(0),
  _u_MV(0),
  _u_V(0),
  _u_P(0),
  _u_time(0),
  ambient{1,1,1},
  diffuse{1,1,1},
  specular{1,1,1},
  emission{0,0,0},
  transmission{1,1,1},
  alpha(1),
  shininess(40),
  refractionIndex(1),
  IShaderReferrer(),
  ITextureReferrer<MAX_MATERIAL_TEXTURES>() {
  
  _shaderReferrer=this;
}

Material::~Material() { }

void Material::updateUniforms() {
  _u_MVP =_shader->locate("u_MVP");
  _u_MV  =_shader->locate("u_MV");
  _u_V   =_shader->locate("u_V");
  _u_P   =_shader->locate("u_P");
  _u_time=_shader->locate("u_time");
  // todo: locate material parameters
}

void Material::applyUniforms(SceneContext ctx) {
  if (_u_P   ) glUniformMatrix4fv(_u_P  ,1,0,&ctx.V.a11);
  if (_u_V   ) glUniformMatrix4fv(_u_V  ,1,0,&ctx.V.a11);
  if (_u_MV  ) glUniformMatrix4fv(_u_MV ,1,0,&ctx.MV.a11);
  if (_u_MVP ) glUniformMatrix4fv(_u_MVP,1,0,&ctx.MVP.a11);
  if (_u_time) glUniform1f(_u_time,ctx.time);
  // todo: transmit material parameters (perhaps use UBOs)
}

void Material::bind(SceneContext ctx) {
  int i;
  if (_shader) {
    _shader->bind();
    for(i=0;i<MAX_MATERIAL_TEXTURES;i++)
      if (_texture_locs[i]) 
        glUniform1i(_texture_locs[i],i);
    applyUniforms(ctx);
  }
  for(i=0;i<MAX_MATERIAL_TEXTURES;i++)
    if (_textures[i]) _textures[i]->bind(i);
}

void Material::unbind() {
  int i;
  
  for(i=0;i<MAX_MATERIAL_TEXTURES;i++)
    if (_textures[i]) _textures[i]->unbind();
  
  if (_shader) _shader->unbind();
}
MaterialLibrary::MaterialLibrary() {
  ARRAY_INIT(_materials);
  _filename=0;
}

MaterialLibrary::~MaterialLibrary() {
  size_t idx;
  NamedMaterial *pmat;
  
  FOREACH(idx,pmat,_materials) {
    pmat->mat->drop();
    free((void*)pmat->name);
  }
  
  ARRAY_DESTROY(_materials);
  
  if (_filename)
    free((void*)_filename);
}

int MaterialLibrary::find(const char *name, int create) {
  size_t idx;
  NamedMaterial *pmat;
  NamedMaterial newMat;
  
  FOREACH(idx,pmat,_materials) 
    if (strcmp(name,pmat->name)==0) return idx;
  
  if (create) {
    newMat.mat=new Material();
    newMat.mat->grab();
    newMat.name=strdup(name);
    APPEND(_materials,newMat);
    return _materials_n-1;
  }
  return -1;
}

int MaterialLibrary::find(const SubString &name, int create) {
  size_t idx;
  NamedMaterial *pmat;
  NamedMaterial newMat;
  
  FOREACH(idx,pmat,_materials) 
    if (name==pmat->name) return idx;
  
  if (create) {
    newMat.mat=new Material();
    newMat.mat->grab();
    newMat.name=name.dup();
    APPEND(_materials,newMat);
    return _materials_n-1;
  }
  return -1;
}

Material *MaterialLibrary::get(const char *name, int create) {
  Material *mat;
  int idx;
  
  idx=find(name,create);
  if (idx>-1) return _materials_v[idx].mat;
  return 0;
}

Material *MaterialLibrary::get(const SubString &name, int create) {
  Material *mat;
  int idx;
  
  idx=find(name,create);
  if (idx>-1) return _materials_v[idx].mat;
  return 0;
}

void MaterialLibrary::reload() {
  if (!_filename) return;
  load(_filename,MATLIB_LOAD_UPDATE);
}

timestamp_t MaterialLibrary::filesTimestamp() {
  if (_filename)
    return file_timestamp(_filename);
  
  return 0;
}

int MaterialLibrary::load(const char *fn, int flags) {
  char *refmask_v=0;
  int refmask_n=0;
  
  void *data;
  size_t cb;
  LineScanner *scanner;
  SubString str;
  int idx, i,j;
  size_t   imat;
  NamedMaterial *pmat;
  Material *mat=0;
  
  double bd;
  long   bl;
  unsigned long bul;
  
  AssetRegistry<Texture> *tex=reg_tex();
  AssetRegistry<Shader> *shd=reg_shd();
  
  // locate and read the material library file
  if (_filename) free((void*)_filename);
  _filename=vfs_locate(fn,REPOSITORY_MASK_MESH);
  if (!_filename)
    return 0;
  
  if (!readFile(_filename,&data,&cb))
    return 0;
  
  // initiate structures for handling updates
  if (flags&MATLIB_LOAD_UPDATE) {
    refmask_n=_materials_n;
    if (refmask_n) {
      refmask_v=(char*)malloc(refmask_n);
      memset(refmask_v,0,refmask_n);
    }
  } else {
    FOREACH(imat,pmat,_materials) {
      pmat->mat->drop();
      free((void*)pmat->name);
    }
    ARRAY_DESTROY(_materials);
  }
  
  // scan the file
  scanner=new LineScanner();
  scanner->grab();
  scanner->assign((char*)data,cb);
  
  while(scanner->getLnFirstString(&str)) {
    if (str=="newmtl") {
      if (!scanner->getLnString(&str)) continue;
      if (mat) mat->drop();
      idx=find(str,1);
      if (idx<refmask_n) refmask_v[idx]=1;
      mat=_materials_v[idx].mat;
      mat->grab();
    } else if (!mat) {
    } else if (str=="Ka") {
      if (!scanner->getFloat(&mat->ambient.x,1)) continue;
      if (!scanner->getFloat(&mat->ambient.y,1)) mat->ambient.y=mat->ambient.x;
      if (!scanner->getFloat(&mat->ambient.z,1)) mat->ambient.z=mat->ambient.x;
    } else if (str=="Kd") {
      if (!scanner->getFloat(&mat->diffuse.x,1)) continue;
      if (!scanner->getFloat(&mat->diffuse.y,1)) mat->diffuse.y=mat->diffuse.x;
      if (!scanner->getFloat(&mat->diffuse.z,1)) mat->diffuse.z=mat->diffuse.x;
    } else if (str=="Ks") {
      if (!scanner->getFloat(&mat->specular.x,1)) continue;
      if (!scanner->getFloat(&mat->specular.y,1)) mat->specular.y=mat->specular.x;
      if (!scanner->getFloat(&mat->specular.z,1)) mat->specular.z=mat->specular.x;
    } else if (str=="Tf") {
      if (!scanner->getFloat(&mat->transmission.x,1)) continue;
      if (!scanner->getFloat(&mat->transmission.y,1)) mat->transmission.y=mat->transmission.x;
      if (!scanner->getFloat(&mat->transmission.z,1)) mat->transmission.z=mat->transmission.x;
    } else if (str=="illum") {
      // ignored. Illumination models are part of the shader
    } else if (str=="d") {
      scanner->getDouble(&mat->alpha,1);
    } else if (str=="Ns") {
      scanner->getDouble(&mat->shininess,1);
    } else if (str=="Ni") {
      scanner->getDouble(&mat->refractionIndex,1);
    } else if (str=="map_Ka") {
      if (!scanner->getLnString(&str)) continue;
      mat->addTexture(tex->get(str),"s_ambient");
    } else if (str=="map_Kd") {
      if (!scanner->getLnString(&str)) continue;
      mat->addTexture(tex->get(str),"s_diffuse");
    } else if (str=="map_Ks") {
      if (!scanner->getLnString(&str)) continue;
      mat->addTexture(tex->get(str),"s_specular");
    } else if (str=="#shader") {
      // todo: investigate if specifying new commands causes problems with
      // common loaders and if so, make it a line comment.
      
      if (!scanner->getLnString(&str)) continue;
      mat->setShader(shd->get(str));
    }
    
  }
  
  // drop materials no longer found in the file
  for(i=0,idx=0;i<refmask_n;i++)
    if (!refmask_v[i]) {
      _materials_v[idx].mat->drop();
      for(j=idx;j<_materials_n-1;j++)
        _materials_v[idx]=_materials_v[idx+1];
      _materials_n--;
    } else
      idx++;
  
  // cleanup
  scanner->drop();
  free(data);
  if (refmask_v) free(refmask_v);
}


AssetRegistry<MaterialLibrary> *_reg_mtl=0;
AssetRegistry<MaterialLibrary> *reg_mtl() {
  if (!_reg_mtl)
    _reg_mtl=new AssetRegistry<MaterialLibrary>(REPOSITORY_MASK_MESH);
  return _reg_mtl;
}