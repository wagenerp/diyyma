
/** \file scenecontext.h
  * \author Peter Wagener
  * \brief Contextual information for passing down during rendering.
  *
  */

#ifndef _DIYYMA_SCENECONTEXT_H
#define _DIYYMA_SCENECONTEXT_H

#include "diyyma/util.h"
#include "diyyma/math.h"

/** \brief Data to be passed to scene nodes for processing,
  * containing information such as the current projection matrix.
  *
  * Also, this is used for sharing a common view/projection among 
  * multiple components, e.g. an FPSCameraComponent usually generates
  * a SceneContext, which is then fed into a SceneNodeRenderPass, etc.
  */
struct SceneContext {
  Matrixf MVP; ///<\brief The current model-view-projection matrix.
  Matrixf MV;  ///<\brief The current model-view matrix.
  Matrixf M;   ///<\brief The current model matrix.
  Matrixf V;   ///<\brief The current view matrix.
  Matrixf P;   ///<\brief The current projection matrix.
  Vector3f camPos_w; ///<\brief The current camera position in world coordinates.

  double time; ///<\brief The current frame time.
  
  void setIdentity() {
    P.setIdentity();
    V.setIdentity();
    M.setIdentity();
    MV.setIdentity();
    MVP.setIdentity();
    camPos_w.set(0,0,0);
    time=0;
  }
  
};

/** \brief Interface for a source of a scene context, usually something
  * like a camera or an offscreen render pass.
  *
  * \todo This should be an RCObject, however i cannot figure out how to
  * actually make this happen as implementing classes are RCObjects already
  * and virtual inheritance seems to produce segfaults.
  */
class ISceneContextSource : public virtual IRCObject {
  
  public:
    virtual ~ISceneContextSource() { }
    virtual SceneContext context() =0;
};

class ISceneContextReferrer {
  protected:
    ISceneContextSource *_contextSource;
  
  public:
    ISceneContextReferrer();
    ~ISceneContextReferrer();
    
    ISceneContextSource *contextSource();
    void setContextSource(ISceneContextSource *l);
};


#endif