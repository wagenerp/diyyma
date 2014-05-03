
/** \file manager.h
  * \author Peter Wagener
  * \brief Management of asset loading, scene composition and modification.
  *
  *
  */

#ifndef _DIYYMA_MATERIAL_H
#define _DIYYMA_MATERIAL_H

#include "diyyma/util.h"
#include "diyyma/math.h"
#include "diyyma/texture.h"
#include "diyyma/shader.h"
#include "diyyma/scenecontext.h"

#define MAX_MATERIAL_TEXTURES 8

/** \brief Material library loading flag demarking updates - this will not 
  * recreate materials of an already existing name but update them instead.
  * 
  * This should *always* be set if a reloading is performed, otherwise
  * previously referred materials will not be updated and memory is wasted, 
  * big time.
  */
#define MATLIB_LOAD_UPDATE 0x01


class Material : public RCObject,
  public IShaderReferrer,
  public ITextureReferrer<MAX_MATERIAL_TEXTURES>
  {
  private:
    
    GLuint _u_MVP;
    GLuint _u_MV;
    GLuint _u_V;
    GLuint _u_P;
    GLuint _u_time;
  public:
    /** \brief Ambient reflection coefficient. */
    Vector3f ambient;
    /** \brief Diffuse reflection coefficient. */
    Vector3f diffuse;
    /** \brief Specular reflection coefficient. */
    Vector3f specular;
    /** \brief Autoemissive color */
    Vector3f emission;
    /** \brief Translucent material transmission coefficient.
      *
      * When light passes through a translucent object, it gets attenuated.
      * This vector describes attenuation factors per color component.
      */
    Vector3f transmission;
    
    double alpha;
    double shininess;
    double refractionIndex;
  
  public:
    Material();
    ~Material();
    
    virtual void updateUniforms();
    virtual void applyUniforms(SceneContext ctx);
    
    virtual void bind(SceneContext ctx);
    virtual void unbind();
};

struct NamedMaterial {
  Material *mat;
  char     *name;
};

/** \brief A .mtl material library as seen refered from wavefront objects.
  */
class MaterialLibrary : public IAsset {
  private:
    ARRAY(NamedMaterial,_materials);
    char *_filename;
  
  public:
    MaterialLibrary();
    ~MaterialLibrary();
    
    Material *get(const char *name, int create=0);
    Material *get(const SubString &name, int create=0);
    
    int find(const char *name, int create=0);
    int find(const SubString &name, int create=0);
    
    virtual void reload();
    virtual timestamp_t filesTimestamp();
    virtual int load(const char *fn, int flags);
};

AssetRegistry<MaterialLibrary> *reg_mtl();
void reg_mtl_free();

#endif