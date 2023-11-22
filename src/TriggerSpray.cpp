#include "plugin.hpp"
#include "./controls.hpp"

struct TriggerSpray : Module {
  enum ParamId {
    DELAY_TIME_PARAM,
    VOICES_PARAM,
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
  dsp::ClockDivider uiDivider;
  bool armed[16];
  float delay[16];
  int channels = 4;

  TriggerSpray() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(DELAY_TIME_PARAM, 0.f, 1.f, 0.f, "Max time", " s");
    configParam(VOICES_PARAM, 1.f, 16.f, 4.f, "Voices");
    configInput(TRIGGER_INPUT, "Trigger");
    configInput(DELAY_TIME_INPUT, "Time attenuator");
    configOutput(TRIGGER_OUTPUT, "");
    paramQuantities[VOICES_PARAM]->snapEnabled = true;
    uiDivider.setDivision(1024);
  }

  void process(const ProcessArgs &args) override {
    if (uiDivider.process()) {
      channels = static_cast<float>(getParam(VOICES_PARAM).getValue());
      getOutput(TRIGGER_OUTPUT).setChannels(channels);
    }

    timer.process(args.sampleTime);

    for (size_t i = 0; i < channels; i++) {
      if (armed[i] && timer.getTime() > delay[i]) {
        trigs[i].trigger();
        armed[i] = false;
      }
      getOutput(TRIGGER_OUTPUT).setVoltage(trigs[i].process(args.sampleTime) * 10.f, i);
    }

    if (inputTrig.process(getInput(TRIGGER_INPUT).getVoltage() > 0.f)) {
      timer.reset();
      for (size_t i = 0; i < channels; i++) {
        armed[i] = true;
        delay[i] = random::uniform() * getParam(DELAY_TIME_PARAM).getValue() * (getInput(DELAY_TIME_INPUT).isConnected() ? getInput(DELAY_TIME_INPUT).getVoltage() / 10.f : 1.f);
      }
    };
  }
};

struct TriggerSprayWidget : ModuleWidget {
  TriggerSprayWidget(TriggerSpray *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/TriggerSpray.svg")));

    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));

    addParam(createParamCentered<LilacKnob>(mm2px(Vec(7.62, 37.043)), module, TriggerSpray::DELAY_TIME_PARAM));
    addParam(createParamCentered<LilacKnob>(mm2px(Vec(7.62, 74.084)), module, TriggerSpray::VOICES_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 15.89)), module, TriggerSpray::TRIGGER_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 50.222)), module, TriggerSpray::DELAY_TIME_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 112.359)), module, TriggerSpray::TRIGGER_OUTPUT));
  }
};

Model *modelTriggerSpray = createModel<TriggerSpray, TriggerSprayWidget>("TriggerSpray");
