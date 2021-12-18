#include "plugin.hpp"
#include "./components.hpp"

struct Accumulator : Module {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    RATE_1_INPUT,
    RESET_1_INPUT,
    RATE_2_INPUT,
    RESET_2_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    SUM_1_OUTPUT,
    SUM_2_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  int rateI[2];
  int resetI[2];
  int sumO[2];

  float sums[2][16] = {{0.0f}};
  float channels[2] = {0};
  dsp::BooleanTrigger resetTrigger[2][16];

  Accumulator() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(RATE_1_INPUT, "Growth rate");
    configInput(RATE_2_INPUT, "Growth rate");
    configInput(RESET_1_INPUT, "Reset");
    configInput(RESET_2_INPUT, "Reset");
    configOutput(SUM_1_OUTPUT, "Total");
    configOutput(SUM_2_OUTPUT, "Total");

    rateI[0] = RATE_1_INPUT;
    rateI[1] = RATE_2_INPUT;
    resetI[0] = RESET_1_INPUT;
    resetI[1] = RESET_2_INPUT;
    sumO[0] = SUM_1_OUTPUT;
    sumO[1] = SUM_2_OUTPUT;
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_t *configsJ = json_array();
    for (size_t i = 0; i < 2; i++) {
      json_t *sumJ = json_object();
      json_object_set_new(sumJ, "channels", json_integer(channels[i]));
      json_array_append_new(configsJ, sumJ);
      json_t *sumsJ = json_array();
      for (size_t c = 0; c < channels[i]; c++) {
        json_array_append_new(sumsJ, json_real(sums[i][c]));
      }
      json_object_set_new(sumJ, "sums", sumsJ);
    }
    json_object_set_new(rootJ, "accumulator", configsJ);
    return rootJ;
  }

  void dataFromJson(json_t *root) override {
    json_t *configsJ = json_object_get(root, "accumulator");
    if (configsJ) {
      size_t i;
      json_t *configJ;
      json_array_foreach(configsJ, i, configJ) {
        json_t *channelsJ = json_object_get(configJ, "channels");
        channels[i] = json_number_value(channelsJ);
        json_t *sumsJ = json_object_get(configJ, "sums");
        if (sumsJ) {
          size_t c;
          json_t *sumJ;
          json_array_foreach(sumsJ, c, sumJ) {
            sums[i][c] = json_number_value(sumJ);
          }
        }
      }
    }
  }

  void process(const ProcessArgs &args) override {
    for (int i = 0; i < 2; i++) {
      if (inputs[rateI[i]].getChannels() > channels[i]) {
        channels[i] = inputs[rateI[i]].getChannels();
      }

      if (channels[i] == 0) {
        return;
      }

      outputs[sumO[i]].setChannels(channels[i]);

      for (int c = 0; c < channels[i]; c++) {
        sums[i][c] += inputs[rateI[i]].getVoltage(c) * args.sampleTime;
        outputs[sumO[i]].setVoltage(sums[i][c], c);
      }

      if (inputs[resetI[i]].isMonophonic()) {
        if (resetTrigger[i][0].process(inputs[resetI[i]].getVoltage() > 0.0f)) {
          for (int c = 0; c < 16; c++) {
            sums[i][c] = 0.0f;
          }
          channels[i] = 0;
        }
      }

      if (inputs[resetI[i]].isPolyphonic()) {
        for (int c = 0; c < inputs[resetI[i]].getChannels(); c++) {
          if (resetTrigger[i][c].process(inputs[resetI[i]].getVoltage(c) > 0.0f)) {
            sums[i][c] = 0.0f;
            if (c == channels[i] - 1) {
              channels[i]--;
            }
          }
        }
      }
    }
  }

  void onReset() override {
    for (int i = 0; i < 2; i++) {
      for (int c = 0; c < 16; c++) {
        channels[i] = 0;
        sums[i][c] = 0.0f;
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

    addInput(createInputCentered<V1Port>(mm2px(Vec(7.62, 20.929)), module, Accumulator::RATE_1_INPUT));
    addInput(createInputCentered<V1Port>(mm2px(Vec(7.62, 39.429)), module, Accumulator::RESET_1_INPUT));
    addInput(createInputCentered<V1Port>(mm2px(Vec(7.62, 76.429)), module, Accumulator::RATE_2_INPUT));
    addInput(createInputCentered<V1Port>(mm2px(Vec(7.62, 94.929)), module, Accumulator::RESET_2_INPUT));

    addOutput(createOutputCentered<V1Port>(mm2px(Vec(7.62, 56.857)), module, Accumulator::SUM_1_OUTPUT));
    addOutput(createOutputCentered<V1Port>(mm2px(Vec(7.62, 112.357)), module, Accumulator::SUM_2_OUTPUT));
  }
};

Model *modelAccumulator = createModel<Accumulator, AccumulatorWidget>("Accumulator");
