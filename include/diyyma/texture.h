
/** \file texture.h
  * \author Peter Wagener
  * \brief Texture loading wrapper.
  *
  * For this project, a simple texture format is used, called diyyma texture file
  * (.dtf). It has the following structure:
  *
  *        44 59 54 58  [4B]  (ASCII "DYTX") - magic token
  *        cbHead       [4B]   size of a DTF header following immediately after
  *                            the file header.
  *        cbRaw        [4B]   size of the raw image data, uncompressed.
  *        cbCompressed [4B]   size of the image data stored in the file.
  *        compression  [4B]   compression method to used, fixed to 1.
  *        head         [cbHead]
  *        raw date     [cbCompressed]
  *
  * For details on the DTF head, refer to the DTFHead structure.
  */

#ifndef _DIYYMA_TEXTURE_H
#define _DIYYMA_TEXTURE_H

#include "GL/glew.h"
#include "diyyma/util.h"
#include "diyyma/shader.h"
#include "diyyma/math.h"

struct DTFHead {
  u_int32_t channels;  ///<\brief Number of color channels in the stream.
  u_int32_t format;    ///<\brief Format of the stream (e.g. GL_RGBA)
  u_int32_t type;      ///<\brief Data type used in the stream (e.g. GL_UNSIGNED_BYTE))
  u_int32_t width;     ///<\brief Width, in pixels, of the texture.
  u_int32_t height;    ///<\brief Height, in pixels, of the texture.
  
  /** \brief Depth, in pixels, of the texture.
    * 
    * If set to a number less than 2, a 2D texture is assumed.
    */
  u_int32_t depth;     
  u_int32_t min_filter; ///<\brief Minimization filter to use.
  u_int32_t mag_filter; ///<\brief Magnification filter to use.
  
  u_int32_t clamp_s; ///<\brief clamping method to use in first direction
  u_int32_t clamp_t; ///<\brief clamping method to use in second direction
  u_int32_t clamp_u; ///<\brief clamping method to use in third direction
};

/** \brief Loads a texture file into an OppenGL texture resource.
  *
  * In case of an arbitrary texture file, it is converted to RGBA and stored
  * as UNSIGNED_BYTE, filtering is set to LINEAR.
  * If a .dtf file is specified, meta information from the file are used.
  *
  * \param fn file name pointing to the texture resource to load. File type
  * is deduced automatically. The file name is always passed through
  * vfs_locate with REPOSITORY_MASK_TEXTURE.
  *
  * \return Name of the generated OpenGL texture, 0 on error.
  */
GLuint loadTextureFile(const char *fn_in, GLuint tex_in=0);

#define TEXTURE_SLOTS 8

/** \brief Texture loading flag causing a cubemap to be loaded.
  *
  * The file name must then be a valid snprintf pattern containing exactly
  * one string input. This input will receive 'xp', 'xn', 'yp', ... for
  * the positive x, negative x, ... faces of the cube map.
  */
#define TEXTURE_LOAD_CUBEMAP 0x01

/** \brief Texture loading flag causing an HDR texture to be generated.
  *
  * This is only useful when the loaded file supplies extended color information
  * such as .hdr does.
  */
#define TEXTURE_LOAD_HDR 0x02

struct CubemapData {
  Matrixf V;
  GLenum textureTarget;
  const char *name;
};

// todo: These matrices were created by hand. Check them.
const CubemapData CUBEMAP_DATA[6] = {
  {
    Matrixf(
      1,0,0,0,
      0,0,1,0,
      0,-1,0,0,
      0,0,0,1 
    ),
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    "xp"
  },
  {
    Matrixf(
      -1,0,0,0,
      0,0,-1,0,
      0,-1,0,0,
      0,0,0,1
    ),
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    "xn"
  },
  {
    Matrixf(
      0,1,0,0,
      -1,0,0,0,
      0,0,1,0,
      0,0,0,1
    ),
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    "yp"
  },
  {
    Matrixf(
      0,-1,0,0,
      -1,0,0,0,
      0,0,-1,0,
      0,0,0,1
    ),
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    "yn"
  },
  {
    Matrixf(
      0,0,1,0,
      -1,0,0,0,
      0,-1,0,0,
      0,0,0,1
    ),
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    "zp"
  },
  {
    Matrixf(
      0,0,-1,0,
      1,0,0,0,
      0,-1,0,0,
      0,0,0,1
    ),
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    "zn"
  }
};
class Texture;

class Texture : public IAsset {
  private:
    char *_filename;
    char *_filename_cube[6];
    GLuint _name;
    int _slot;
    GLenum _target;
    int _loadHDR;
    static Texture *__boundTextures[TEXTURE_SLOTS];
  public:
    Texture();
    Texture(const char *fn_in);
    Texture(GLenum target);
    ~Texture();
    
    GLuint name();
    GLenum target();
    
    virtual void reload();
    virtual timestamp_t filesTimestamp();
    
    /** \brief Initiates the texture buffers for receiving a cubemap
      * texture. 
      *
      * This is only required if the cubemap is computed and stored in this
      * texture.
      * 
      * \param resolution The width and height of the texture.
      * \param hdr Set to non-zero to allow hdr image data.
      *
      */
    void initCubemap(unsigned int resolution, int hdr);
    
    void bind(int slot);
    /** \brief Binds the texture to any slot, if available.
      *
      * If the texture is already bound or was sucessfully bound to a new
      * slot, its index is returned. Otherwise the function returns -1.
      */
    int bind();
    
    void unbind();
    
    static void Unbind();
    static void Unbind(int slot);
    
    /** \brief Behaves exactly as the constructor.
      */
    virtual int load(const char *fn, int flags);
};


template<int N> class ITextureReferrer {
  protected:
    Texture    *_textures[N];
    GLint       _texture_locs[N];
    
    IShaderReferrer *_shaderReferrer;
    
  public:
    ITextureReferrer():
      _shaderReferrer(0)
      {
      int i;
      for(i=0;i<N;i++) {
        _textures[i]=0;
        _texture_locs[i]=-1;
      }
    }
    ~ITextureReferrer() {
      int i;
      for(i=0;i<N;i++)
        if (_textures[i]) _textures[i]->drop();
    }
    
    Texture *texture(size_t index) {
      if (index>=N) return 0;
      return _textures[index];
    }
    size_t textureCount() {
      int i;
      
      for(i=0;i<N;i++)
        if (!_textures[i]) break;
      return i;
    }
    void addTexture(Texture *t, const char *loc) {
      int i;
      Shader *shd=0;
      if (_shaderReferrer) shd=_shaderReferrer->shader();
      for(i=0;i<N;i++)
        if (!_textures[i]) {
          t->grab();
          _textures[i]=t;
          if (shd)
            _texture_locs[i]=shd->locate(loc);
          else 
            _texture_locs[i]=-1;
          break;
        }
      
    }
    
};

AssetRegistry<Texture> *reg_tex();
void reg_tex_free();

#endif