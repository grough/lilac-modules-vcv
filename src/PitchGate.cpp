#include "plugin.hpp"

struct TimedGate {
  dsp::Timer timer;
  float duration;
  bool open = false;

  float process(float deltaTime) {
    timer.process(deltaTime);
    if (timer.getTime() >= duration) {
      open = false;
    }
    return open ? 1.f : 0.f;
  }

  void trigger(float duration) {
    this->duration = duration;
    open = true;
    timer.reset();
  }
};

struct Pitch2Gate {
  dsp::SchmittTrigger schmitt;
  TimedGate gate;

  float process(float deltaTime, float voltsPerOctave, float trigger) {
    if (schmitt.process(trigger)) {
      gate.trigger(1.f / (dsp::FREQ_C4 * dsp::approxExp2_taylor5(voltsPerOctave)));
    };
    return gate.process(deltaTime);
  }
};

struct PitchGate : Module {
  enum ParamId { PARAMS_LEN };
  enum InputId {
    PITCH_1_INPUT,
    TRIG_1_INPUT,
    PITCH_2_INPUT,
    TRIG_2_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    GATE_1_OUTPUT,
    GATE_2_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId { LIGHTS_LEN };

  dsp::ClockDivider logDivider;
  dsp::ClockDivider uiDivider;

  Pitch2Gate p2g[16];
  int channels;

  PitchGate() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(PITCH_1_INPUT, "");
    configInput(TRIG_1_INPUT, "");
    configInput(PITCH_2_INPUT, "");
    configInput(TRIG_2_INPUT, "");
    configOutput(GATE_1_OUTPUT, "");
    configOutput(GATE_2_OUTPUT, "");

    logDivider.setDivision(8192);
    uiDivider.setDivision(1024);

    channels = 1;
  }

  void debug() {
    // DEBUG("Freq %f Period %f FInput %f", p2g.freq, p2g.period, getInput(PITCH_1_INPUT).getVoltage());
  }

  void process(const ProcessArgs &args) override {
    if (uiDivider.process()) {
      channels = std::max(getInput(PITCH_1_INPUT).getChannels(), getInput(TRIG_1_INPUT).getChannels());
      getOutput(GATE_1_OUTPUT).setChannels(channels);
    }

    for (size_t channel = 0; channel < channels; channel++) {
      getOutput(GATE_1_OUTPUT).setVoltage(p2g[channel].process(args.sampleTime, getInput(PITCH_1_INPUT).getVoltage(channel), getInput(TRIG_1_INPUT).getVoltage(channel)) * 10.f, channel);
    }

    if (logDivider.process()) {
      debug();
    }
  }
};

struct PitchGateWidget : ModuleWidget {
  PitchGateWidget(PitchGate *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/PitchGate.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 20.929)), module, PitchGate::PITCH_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 39.429)), module, PitchGate::TRIG_1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 76.429)), module, PitchGate::PITCH_2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 94.929)), module, PitchGate::TRIG_2_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 56.857)), module, PitchGate::GATE_1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 112.357)), module, PitchGate::GATE_2_OUTPUT));
  }
};

Model *modelPitchGate = createModel<PitchGate, PitchGateWidget>("PitchGate");