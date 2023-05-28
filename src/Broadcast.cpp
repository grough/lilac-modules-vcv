#include "plugin.hpp"

struct Broadcast : Module {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    LIVE_1_INPUT,
    LIVE_2_INPUT,
    BROADCAST_1_INPUT,
    BROADCAST_2_INPUT,
    AUDITION_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    AUDITION_OUTPUT,
    MONITOR_1_OUTPUT,
    MONITOR_2_OUTPUT,
    BROADCAST_1_OUTPUT,
    BROADCAST_2_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Broadcast() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(LIVE_1_INPUT, "");
    configInput(LIVE_2_INPUT, "");
    configInput(BROADCAST_1_INPUT, "");
    configInput(BROADCAST_2_INPUT, "");
    configInput(AUDITION_INPUT, "");
    configOutput(MONITOR_1_OUTPUT, "");
    configOutput(MONITOR_2_OUTPUT, "");
    configOutput(BROADCAST_1_OUTPUT, "");
    configOutput(BROADCAST_2_OUTPUT, "");
  }

  void process(const ProcessArgs &args) override {
    float audition = inputs[AUDITION_INPUT].getVoltage() / 10.f;

    // Monitor output plays both live input and performance when audition is high. Plays performance when audition is low.
    outputs[MONITOR_1_OUTPUT].setVoltage(audition * inputs[LIVE_1_INPUT].getVoltage() + inputs[BROADCAST_1_INPUT].getVoltage());
    outputs[MONITOR_2_OUTPUT].setVoltage(audition * inputs[LIVE_2_INPUT].getVoltage() + inputs[BROADCAST_2_INPUT].getVoltage());

    // This output feeds into a performance patch. The performance patch feeds into broadcast audio device
    outputs[BROADCAST_1_OUTPUT].setVoltage((1.f - audition) * inputs[LIVE_1_INPUT].getVoltage());
    outputs[BROADCAST_2_OUTPUT].setVoltage((1.f - audition) * inputs[LIVE_2_INPUT].getVoltage());
  }
};

struct BroadcastWidget : ModuleWidget {
  BroadcastWidget(Broadcast *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Broadcast.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.948, 15.638)), module, Broadcast::LIVE_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.532, 15.638)), module, Broadcast::LIVE_2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.948, 34.667)), module, Broadcast::BROADCAST_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.532, 34.667)), module, Broadcast::BROADCAST_2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 60.596)), module, Broadcast::AUDITION_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.625, 91.782)), module, Broadcast::MONITOR_1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.855, 91.782)), module, Broadcast::MONITOR_2_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.361, 112.357)), module, Broadcast::BROADCAST_1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.119, 112.357)), module, Broadcast::BROADCAST_2_OUTPUT));
  }
};

Model *modelBroadcast = createModel<Broadcast, BroadcastWidget>("Broadcast");
