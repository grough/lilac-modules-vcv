#include "plugin.hpp"
#include "./components.hpp"

struct Accumulator : Module {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    RATE_INPUT,
    RESET_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    SUM_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  float sums[16] = {0.0f};
  float channels = 0;
  dsp::BooleanTrigger resetTrigger[16];

  Accumulator() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(RATE_INPUT, "Rate");
    configInput(RESET_INPUT, "Reset");
    configOutput(SUM_OUTPUT, "Total");
  }

  void process(const ProcessArgs &args) override {
    if (inputs[RATE_INPUT].getChannels() > channels) {
      channels = inputs[RATE_INPUT].getChannels();
    }

    outputs[SUM_OUTPUT].setChannels(channels);

    for (int c = 0; c < channels; c++) {
      sums[c] += inputs[RATE_INPUT].getVoltage(c) * args.sampleTime;
      outputs[SUM_OUTPUT].setVoltage(sums[c], c);
    }

    if (inputs[RESET_INPUT].isMonophonic()) {
      if (resetTrigger[0].process(inputs[RESET_INPUT].getVoltage() > 0.0f)) {
        for (int c = 0; c < 16; c++) {
          sums[c] = 0.0f;
        }
        channels = 0;
      }
    }

    if (inputs[RESET_INPUT].isPolyphonic()) {
      for (int c = 0; c < inputs[RESET_INPUT].getChannels(); c++) {
      if (resetTrigger[c].process(inputs[RESET_INPUT].getVoltage(c) > 0.0f)) {
          sums[c] = 0.0f;
          if (c == channels - 1) {
            channels--;
          }
        }
      }
    }
  }
};

struct AccumulatorWidget : ModuleWidget {

  AccumulatorWidget(Accumulator *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Accumulator.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addInput(createInputCentered<V1Port>(mm2px(Vec(7.62, 59.123)), module, Accumulator::RATE_INPUT));
    addInput(createInputCentered<V1Port>(mm2px(Vec(7.62, 81.454)), module, Accumulator::RESET_INPUT));

    addOutput(createOutputCentered<V1Port>(mm2px(Vec(7.62, 112.359)), module, Accumulator::SUM_OUTPUT));
  }
};

Model *modelAccumulator = createModel<Accumulator, AccumulatorWidget>("Accumulator");
