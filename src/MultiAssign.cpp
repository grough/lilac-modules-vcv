#include "plugin.hpp"
#include "./controls.hpp"

struct MultiAssign : Module {
  enum ParamId {
    CHANNELS_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    GATE_1_INPUT,
    PITCH_1_INPUT,
    GATE_2_INPUT,
    PITCH_2_INPUT,
    RESET_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    GATE_OUTPUT,
    PITCH_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  MultiAssign() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(CHANNELS_PARAM, 0.f, 1.f, 0.f, "");
    configInput(GATE_1_INPUT, "");
    configInput(PITCH_1_INPUT, "");
    configInput(GATE_2_INPUT, "");
    configInput(PITCH_2_INPUT, "");
    configInput(RESET_INPUT, "");
    configOutput(GATE_OUTPUT, "");
    configOutput(PITCH_OUTPUT, "");
  }

  void process(const ProcessArgs &args) override {
  }
};

struct MultiAssignWidget : ModuleWidget {
  MultiAssignWidget(MultiAssign *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/MultiAssign.svg")));

    // addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, 0)));
    // addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    // addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    // addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<LilacKnob>(mm2px(Vec(7.62, 12.172)), module, MultiAssign::CHANNELS_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 29.648)), module, MultiAssign::GATE_1_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 42.348)), module, MultiAssign::PITCH_1_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 58.752)), module, MultiAssign::GATE_2_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 71.452)), module, MultiAssign::PITCH_2_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 87.327)), module, MultiAssign::RESET_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 103.893)), module, MultiAssign::GATE_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 119.768)), module, MultiAssign::PITCH_OUTPUT));
  }
};

Model *modelMultiAssign = createModel<MultiAssign, MultiAssignWidget>("MultiAssign");