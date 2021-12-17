#include "plugin.hpp"
#include "./components.hpp"

struct Comparator : Module {
  enum ParamId {
    A_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    A_INPUT,
    B_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    LESS_OUTPUT,
    EQUAL_OUTPUT,
    GREATER_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Comparator() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(A_PARAM, -10.0f, 10.0f, 0.0f, "A", "V");
    configInput(A_INPUT, "A");
    configInput(B_INPUT, "B");
    configOutput(LESS_OUTPUT, "A < B");
    configOutput(EQUAL_OUTPUT, "A = B");
    configOutput(GREATER_OUTPUT, "A > B");
  }

  void process(const ProcessArgs &args) override {
    int channels = inputs[A_INPUT].getChannels();

    if (inputs[B_INPUT].getChannels() > channels) {
      channels = inputs[B_INPUT].getChannels();
    }

    outputs[LESS_OUTPUT].setChannels(channels);
    outputs[EQUAL_OUTPUT].setChannels(channels);
    outputs[GREATER_OUTPUT].setChannels(channels);

    for (int c = 0; c < channels; c++) {
      outputs[LESS_OUTPUT].setVoltage(0.0f, c);
      outputs[EQUAL_OUTPUT].setVoltage(0.0f, c);
      outputs[GREATER_OUTPUT].setVoltage(0.0f, c);

      float a = params[A_PARAM].getValue();

      if (inputs[A_INPUT].isConnected()) {
        a = inputs[A_INPUT].getVoltage(c);
      }

      float b = inputs[B_INPUT].getVoltage(c);

      if (a < b) {
        outputs[LESS_OUTPUT].setVoltage(10.0f, c);
      }

      if (a == b) {
        outputs[EQUAL_OUTPUT].setVoltage(10.0f, c);
      }

      if (a > b) {
        outputs[GREATER_OUTPUT].setVoltage(10.0f, c);
      }
    }
  }
};

struct ComparatorWidget : ModuleWidget {
  ComparatorWidget(Comparator *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Comparator.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<WarmKnob>(mm2px(Vec(7.62, 22.902)), module, Comparator::A_PARAM));

    addInput(createInputCentered<V1Port>(mm2px(Vec(7.62, 34.16)), module, Comparator::A_INPUT));
    addInput(createInputCentered<V1Port>(mm2px(Vec(7.62, 54.256)), module, Comparator::B_INPUT));

    addOutput(createOutputCentered<V1Port>(mm2px(Vec(7.62, 75.534)), module, Comparator::LESS_OUTPUT));
    addOutput(createOutputCentered<V1Port>(mm2px(Vec(7.62, 93.947)), module, Comparator::EQUAL_OUTPUT));
    addOutput(createOutputCentered<V1Port>(mm2px(Vec(7.62, 112.359)), module, Comparator::GREATER_OUTPUT));
  }
};

Model *modelComparator = createModel<Comparator, ComparatorWidget>("Comparator");
