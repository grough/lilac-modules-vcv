#include "plugin.hpp"
#include "./controls.hpp"

struct TriggerSpray : Module {
  enum ParamId {
    DELAY_TIME_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    TRIGGER_INPUT,
    DELAY_TIME_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    TRIGGER_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  dsp::BooleanTrigger inputTrig;
  dsp::Timer timer;
  dsp::PulseGenerator trigs[16];
  bool armed[16];
  float delay[16];

  TriggerSpray() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(DELAY_TIME_PARAM, 0.f, 1.f, 0.f, "");
    configInput(TRIGGER_INPUT, "");
    configInput(DELAY_TIME_INPUT, "");
    configOutput(TRIGGER_OUTPUT, "");
  }

  void process(const ProcessArgs &args) override {
    getOutput(TRIGGER_OUTPUT).setChannels(16);
    timer.process(args.sampleTime);

    for (size_t i = 0; i < 16; i++) {
      if (armed[i] && timer.getTime() > delay[i]) {
        trigs[i].trigger();
        armed[i] = false;
      }
      getOutput(TRIGGER_OUTPUT).setVoltage(trigs[i].process(args.sampleTime) * 10.f, i);
    }

    if (inputTrig.process(getInput(TRIGGER_INPUT).getVoltage() > 0.f)) {
      timer.reset();
      for (size_t i = 0; i < 16; i++) {
        armed[i] = true;
        delay[i] = random::uniform() * getParam(DELAY_TIME_PARAM).getValue() * (getInput(DELAY_TIME_INPUT).isConnected() ? (getInput(DELAY_TIME_INPUT).getVoltage() / 10.f) : 1.f);
      }
    };
  }
};

struct TriggerSprayWidget : ModuleWidget {
  TriggerSprayWidget(TriggerSpray *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/TriggerSpray.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<LilacKnob>(mm2px(Vec(7.62, 37.043)), module, TriggerSpray::DELAY_TIME_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 15.89)), module, TriggerSpray::TRIGGER_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 50.222)), module, TriggerSpray::DELAY_TIME_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 112.359)), module, TriggerSpray::TRIGGER_OUTPUT));
  }
};

Model *modelTriggerSpray = createModel<TriggerSpray, TriggerSprayWidget>("TriggerSpray");