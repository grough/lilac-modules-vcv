#include "plugin.hpp"
#include "quantize.hpp"

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
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  enum DistMode {
    NATURAL,
    BALANCED
  };
  enum RangeMode {
    UNCONSTRAINED,
    CONSTRAINED
  };

  dsp::ClockDivider logDiv;
  InputId srcIns[4];
  int srcCount;
  std::vector<float> sources;
  float inputRange[2];
  DistMode distMode;
  RangeMode rangeMode;

  Rounder() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(SOURCE_1_INPUT, "Source 1");
    configInput(SOURCE_2_INPUT, "Source 2");
    configInput(SOURCE_3_INPUT, "Source 3");
    configInput(SOURCE_4_INPUT, "Source 4");
    configInput(MAIN_INPUT, "Main");
    configOutput(MAIN_OUTPUT, "Quantized");
    configBypass(MAIN_INPUT, MAIN_OUTPUT);
    srcIns[0] = SOURCE_1_INPUT;
    srcIns[1] = SOURCE_2_INPUT;
    srcIns[2] = SOURCE_3_INPUT;
    srcIns[3] = SOURCE_4_INPUT;
    srcCount = 0;
    inputRange[0] = 0.0f;
    inputRange[1] = 10.0f;
    distMode = NATURAL;
    rangeMode = UNCONSTRAINED;
    logDiv.setDivision(8192);
  }

  json_t *dataToJson() override {
    json_t *root = json_object();
    json_object_set_new(root, "distMode", json_integer(distMode));
    return root;
  }

  void dataFromJson(json_t *root) override {
    json_t *distModeJson = json_object_get(root, "distMode");
    if (distModeJson) {
      distMode = (DistMode)json_number_value(distModeJson);
    }
  }

  void process(const ProcessArgs &args) override {
    sources.clear();
    for (size_t i = 0; i < 4; i++) {
      int channels = inputs[srcIns[i]].getChannels();
      if (channels > 0) {
        for (size_t c = 0; c < channels; c++) {
          sources.push_back(inputs[srcIns[i]].getVoltage(c));
        }
      }
    }

    int channels = inputs[MAIN_INPUT].getChannels();
    outputs[MAIN_OUTPUT].setChannels(channels);

    for (size_t c = 0; c < channels; c++) {
      outputs[MAIN_OUTPUT].setVoltage(mapInputToSource(c), c);
    }

    // if (logDiv.process()) {
    // 	DEBUG("…");
    // }
  }

  float mapInputToSource(int chan) {
    // Distribution mode: natural, balanced
    // Input range: natural, 0-10V, -5-5V, 0-1V, 0-2V
    float in = inputs[MAIN_INPUT].getVoltage(chan);

    if (distMode == NATURAL) {
      return quantize(sources, in);
    }
    if (distMode == BALANCED && rangeMode == UNCONSTRAINED) {
      return quantizeBalanced(sources, in);
    }
    if (distMode == BALANCED && rangeMode == CONSTRAINED) {
      return quantizeSwitch(sources, 0.f, 10.f, in);
    }
  }
};

struct RounderWidget : ModuleWidget {
  struct DistModeItem : MenuItem {
    Rounder *module;
    Rounder::DistMode distMode;

    void onAction(const event::Action &e) override {
      module->distMode = distMode;
    }
  };

  struct RangeItem : MenuItem {
    Rounder *module;
    Rounder::RangeMode rangeMode;
    float min;
    float max;

    void onAction(const event::Action &e) override {
      module->rangeMode = rangeMode;
      module->inputRange[0] = min;
      module->inputRange[1] = max;
    }
  };

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
  }

  void appendContextMenu(Menu *menu) override {
    Rounder *module = dynamic_cast<Rounder *>(this->module);

    menu->addChild(new MenuSeparator());

    MenuLabel *distModeLabel = new MenuLabel();
    distModeLabel->text = "Distribution";
    menu->addChild(distModeLabel);

    DistModeItem *naturalDistItem = new DistModeItem;
    naturalDistItem->text = "Natural";
    naturalDistItem->rightText = CHECKMARK(module->distMode == Rounder::NATURAL);
    naturalDistItem->distMode = Rounder::NATURAL;
    naturalDistItem->module = module;
    menu->addChild(naturalDistItem);

    DistModeItem *equalItem = new DistModeItem;
    equalItem->text = "Equal";
    equalItem->rightText = CHECKMARK(module->distMode == Rounder::BALANCED);
    equalItem->distMode = Rounder::BALANCED;
    equalItem->module = module;
    menu->addChild(equalItem);

    MenuLabel *rangeLabel = new MenuLabel();
    rangeLabel->text = "Input Range";
    menu->addChild(rangeLabel);

    RangeItem *naturalRangeItem = new RangeItem;
    naturalRangeItem->text = "Natural";
    // naturalRangeItem->rightText = CHECKMARK(module->distMode == Rounder::NATURAL);
    naturalRangeItem->rangeMode = Rounder::UNCONSTRAINED;
    naturalRangeItem->min = 0.0f;
    naturalRangeItem->max = 0.0f;
    naturalRangeItem->module = module;
    menu->addChild(naturalRangeItem);

    RangeItem *rangeItem2 = new RangeItem;
    rangeItem2->text = "0 - 10V";
    // rangeItem2->rightText = CHECKMARK(module->distMode == Rounder::NATURAL);
    rangeItem2->rangeMode = Rounder::CONSTRAINED;
    rangeItem2->min = 0.0f;
    rangeItem2->max = 10.0f;
    rangeItem2->module = module;
    menu->addChild(rangeItem2);
    
    RangeItem *rangeItem3 = new RangeItem;
    rangeItem3->text = "0 - 1V";
    rangeItem3->rangeMode = Rounder::CONSTRAINED;
    rangeItem3->min = 0.f;
    rangeItem3->max = 1.f;
    rangeItem3->module = module;
    menu->addChild(rangeItem3);
    
    RangeItem *rangeItem4 = new RangeItem;
    rangeItem4->text = "0 - 2V";
    rangeItem4->rangeMode = Rounder::CONSTRAINED;
    rangeItem4->min = 0.f;
    rangeItem4->max = 2.f;
    rangeItem4->module = module;
    menu->addChild(rangeItem4);
    
    RangeItem *rangeItem5 = new RangeItem;
    rangeItem5->text = "-5 - 5V";
    rangeItem5->rangeMode = Rounder::CONSTRAINED;
    rangeItem5->min = -5.f;
    rangeItem5->max = 5.f;
    rangeItem5->module = module;
    menu->addChild(rangeItem5);
  }
};

Model *modelRounder = createModel<Rounder, RounderWidget>("Rounder");
