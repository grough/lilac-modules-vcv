#include "plugin.hpp"
#include "./controls.hpp"

struct Counter : Module {
  enum ParamId {
    COUNT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    CLOCK_INPUT,
    RESET_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    GATE_OUTPUT,
    END_OF_CYCLE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  dsp::ClockDivider uiDivider;
  dsp::BooleanTrigger clockTrig;
  dsp::BooleanTrigger resetTrig;
  dsp::PulseGenerator endOfCycle;
  float gate = 0.f;
  bool init = true;
  int limit = 0;
  int count = 0;
  bool autoReset = false;

  Counter() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(COUNT_PARAM, 1.f, 128.f, 5.f, "Count");
    configInput(CLOCK_INPUT, "Clock");
    configInput(RESET_INPUT, "Reset");
    configOutput(GATE_OUTPUT, "Gate");
    configOutput(END_OF_CYCLE_OUTPUT, "End of cycle");
    paramQuantities[COUNT_PARAM]->snapEnabled = true;
    uiDivider.setDivision(1024);
  }

  void process(const ProcessArgs &args) override {
    if (uiDivider.process()) {
      limit = static_cast<float>(getParam(COUNT_PARAM).getValue());
    }

    if (resetTrig.process(getInput(RESET_INPUT).getVoltage() > 0.f)) {
      count = 0;
      gate = 0.f;
      init = true;
    }

    if (init && clockTrig.process(getInput(CLOCK_INPUT).getVoltageSum() > 0.f)) {
      gate = 10.f;
      count = count + 1;
      if (count >= limit) {
        count = 0;
        gate = 0.f;
        init = autoReset;
        endOfCycle.trigger();
      }
    }

    getOutput(GATE_OUTPUT).setVoltage(gate);
    getOutput(END_OF_CYCLE_OUTPUT).setVoltage(endOfCycle.process(args.sampleTime) * 10.f);
  }
};

struct CounterWidget : ModuleWidget {
  CounterWidget(Counter *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Counter.svg")));

    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<LilacKnob>(mm2px(Vec(7.62, 28.047)), module, Counter::COUNT_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 47.669)), module, Counter::CLOCK_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 65.661)), module, Counter::RESET_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 94.897)), module, Counter::GATE_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 112.209)), module, Counter::END_OF_CYCLE_OUTPUT));
  }

  void appendContextMenu(Menu *menu) override {
    Counter *module = getModule<Counter>();
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("Reset automatically", "", &module->autoReset));
  }
};

Model *modelCounter = createModel<Counter, CounterWidget>("Counter");
