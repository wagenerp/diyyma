
/** \file shader.h
  * \author Peter Wagener
  * \brief Static mesh object.
  *
  * Shader creation / compilation, all wrapped up in one convenient class.
  */
#ifndef _DIYYMA_SHADER_H
#define _DIYYMA_SHADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "GL/glew.h"

#include "diyyma/util.h"


#define SHADER_PROGRAM_COUNT 3

#define SHADER_INDEX_VERTEX 0
#define SHADER_INDEX_GEOMETRY 1
#define SHADER_INDEX_FRAGMENT 2

#define SHADER_INDEX(gle) (\
  gle==GL_VERTEX_SHADER?0:\
  gle==GL_GEOMETRY_SHADER?1:\
  gle==GL_FRAGMENT_SHADER?2:\
  -1)
  
#define SHADER_MODE(idx) (\
  idx==0?GL_VERTEX_SHADER:\
  idx==1?GL_GEOMETRY_SHADER:\
  idx==2?GL_FRAGMENT_SHADER:\
  -1)

/** \brief Representation of a single OpenGL shader program consisting of
  * fragment, vertex and / or geometry shaders.
  */
class Shader : public IAsset {
  private:
    GLuint 
      _shader[SHADER_PROGRAM_COUNT];
    GLuint
      _program;
    
    int
      _linked;
    
    char *_sourceFiles[SHADER_PROGRAM_COUNT];
    
  public:
    /** \brief Creates an empty, unlinked shader program. */
    Shader();
    
    /** \brief Creates a shader program and attempts to load shaders from
      * files of a common base name.
      *
      * Fragment shaders are loaded from basename ~ ".fsd",
      * vertex shaders are loaded from basename ~ ".vsd",
      * geometry shaders are loaded from basename ~ ".gsd".
      */
    Shader(const char *basename);
    /** \brief Creates a shader program, attaching shaders and linking.
      *
      * \param vsd Code of a vertex shader to attach, can be null.
      * \param fsd Code of a fragment shader to attach, can be null.
      * \param gsd Code of a geometry shader to attach, can be null.
      */
    Shader(const char *vsd, const char *fsd, const char *gsd);
    
    ~Shader();
    
    /** \brief Getter for the OpenGL program object name. */
    GLuint program();
    /** \brief Calls glGetUniformLocation for this shader and the specified
      * identifier.*/
    GLuint locate(const char *id);
    
    /** \brief Attaches shader code to this program if the specified
      * mode was not already attached.
      *
      * \param code Code to attach.
      * \param cc Number of characters in the string pointed to by code, or
      * 0 to use strlen.
      * \param mode GL_VERTEX_SHADER, GL_FRAGMENT_SHADER or GL_GEOMETRY_SHADER.
      * \return 1 on success, 0 otherwise.
      */
    int attach(const char *code, size_t cc, int mode);
    
    /** \brief Attaches a shader code file to this program if the specified
      * mode was not already attached.
      *
      * \param code file name to attach, will be passed through vfs_locate
      * with REPOSITORY_MASK_SHADER.
      * \param mode GL_VERTEX_SHADER, GL_FRAGMENT_SHADER or GL_GEOMETRY_SHADER.
      * \return 1 on success, 0 otherwise.
      */
    int attachFile(const char *fn, int mode);
    
    /** \brief Links this program if it hasn't been done already.
      *
      */
    int link();
    
    /** \brief Activates this shader for use in rendering. */
    void bind();
    /** \brief Deactivates this shader after use in rendering. */
    void unbind();
    
    /** \brief Deactivates any bound shader. */
    static void Unbind();
    
    virtual void reload();
    virtual timestamp_t filesTimestamp();
    
    /** \brief behaves exactly like creating a new shader, specifying 
      * fn as sole argument. */
    virtual int load(const char *fn);
};


class IShaderReferrer {
  protected:
    Shader *_shader;
  
  public:
    IShaderReferrer();
    ~IShaderReferrer();
    
    Shader *shader();
    void setShader(Shader *s);
    
    virtual void updateUniforms() =0;
};

#endif