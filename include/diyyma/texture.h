
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

class Texture : public IAsset {
  private:
    char *_filename;
    GLuint _name;
    int _slot;
    static GLuint __boundTextures[TEXTURE_SLOTS];
  public:
    Texture();
    Texture(const char *fn_in);
    ~Texture();
    
    GLuint name();
    
    virtual void reload();
    virtual timestamp_t filesTimestamp();
    
    void bind(int slot);
    void unbind();
    
    static void Unbind();
    static void Unbind(int slot);
    
    /** \brief Behaves exactly as the constructor.
      */
    virtual int load(const char *fn);
};

#endif