#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
  pluginInstance = p;
  p->addModel(modelAccumulator);
  p->addModel(modelAccumulatorSingle);
  p->addModel(modelComparator);
  p->addModel(modelBroadcast);
}
