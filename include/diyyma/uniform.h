
/** \file manager.h
  * \author Peter Wagener
  * \brief Management of asset loading, scene composition and modification.
  *
  *
  */

#ifndef _DIYYMA_UNIFORM_H
#define _DIYYMA_UNIFORM_H

#include "diyyma/util.h"
#include "GL/glew.h"

#define UNIFORM_BOOL 0
#define UNIFORM_INT 1
#define UNIFORM_UINT 2
#define UNIFORM_FLOAT 3
#define UNIFORM_DOUBLE 4
#define UNIFORM_BVEC2 5
#define UNIFORM_BVEC3 6
#define UNIFORM_BVEC4 7
#define UNIFORM_IVEC2 8
#define UNIFORM_IVEC3 9
#define UNIFORM_IVEC4 10
#define UNIFORM_UVEC2 11
#define UNIFORM_UVEC3 12
#define UNIFORM_UVEC4 13
#define UNIFORM_VEC2 14
#define UNIFORM_VEC3 15
#define UNIFORM_VEC4 16
#define UNIFORM_DVEC2 17
#define UNIFORM_DVEC3 18
#define UNIFORM_DVEC4 19
#define UNIFORM_MAT2 20
#define UNIFORM_MAT3 21
#define UNIFORM_MAT4 22
#define UNIFORM_MAT2x3 23
#define UNIFORM_MAT2x4 24
#define UNIFORM_MAT3x2 25
#define UNIFORM_MAT3x4 26
#define UNIFORM_MAT4x2 27
#define UNIFORM_MAT4x3 28
#define UNIFORM_DMAT2 29
#define UNIFORM_DMAT3 30
#define UNIFORM_DMAT4 31
#define UNIFORM_DMAT2x3 32
#define UNIFORM_DMAT2x4 33
#define UNIFORM_DMAT3x2 34
#define UNIFORM_DMAT3x4 35
#define UNIFORM_DMAT4x2 36
#define UNIFORM_DMAT4x3 37

#define UNIFORM_FORMAT_COUNT 38

const size_t UNIFORM_SIZE[UNIFORM_FORMAT_COUNT]= {
  1,4,4,4,8,
  2,3,4,
  8,12,16,
  8,12,16,
  8,12,16,
  16,24,32,
  16,36,64,
  24,32,
  24,48,
  32,48,
  
  32,72,128,
  48,64,
  48,96,
  64,96
  
};

class UniformBlock {
  private:
    ARRAY(GLenum,_formats);
    size_t _cb;
    void *_data;
    
    GLuint _ubo;
    
  public:
    UniformBlock();
    ~UniformBlock();
    
    void operator+(GLenum fmt);
    GLenum operator[](unsigned int idx) const;
    size_t count() const;
    size_t cb() const;
    void *data() const;
    
    void update();
    void bind(GLuint idx) const;
};


#endif