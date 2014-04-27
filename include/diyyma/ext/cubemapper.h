
/** \file cubemapper.h
  * \author Peter Wagener
  * \brief Camera component capturing cubemap images
  */
#ifndef _DIYYMA_EXT_FRAMEDUMPER_H
#define _DIYYMA_EXT_FRAMEDUMPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "SDL/SDL.h"
#include "GL/glew.h"

#include "diyyma/renderpass.h"
#include "diyyma/scenegraph.h"
#include "diyyma/util.h"

struct Cubemap {
  Vector3f origin;
  size_t   resolution;
  /** \brief Target file name to store the results in.
    *
    * This is a snprintf pattern that must contain exactly
    * one string pattern, which will receive 
    * 'xp', 'xn', 'yp', 'yn', 'zp', 'zn' for the positive x, negative x,
    * positive y, negative y, positive z and negative z face respectively.
    *
    * The file name extension should be .hdr if high dynamic range rendering
    * is performed.
    *
    * Note that this is only used if the Cubemapper is set to
    * export cubemaps.
    * 
    */
  char     *fn_pattern;
  
  /** \brief Target cubemap texture receiving the rendered cubemap
    *
    * This does not have to be set if cubemaps are exported to files only.
    */
  Texture  *target;
};

/** \brief Cubemapper flag. Set to export cubemaps to texture files.
  */
#define CUBEMAP_EXPORT 0x01


/** \brief Cubemapper flag. Set to generate high dynamic range images for
  * cubemaps.
  *
  * If textures are exported, they must have a valid format for holding the
  * extended value range, like .hdr.
  */
#define CUBEMAP_HDR 0x02

/** \brief Cubemap computation and export class.
  *
  * To render each cubemap, you first need to specify render passes
  * to be executed for each cubemap. These are executed in the order they
  * are assigned.
  *
  * While most renderpasses operate on the same write buffer (render target),
  * some post-processing passes may require a swap buffer. In this case,
  * additional color buffers can be acquired through addColorAttachment and
  * a subsequent call to getColorTexture. 
  * Care must be taken, however, to ensure that the final render pass 
  * writes onto GL_COLOR_ATTACHMENT0 as this is the attachment to which the
  * target texture is bound and from where pixels are exported into texture
  * files.
  *
  * A cubemap can be added for computation with addCubemap. Both texture 
  * targets and file targets can be intermixed at will, however computing
  * exported cubemaps more than once does not make any sense. This feature
  * was implemented purely for flexibility in cubemap computation.
  * To allow for statically and dynamically computed cubemaps to interchange,
  * the flag CUBEMAP_EXPORT is used: If unset, no cubemaps are exported to
  * any files.
  *
  * After all cubemaps were registered and renderpasses were assigned, a call
  * to computeCubemaps will render all cubemap faces and export them to their
  * respective files.
  *
  * To control the cubemapped scene further, the frame time can be set 
  * for all subsequently rendered cubemaps with a call to setFrameTime.
  * Also, the projection's znear and zfar parameters can be altered using
  * setProjectionParameters.
  * 
  * Most renderpasses require a context source to compute the correct
  * transformation matrices, therefore the Cubemapper implements the
  * ISceneContextSource which will return the current cubemap face's 
  * transformation matrices and the designated frame time.
  * 
  * Behavior as an ISceneContextSource is undefined outside of cubemap
  * computation, so it cannot be used in displayed render passes.
  * 
  */
class Cubemapper : 
  public ISceneContextSource,
  public RCObject {
  private:
    ARRAY(Cubemap,_cubemap);
    ARRAY(IRenderPass*,_renderpass);
    ARRAY(Texture*,_colorAttachment);
    
    unsigned int _max_resolution;
    
    GLuint _framebuffer;
    GLuint _b_depth;
    
    SceneContext _context;
    
    void *_pixel_buffer;
    
    void _initColorAttachment(
      Texture **ptex,GLenum attachment,
      int width, int height);
    
  public:
    /** \brief Constructor.
      * \param max_width the absolute maximum resolution *any* computed 
      * cubemap will have.
      */
    Cubemapper(unsigned int max_resolution);
    ~Cubemapper();
    
    int flags;
    
    /** \brief Sets near and far clipping planes for all generated cubemaps.
      *
      * Default (initialization) values are near clipping at 1 and far clipping
      * at 1000.
      *
      * \param znear Near clipping plane. Must be larger than zero.
      * \param zfar Far clipping plane. Must be larger zan the near clipping
      * plane.
      */
    
    void setProjectionParameters(float znear, float zfar);
    
    
    
    /** \brief Adds a new color attachment to the internal frame buffer.
      *
      * Used for scenes with multiple (off-screen) renderpasses.
      */
    GLenum addColorAttachment();
    
    /** \brief Returns the texture assigned with the specified color 
      * attachment, if it exists.
      *
      * The first (0) color attachment always exist and is the default
      * render target if no other gets bound.
      * If a different render target is specified, the final render pass
      * must set the draw buffer to GL_COLOR_ATTACHMENT0.
      *
      * \param index The index of the color attachment. Coincides with the
      * GL_COLOR_ATTACHMENT offset. The index must not exceed the number
      * of preceeding calls to addColorAttachment plus one.
      */
    
    Texture *getColorTexture(unsigned int index);
    
    /** \brief Appends a render pass for cubemap generation.
      * 
      * Renderpasses are executed in the order they were appended.
      */
    void addRenderpass(IRenderPass *p);
    
    /** \brief Adds a cubemap for computation.
      * \param origin World-coordinate vector specifying the point of view
      * for the generated cubemap.
      * \param resolution Number of pixels in the x and y direction of all
      * generated images. Must not exceed max_resolution specified in the
      * constructor.
      * \param fn_format Optional. Specifies a file name pattern to output
      * the cubemap to. See Cubemap.fn_format for more details.
      * \param target Optional. Cubemap texture receiving the generated image.
      *
      * If neither fn_format nor target are specified, no cubemap is added.
      */
    void addCubemap(
      const Vector3f &origin,unsigned int resolution,
      const char *fn_pattern, Texture *target);
    
    /** \brief sets the frame time to be used in all cube maps.
      */
    
    void setFrameTime(double time);
    
    /** \brief Computes all assigned cubemaps
      */
    void computeCubemaps();
    
    
    virtual SceneContext context();
};
#endif