#include "plugin.hpp"

struct Broadcast : Module {
  enum ParamId {
    CLICK_LISTEN_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    LIVE_1_INPUT,
    LIVE_2_INPUT,
    BROADCAST_1_INPUT,
    BROADCAST_2_INPUT,
    AUDITION_INPUT,
    CLICK_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    MONITOR_1_OUTPUT,
    MONITOR_2_OUTPUT,
    BROADCAST_1_OUTPUT,
    BROADCAST_2_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  dsp::SlewLimiter fade;

  Broadcast() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(CLICK_LISTEN_PARAM, 0.f, 1.f, 1.f, "Click level");
    configInput(LIVE_1_INPUT, "");
    configInput(LIVE_2_INPUT, "");
    configInput(BROADCAST_1_INPUT, "");
    configInput(BROADCAST_2_INPUT, "");
    configInput(AUDITION_INPUT, "");
    configInput(CLICK_INPUT, "");
    configOutput(MONITOR_1_OUTPUT, "");
    configOutput(MONITOR_2_OUTPUT, "");
    configOutput(BROADCAST_1_OUTPUT, "");
    configOutput(BROADCAST_2_OUTPUT, "");

    configBypass(LIVE_1_INPUT, BROADCAST_1_OUTPUT);
    configBypass(LIVE_2_INPUT, BROADCAST_2_OUTPUT);
    configBypass(BROADCAST_1_INPUT, MONITOR_1_OUTPUT);
    configBypass(BROADCAST_2_INPUT, MONITOR_2_OUTPUT);

    fade.setRiseFall(100.f, 1.f);
  }

  void process(const ProcessArgs &args) override {
    fade.process(args.sampleTime, inputs[AUDITION_INPUT].getVoltage() / 10.f);
    float audition = fade.out;

    float click = inputs[CLICK_INPUT].getVoltage() * params[CLICK_LISTEN_PARAM].getValue();

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

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.532, 29.904)), module, Broadcast::CLICK_LISTEN_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.948, 14.579)), module, Broadcast::LIVE_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.532, 14.579)), module, Broadcast::LIVE_2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.948, 29.904)), module, Broadcast::CLICK_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.948, 46.309)), module, Broadcast::BROADCAST_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.532, 46.309)), module, Broadcast::BROADCAST_2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.948, 66.417)), module, Broadcast::AUDITION_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.625, 91.782)), module, Broadcast::MONITOR_1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.855, 91.782)), module, Broadcast::MONITOR_2_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.361, 112.357)), module, Broadcast::BROADCAST_1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.119, 112.357)), module, Broadcast::BROADCAST_2_OUTPUT));
  }
};

Model *modelBroadcast = createModel<Broadcast, BroadcastWidget>("Broadcast");
