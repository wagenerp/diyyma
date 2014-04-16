
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
#include "diyyma/texture.h"

#include "SDL/SDL.h"
#include "GL/glew.h"

class IComponent : public RCObject {
  public:
    virtual ~IComponent() { }
    virtual void render() =0;
    virtual int event(const SDL_Event *ev) =0;
    virtual void iterate(double dt, double time) =0;
    
};

/** \brief FPS-like camera motion using mouse and WSAD. */
class FPSCameraComponent : public IComponent {
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
    
  public:
    FPSCameraComponent();
    ~FPSCameraComponent();
    virtual void render();
    virtual int event(const SDL_Event *ev);
    virtual void iterate(double dt, double time);
    
    /** \brief Recomputes the view and view-projection matrices.
      *
      * This is performed automatically on render() so you usually do
      * not have to bother with this. 
      *
      * Please note that the projection matrix is *not* computed here, but 
      * may be set manually or computed in the constructor.
      */
    void compute();
};


struct AssetRecord {
  IAsset *asset;
  timestamp_t lastModified;
  
  AssetRecord(IAsset *a, timestamp_t t) : asset(a), lastModified(t) { }
};

class AssetReloader : public IComponent {
  private:
    AssetRecord *_asset_v;
    size_t       _asset_n;
    double       _tCheck;
  public:
    AssetReloader();
    AssetReloader(IAsset *asset);
    ~AssetReloader();
    
    double delay;
    
    virtual void render();
    virtual int event(const SDL_Event *ev);
    virtual void iterate(double dt, double time);
    

    int operator[](const IAsset *asset) const;
    void operator+=(IAsset *asset);
    void operator-=(IAsset *asset);
    
};


#endif