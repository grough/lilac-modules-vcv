#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
  pluginInstance = p;
  p->addModel(modelAccumulator);
  p->addModel(modelAccumulatorSingle);
  p->addModel(modelComparator);
  p->addModel(modelBroadcast);
  p->addModel(modelSpray);
  p->addModel(modelCounter);
  p->addModel(modelPitchGate);
}
