
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
#include "diyyma/material.h"


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

struct MaterialSlice {
  Material *mat;
  int       vertexCount;
  int       vertexOffset;
};

#define MAX_ARRAY_BUFFERS 6

/** \brief Represents a single mesh of static data.
  * 
  * This data consists of any number of array buffers of various purposes
  * (See BUFIDX_* constants) and one or more materials.
  * Each material is responsible for a continuous block of vertices and is
  * specified only by its name, to be loaded and handled by surrounding objects 
  * such as scene nodes.
  */

class StaticMesh : public IAsset {
  private:
    int _vertexCount;
    ArrayBuffer _buffers[MAX_ARRAY_BUFFERS];
    char *_filename;
    int _fileFormat;
    ARRAY(MaterialSlice,_materials);
    
  public:
    StaticMesh();
    ~StaticMesh();
    
    size_t materialCount();
    const MaterialSlice &material(int idx);
    
    
    void loadOBJ(char *code);
    int loadOBJFile(const char *fn);
    int loadDOFFile(const char *fn);
    
    void bind();
    void unbind();
    /** \brief Renders all faces using the currently configured material.*/
    void send();
    /** \brief Renders faces assigned with a single material using the 
      * currently configured material. */
    void send(int idx);
    
    /** \brief Renders all geometry using assigned materials.
      *
      * This is a compbination of bind, send and unbind, all in one call.
      *
      */
    void render(SceneContext ctx);
    
    void clear();
    
    
    virtual void reload();
    virtual timestamp_t filesTimestamp();
    
    /** \brief Deduces the file type by extension (lowercase .ecn or .dof)
      */
    virtual int load(const char *fn, int flags);
};


class IStaticMeshReferrer {
  protected:
    StaticMesh *_mesh;
  
  public:
    IStaticMeshReferrer();
    ~IStaticMeshReferrer();
    
    StaticMesh *mesh();
    void setMesh(StaticMesh *m);
    
};


AssetRegistry<StaticMesh> *reg_mesh();

#endif