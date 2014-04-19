
#include "diyyma/renderpass.h"

SceneNodeRenderPass::SceneNodeRenderPass() {
  ARRAY_INIT(_nodes);
  _context.MV.setIdentity();
  _context.MVP.setIdentity();
}

SceneNodeRenderPass::~SceneNodeRenderPass() {
  int idx;
  IRenderableSceneNode **pnode;
  FOREACH(idx,pnode,_nodes) {
    (*pnode)->drop();
  }
  ARRAY_DESTROY(_nodes);
}

void SceneNodeRenderPass::operator+=(IRenderableSceneNode *node) {
  node->grab();
  APPEND(_nodes,node);
}

void SceneNodeRenderPass::sortByDistance(const Vector3f &origin) {
  int idx;
  IRenderableSceneNode **pnode;
  int sorted=1;
  Matrixf transform;
  
  if (_distance_n<_nodes_n) 
    ARRAY_SETSIZE(_distance,_nodes_n);
  
  FOREACH(idx,pnode,_nodes) {
    transform=(*pnode)->absTransform();
    _distance_v[idx]=(
      Vector3f(transform.a14,transform.a24,transform.a34)
      -origin).sqr();
    
    if (sorted && (idx>0)&&(_distance_v[idx-1]>_distance_v[idx]))
      sorted=0;
  }
  
  if (!sorted) 
    sortByKeys<IRenderableSceneNode*,double>(_nodes_v,_distance_v,_nodes_n);
}

void SceneNodeRenderPass::sortByDistance() {
  sortByDistance(Vector3f(_context.MV.a14,_context.MV.a24,_context.MV.a34));
}


void SceneNodeRenderPass::render() {
  int idx;
  IRenderableSceneNode **pnode;
  
  FOREACH(idx,pnode,_nodes) {
    (*pnode)->render(_context);
  }
}
int SceneNodeRenderPass::event(const SDL_Event *ev) {
  return 0;
}

void SceneNodeRenderPass::iterate(double dt, double time) {

}