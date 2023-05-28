#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
  pluginInstance = p;
  p->addModel(modelAccumulator);
  p->addModel(modelComparator);
  p->addModel(modelRounder);
  p->addModel(modelBroadcast);
}
