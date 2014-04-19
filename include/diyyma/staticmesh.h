
/** \file staticmesh.h
  * \author Peter Wagener
  * \brief Static mesh object.
  *
  * A static mesh is a single entity, consisting of a single stream of triangles
  * defined by an arbitrary list of array buffers.
  */

#ifndef _DIYYMA_STATICMESH_H
#define _DIYYMA_STATICMESH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "GL/glew.h"

#include "diyyma/util.h"


#define BUFIDX_VERTICES 0 ///< \brief layout index for vertex streams
#define BUFIDX_NORMALS 1 ///< \brief layout index for normal data streams
#define BUFIDX_TEXCOORDS 2 ///< \brief layout index for texcoord streams
#define BUFIDX_COLORS 3 ///< \brief layout index for color streams
#define BUFIDX_BINORMALS 4 ///< \brief layout index for binormal data streams
#define BUFIDX_TANGENTS 5 ///< \brief layout index for tangent data streams

/** \brief Universally unique token identifying DIYYMA object sub-format
  * in XCO trees. */
#define XCO_DIYYMA_OBJECT 0x4f594944
/** \brief Locally unique token identifying DIYYMA object array buffers */
#define XCO_DIYYMA_OBJECT_ARRAY  0x00010001

struct DOFArray {
  u_int32_t index;
  u_int32_t type;
  u_int32_t dimension;
  u_int32_t cbData;
};


struct ArrayBuffer {
  GLuint handle;
  int index;
  GLenum type;
  int dimension;
  
};

#define MAX_ARRAY_BUFFERS 6

class StaticMesh : public IAsset {
  private:
    int _vertexCount;
    ArrayBuffer _buffers[MAX_ARRAY_BUFFERS];
    char *_filename;
    int _fileFormat;
    
  public:
    StaticMesh();
    ~StaticMesh();
    
    void loadOBJ(char *code);
    int loadOBJFile(const char *fn);
    int loadDOFFile(const char *fn);
    
    void bind();
    void unbind();
    void send();
    
    void clear();
    
    
    virtual void reload();
    virtual timestamp_t filesTimestamp();
    
    /** \brief Deduces the file type by extension (lowercase .ecn or .dof)
      */
    virtual int load(const char *fn);
};

#endif