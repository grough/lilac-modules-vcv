#include "plugin.hpp"

struct Rounder : Module {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    SOURCE_1_INPUT,
    SOURCE_2_INPUT,
    SOURCE_3_INPUT,
    SOURCE_4_INPUT,
    MAIN_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    MAIN_OUTPUT,
    // TRIG_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  InputId srcIns[4];
  float sources[64];
  dsp::ClockDivider logDiv;

  Rounder() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(SOURCE_1_INPUT, "Quantize source 1");
    configInput(SOURCE_2_INPUT, "Quantize source 2");
    configInput(SOURCE_3_INPUT, "Quantize source 3");
    configInput(SOURCE_4_INPUT, "Quantize source 4");
    configInput(MAIN_INPUT, "Main");
    configOutput(MAIN_OUTPUT, "Quantized");
    // configOutput(TRIG_OUTPUT, "");
    logDiv.setDivision(8192);
    srcIns[0] = SOURCE_1_INPUT;
    srcIns[1] = SOURCE_2_INPUT;
    srcIns[2] = SOURCE_3_INPUT;
    srcIns[3] = SOURCE_4_INPUT;
  }

  void process(const ProcessArgs &args) override {
    int srcCount = 0;
    for (size_t i = 0; i < 4; i++) {
      int channels = inputs[srcIns[i]].getChannels();
      if (channels > 0) {
        for (size_t c = 0; c < channels; c++) {
          sources[srcCount] = inputs[srcIns[i]].getVoltage(c);
          srcCount++;
        }
      }
    }

    int channels = inputs[MAIN_INPUT].getChannels();
    outputs[MAIN_OUTPUT].setChannels(channels);

    for (size_t chan = 0; chan < channels; chan++) {
      float minSrc = sources[0];
      float minDiff = std::numeric_limits<float>::max();
      float in = inputs[MAIN_INPUT].getVoltage(chan);
      for (size_t i = 0; i < srcCount; i++) {
        float src = sources[i];
        if (abs(in - src) < minDiff) {
          minDiff = abs(in - src);
          minSrc = src;
        }
      }
      outputs[MAIN_OUTPUT].setVoltage(minSrc, chan);
    }

    // if (logDiv.process()) {
    // 	DEBUG("…");
    // }
  }
};

struct RounderWidget : ModuleWidget {
  RounderWidget(Rounder *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Rounder.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 23.298)), module, Rounder::SOURCE_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 33.881)), module, Rounder::SOURCE_2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 44.465)), module, Rounder::SOURCE_3_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 55.048)), module, Rounder::SOURCE_4_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 76.215)), module, Rounder::MAIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 93.947)), module, Rounder::MAIN_OUTPUT));
    // addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 112.359)), module, Rounder::TRIG_OUTPUT));
  }
};

Model *modelRounder = createModel<Rounder, RounderWidget>("Rounder");
