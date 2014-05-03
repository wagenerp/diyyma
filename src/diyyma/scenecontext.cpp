
#include "diyyma/config.h"
#include "diyyma/scenecontext.h"
#include "diyyma/util.h"

ISceneContextReferrer::ISceneContextReferrer() :
  _contextSource(0) {
}

ISceneContextReferrer::~ISceneContextReferrer() {
  if (_contextSource) _contextSource->drop();
}


ISceneContextSource *ISceneContextReferrer::contextSource() { 
  return _contextSource; 
}
void ISceneContextReferrer::setContextSource(ISceneContextSource *s) {
  if (s==_contextSource) return;
  if (_contextSource) _contextSource->drop();
  _contextSource=s;
  if (_contextSource) _contextSource->grab();
}

