
/** \file renderpass.h
  * \author Peter Wagener
  * \brief Render passes
  *
  *
  */

#ifndef _DIYYMA_RENDERPASS_H
#define _DIYYMA_RENDERPASS_H

#include "diyyma/scenegraph.h"
#include "diyyma/component.h"
#include "diyyma/shader.h"
#include "diyyma/staticmesh.h"
#include "diyyma/texture.h"
#include "diyyma/util.h"
#include "diyyma/math.h"

#include "SDL/SDL.h"

class SceneNodeRenderPass : public IComponent {
  private:
    ARRAY(IRenderableSceneNode*,_nodes);
    ARRAY(double,_distance);
    SceneContext _context;
    
  public:
    SceneNodeRenderPass();
    ~SceneNodeRenderPass();
    
    
    SceneContext *context() { return &_context; }
    
    void sortByDistance(const Vector3f &center);
    /** \brief sorts by distance to the camera, taken from context()->MV.
      */
    void sortByDistance();
    
    void operator+=(IRenderableSceneNode *node);
    
    virtual void render();
    virtual int event(const SDL_Event *ev);
    virtual void iterate(double dt, double time);
  
};


#endif