#include "plugin.hpp"
#include "controls.hpp"

struct AccumulatorSingle : Module {
  enum ParamId {
    RATE_PARAM,
    RESET_PARAM,
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
  dsp::BooleanTrigger resetButtonTrigger;
  dsp::BooleanTrigger resetTrigger[16];
  bool saveSumWithPatch = true;

  AccumulatorSingle() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(RATE_PARAM, -10.f, 10.f, 0.f, "Growth rate", "V/s");
    configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
    configInput(RATE_INPUT, "Rate attenuverter");
    configInput(RESET_INPUT, "Reset");
    configOutput(SUM_OUTPUT, "Sum");
  }

  void process(const ProcessArgs &args) override {
    getOutput(SUM_OUTPUT).setChannels(getInput(RATE_INPUT).getChannels());

    if (getOutput(SUM_OUTPUT).isConnected()) {
      // Rate knob acts as an attenuverter if rate input is connected
      if (getInput(RATE_INPUT).isConnected()) {
        for (int c = 0; c < getInput(RATE_INPUT).getChannels(); c++) {
          sums[c] += getParam(RATE_PARAM).getValue() * args.sampleTime * (getInput(RATE_INPUT).getVoltage(c) / 5.f);
          getOutput(SUM_OUTPUT).setVoltage(sums[c], c);
        }
      }
      // Rate knob controls the first channel if the rate input is connected
      else {
        sums[0] += getParam(RATE_PARAM).getValue() * args.sampleTime;
        getOutput(SUM_OUTPUT).setVoltage(sums[0]);
      }
    }

    if (resetButtonTrigger.process(getParam(RESET_PARAM).getValue() > 0.0f)) {
      for (int c = 0; c < 16; c++) {
        sums[c] = 0.0f;
      }
    }

    if (getInput(RESET_INPUT).isPolyphonic()) {
      for (int c = 0; c < getInput(RESET_INPUT).getChannels(); c++) {
        if (resetTrigger[c].process(getInput(RESET_INPUT).getVoltage(c) > 0.0f)) {
          sums[c] = 0.0f;
        }
      }
    } else {
      if (resetTrigger[0].process(getInput(RESET_INPUT).getVoltage() > 0.0f)) {
        for (int c = 0; c < 16; c++) {
          sums[c] = 0.0f;
        }
      }
    }
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_t *sumsJ = json_array();
    for (size_t c = 0; c < 16; c++) {
      json_array_append_new(sumsJ, json_real(sums[c]));
    }
    json_object_set_new(rootJ, "sums", sumsJ);
    json_object_set_new(rootJ, "saveSumWithPatch", json_boolean(saveSumWithPatch));
    return rootJ;
  }

  void dataFromJson(json_t *root) override {
    json_t *saveSumWithPatchJson = json_object_get(root, "saveSumWithPatch");
    if (saveSumWithPatchJson) {
      saveSumWithPatch = json_boolean_value(saveSumWithPatchJson);
      // Only load sum values if menu option is set
      if (saveSumWithPatch) {
        json_t *sumsJ = json_object_get(root, "sums");
        if (sumsJ) {
          size_t c;
          json_t *sumJ;
          json_array_foreach(sumsJ, c, sumJ) {
            sums[c] = json_number_value(sumJ);
          }
        }
      }
    }
  }
};

struct AccumulatorSingleWidget : ModuleWidget {
  AccumulatorSingleWidget(AccumulatorSingle *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/AccumulatorSingle.svg")));

    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<LilacKnob>(mm2px(Vec(7.62, 34.159)), module, AccumulatorSingle::RATE_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(7.62, 71.729)), module, AccumulatorSingle::RESET_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 46.859)), module, AccumulatorSingle::RATE_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 84.938)), module, AccumulatorSingle::RESET_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 112.357)), module, AccumulatorSingle::SUM_OUTPUT));
  }

  void appendContextMenu(Menu *menu) override {
    AccumulatorSingle *module = dynamic_cast<AccumulatorSingle *>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolMenuItem(
        "Save sum with patch", "",
        [=]() {
          return module->saveSumWithPatch;
        },
        [=](bool value) {
          module->saveSumWithPatch = value;
        }));
  }
};

Model *modelAccumulatorSingle = createModel<AccumulatorSingle, AccumulatorSingleWidget>("AccumulatorSingle");