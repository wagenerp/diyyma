
/** \file component.h
  * \author Peter Wagener
  * \brief Modular component system.
  *
  * Components for recurring tasks, such as camera movement / control,
  * reloading assets, etc.
  */

#ifndef _DIYYMA_COMPONENT_H
#define _DIYYMA_COMPONENT_H

#include "diyyma/shader.h"
#include "diyyma/math.h"
#include "diyyma/staticmesh.h"
#include "diyyma/scenegraph.h"
#include "diyyma/texture.h"

#include "SDL/SDL.h"
#include "GL/glew.h"

/** \brief Interface for application modules (components).
  *
  * Each component taps into the rendering, event-handling and iteration
  * callbacks of the main method, each right before the static callback
  * methods are invoked. (Or between render_pre and render_post in case of
  * the rendering callback)
  *
  * Initialization is performed in the constructor and cleanup in the
  * destructor.
  */

class IComponent : public RCObject {
  public:
    virtual ~IComponent() { }
    virtual void render() =0;
    virtual int event(const SDL_Event *ev) =0;
    virtual void iterate(double dt, double time) =0;
    
};

/** \brief FPS-like camera motion using mouse and WSAD. 
  *
  * This module provides an ISceneContextSource implementation and can 
  * therefore be stuffed into a render pass, light controller, etc.
  *
  * While camera motion is just one part of the deal, a public field
  * P is used for the pure projection matrix.
  * 
  * If manual intervention is desired, the fields pos and angles can be set,
  * followed by a call of the compute method to update the matrices (V and VP).
  *
  * To control motion speed and mouse sensitivity, use the fields
  * speed and mouseSensitivity, defaulting to 1.
  *
  **/
class FPSCameraComponent : public IComponent, public ISceneContextSource {
  private:
    long _keys;
    int _mods;
    
    int _mouseFlight;
    int _mouseFlight_x;
    int _mouseFlight_y;
    
  public:
    Matrixf V, VP, P;
    /** \brief Current camera position, in world coordinates*/
    Vector3f pos;
    /** \brief Current camera orientation, 
      * in x,y,z-axis rotation (roll,pitch,yaw)
      */
    Vector3f angles;
    float speed;
    float mouseSensitivity;
    
  public:
    FPSCameraComponent();
    ~FPSCameraComponent();
    
    /** \brief Recomputes the view and view-projection matrices.
      *
      * This is performed automatically on render() so you usually do
      * not have to bother with this. 
      *
      * Please note that the projection matrix is *not* computed here, but 
      * may be set manually or computed in the constructor.
      */
    void compute();
	/** \brief Returns normalized view direction in world cordinates*/
	Vector3f view();
	/** \brief Returns normalized up direction in world cordinates*/
	Vector3f up();
    
    virtual void render();
    virtual int event(const SDL_Event *ev);
    virtual void iterate(double dt, double time);
    
    virtual SceneContext context();
};

/** \brief Structure used by the AssetReloader component. You do not need to
  * mind this.
  */
struct AssetRecord {
  IAsset *asset;
  timestamp_t lastModified;
  
  AssetRecord(IAsset *a, timestamp_t t) : asset(a), lastModified(t) { }
};

/** \brief Automatic asset reloading component.
  *
  * Periodically checks all assigned assets for changes in the file system
  * counterpart and reloads them if a newer timestamp was detected.
  */
class AssetReloader : public IComponent {
  private:
    AssetRecord *_asset_v;
    size_t       _asset_n;
    double       _tCheck;
  public:
    AssetReloader();
    AssetReloader(IAsset *asset);
    ~AssetReloader();
    
    /** \brief The amount of time, in seconds, to wait between checks.
      * defaults to one.
      */
    double delay;
    
    virtual void render();
    virtual int event(const SDL_Event *ev);
    virtual void iterate(double dt, double time);
    
    /** \brief Returns 1 iff the specified asset is assigned to us. */
    int operator[](const IAsset *asset) const;
    
    /** \brief Appends an asset iff it isn't already assigned to us. */
    void operator+=(IAsset *asset);
    
    /** \brief Removes an asset from our sensitivity list. */
    void operator-=(IAsset *asset);
    
};


#endif