#include "plugin.hpp"
#include "./controls.hpp"

struct Spray : Module {
  enum ParamId {
    DELAY_TIME_PARAM,
    VOICES_PARAM,
    BIAS_PARAM,
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

  Spray() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(DELAY_TIME_PARAM, 0.f, 1.f, 0.f, "Max time", " s");
    configParam(VOICES_PARAM, 1.f, 16.f, 4.f, "Voices");
    configParam(BIAS_PARAM, 1.f, 32.f, 1.f, "Bias");
    configInput(TRIGGER_INPUT, "Trigger");
    configInput(DELAY_TIME_INPUT, "Time attenuator");
    configOutput(TRIGGER_OUTPUT, "Poly trigger");
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

    if (inputTrig.process(getInput(TRIGGER_INPUT).getVoltageSum() > 0.f)) {
      timer.reset();
      for (size_t i = 0; i < channels; i++) {
        armed[i] = true;
        // delay[i] = std::pow(random::uniform(), getParam(BIAS_PARAM).getValue()) * getParam(DELAY_TIME_PARAM).getValue() * (getInput(DELAY_TIME_INPUT).isConnected() ? getInput(DELAY_TIME_INPUT).getVoltage() / 10.f : 1.f);
        delay[i] = random::uniform() * getParam(DELAY_TIME_PARAM).getValue() * (getInput(DELAY_TIME_INPUT).isConnected() ? getInput(DELAY_TIME_INPUT).getVoltage() / 10.f : 1.f);
      }
    };
  }
};

struct SprayWidget : ModuleWidget {
  SprayWidget(Spray *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Spray.svg")));

    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));

    addParam(createParamCentered<LilacKnob>(mm2px(Vec(7.62, 56.622)), module, Spray::DELAY_TIME_PARAM));
    addParam(createParamCentered<LilacKnob>(mm2px(Vec(7.62, 91.018)), module, Spray::VOICES_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 33.352)), module, Spray::TRIGGER_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 69.801)), module, Spray::DELAY_TIME_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 112.359)), module, Spray::TRIGGER_OUTPUT));
  }
};

Model *modelSpray = createModel<Spray, SprayWidget>("Spray");
