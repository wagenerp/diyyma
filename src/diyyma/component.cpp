
/** \file component.cpp
  * \author Peter Wagener
  * \brief Several component implementations
  *
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include <SDL2/SDL.h>
#include "GL/glew.h"

#include "diyyma/scenegraph.h"
#include "diyyma/component.h"
#include "diyyma/util.h"


FPSCameraComponent::FPSCameraComponent() { 
  _time=0;
  speed=1;
  mouseSensitivity=1;
  _keys=0;
  _mouseFlight=0;
  pos.set(0,0,0);
  angles.set(0,0,0);
  P.setPerspective(80,1.0/0.75,0.1,1000);
  compute();
}
FPSCameraComponent::~FPSCameraComponent() { 
}

void FPSCameraComponent::compute() {
  V=
    Matrixf::RotationX(angles.x)
    *Matrixf::RotationY(angles.y)
    *Matrixf::RotationZ(angles.z)
    *Matrixf::Translation(-pos.x,-pos.y,-pos.z);
  VP=P*V;
}

Vector3f FPSCameraComponent::up() {
	return Vector3f(V.row2()).normal();
}

Vector3f FPSCameraComponent::view() {
	return -Vector3f(V.row3()).normal();
}

void FPSCameraComponent::render() {
  
}

int FPSCameraComponent::event(const SDL_Event *ev) {
  int i=-1;
  switch(ev->type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      switch(ev->key.keysym.sym) {
        case SDLK_w:     i=0; break;
        case SDLK_s:     i=1; break;
        case SDLK_a:     i=2; break;
        case SDLK_d:     i=3; break;
        case SDLK_SPACE: i=4; break;
        case SDLK_c:     i=5; break;
        case SDLK_RIGHT: i=6; break;
        case SDLK_LEFT:  i=7; break;
        case SDLK_UP:    i=8; break;
        case SDLK_DOWN:  i=9; break;
        case SDLK_LSHIFT: i=10; break;
        case SDLK_LALT:   i=11; break;
      }
      if (i>-1) {
        _keys|=1<<i;
        if (ev->type==SDL_KEYUP) _keys^=1<<i;
        return true;
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      if (ev->button.button==SDL_BUTTON_LEFT) { 
        _mouseFlight++;
        SDL_SetRelativeMouseMode((SDL_bool)(_mouseFlight>0));
        return true; 
      }
      break;
    case SDL_MOUSEBUTTONUP:
      if (ev->button.button==SDL_BUTTON_LEFT) { 
        _mouseFlight--;
        SDL_SetRelativeMouseMode((SDL_bool)(_mouseFlight>0));
        return true; 
      }
      break;
    case SDL_MOUSEMOTION:
      if (_mouseFlight) {
        angles.z+=ev->motion.xrel*0.01*mouseSensitivity;
        angles.y-=ev->motion.yrel*0.01*mouseSensitivity;;
        
        if (angles.y> 1.57) angles.y= 1.57;
        if (angles.y<-1.57) angles.y=-1.57;
        compute();
      }
      break;
  }
  return 0;
}

void FPSCameraComponent::iterate(double dt, double time) {
  Vector3f v;
  float s;
  Matrixf VInv;
  _time=time;
  
  if (_keys) {
    s=dt*12*speed;
    
    if (_keys&0x400) s*=50;
    if (_keys&0x800) s*=0.4;
    
    VInv=V.inverse();
    
    v.set(VInv.a11,VInv.a21,VInv.a31);
    if (_keys&0x01) pos+=v*s;
    if (_keys&0x02) pos-=v*s;
    
    v.set(VInv.a12,VInv.a22,VInv.a32);
    if (_keys&0x04) pos+=v*s;
    if (_keys&0x08) pos-=v*s;
    
    v.set(VInv.a13,VInv.a23,VInv.a33);
    if (_keys&0x10) pos+=v*s;
    if (_keys&0x20) pos-=v*s;
    
    s=dt*0.01;
    
    if (_keys&0x40) angles.z+=s;
    if (_keys&0x80) angles.z-=s;
    
    if (_keys&0x100) angles.y+=s;
    if (_keys&0x200) angles.y-=s;
    
    if (angles.y> 1.57) angles.y= 1.57;
    if (angles.y<-1.57) angles.y=-1.57;
    
    compute();
  }
  
}

SceneContext FPSCameraComponent::context() {
  SceneContext ctx;
  ctx.P=P;
  ctx.V=V;
  ctx.MV=V;
  ctx.MVP=VP;
  ctx.time=_time;
  ctx.camPos_w=pos;
  return ctx;
}

AssetReloader::AssetReloader() : 
  _asset_v(0), _asset_n(0), _tCheck(0), delay(1) { 
  
}
AssetReloader::AssetReloader(IAsset *asset) : 
  _asset_v(0), _asset_n(0), _tCheck(0), delay(1) {
  operator+=(asset);
}
AssetReloader::~AssetReloader() {
  size_t i;
  for(i=0;i<_asset_n;i++)
    _asset_v[i].asset->drop();
  free((void*)_asset_v);
}

void AssetReloader::render() { }
int AssetReloader::event(const SDL_Event *ev) { return 0; }
void AssetReloader::iterate(double dt, double time) {
  size_t i;
  int reloaded=0;
  timestamp_t tNew;
  if (time>_tCheck) {
    for(i=0;i<_asset_n;i++) {
      tNew=_asset_v[i].asset->filesTimestamp();
      if (tNew<=_asset_v[i].lastModified) continue;
      _asset_v[i].lastModified=tNew;
      _asset_v[i].asset->reload();
      reloaded++;
    }
    
    if (reloaded) fflush(stdout);
      
    _tCheck=time+delay;
  }
}

int AssetReloader::operator[](const IAsset *asset) const {
  size_t i;
  for(i=0;i<_asset_n;i++)
    if (_asset_v[i].asset==asset) return i;
  return -1;
}

void AssetReloader::operator+=(IAsset *asset) {
  if (operator[](asset)!=-1) { return; }
  _asset_v=(AssetRecord*)realloc(
    (void*)_asset_v,
    sizeof(AssetRecord)*(_asset_n+1));
  _asset_v[_asset_n++]=AssetRecord(asset, asset->filesTimestamp());
  asset->grab();
}

void AssetReloader::operator-=(IAsset *asset) {
  int idx;
  if ((idx=operator[](asset))==-1) return;
  asset->drop();
  _asset_v[idx]=_asset_v[--_asset_n];
}