#include "plugin.hpp"
#include "quantize.hpp"
#include "./components.hpp"

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
  enum ModeId {
    QUANTIZE,
    QUANTIZE_P,
    UNI_10,
    UNI_1,
    UNI_2,
    BI_5,
  };

  struct Mode {
    ModeId id;
    bool scan;
    float min;
    float max;
  };

  Mode modes[6] = {
      {QUANTIZE, false, 0.f, 0.f},
      {QUANTIZE_P, false, 0.f, 0.f},
      {UNI_10, true, 0.f, 10.f},
      {UNI_1, true, 0.f, 1.f},
      {UNI_2, true, 0.f, 2.f},
      {BI_5, true, -5.f, 5.f},
  };

  Mode mode = modes[0];

  dsp::ClockDivider logDiv;
  InputId srcIns[4];
  int srcCount;
  std::vector<float> sources;

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
    logDiv.setDivision(8192);
  }

  json_t *dataToJson() override {
    json_t *root = json_object();
    json_object_set_new(root, "modeId", json_integer(mode.id));
    return root;
  }

  void dataFromJson(json_t *root) override {
    json_t *idJson = json_object_get(root, "modeId");
    if (idJson) {
      ModeId modeId = (ModeId)json_number_value(idJson);
      mode = modes[modeId];
    }
  }

  void process(const ProcessArgs &args) override {
    srcCount = 0;
    sources.clear();

    for (size_t i = 0; i < 4; i++) {
      int channels = inputs[srcIns[i]].getChannels();
      if (channels > 0) {
        for (size_t c = 0; c < channels; c++) {
          sources.push_back(inputs[srcIns[i]].getVoltage(c));
          inputs[srcIns[i]].getVoltage(c);
          srcCount++;
        }
      }
    }

    int channels = inputs[MAIN_INPUT].getChannels();
    outputs[MAIN_OUTPUT].setChannels(channels);

    for (size_t c = 0; c < channels; c++) {
      if (srcCount > 0) {
        outputs[MAIN_OUTPUT].setVoltage(mapInputToSource(c), c);
      } else {
        outputs[MAIN_OUTPUT].setVoltage(0.f, c);
      }
    }

    // if (logDiv.process()) {
    // 	DEBUG("…");
    // }
  }

  float mapInputToSource(int chan) {
    float in = inputs[MAIN_INPUT].getVoltage(chan);

    if (mode.id == QUANTIZE) {
      return quantize(sources, in);
    }
    if (mode.id == QUANTIZE_P) {
      return quantizeProportional(sources, in);
    }
    if (mode.scan == true) {
      return scan(sources, mode.min, mode.max, in);
    }
    return 0.f;
  }
};

struct RounderWidget : ModuleWidget {
  struct ModeItem : MenuItem {
    Rounder *module;
    Rounder::ModeId modeId;

    void onAction(const event::Action &e) override {
      module->mode = module->modes[modeId];
    }
  };

  RounderWidget(Rounder *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Rounder.svg")));

    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

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

    MenuLabel *modeLabel = new MenuLabel();
    modeLabel->text = "Mode";
    menu->addChild(modeLabel);

    ModeItem *quantizeModeItem = new ModeItem;
    quantizeModeItem->text = "Quantize";
    quantizeModeItem->modeId = Rounder::QUANTIZE;
    quantizeModeItem->rightText = CHECKMARK(module->mode.id == Rounder::QUANTIZE);
    quantizeModeItem->module = module;
    menu->addChild(quantizeModeItem);

    ModeItem *quantizeEqModeItem = new ModeItem;
    quantizeEqModeItem->text = "Quantize (Proportional)";
    quantizeEqModeItem->modeId = Rounder::QUANTIZE_P;
    quantizeEqModeItem->rightText = CHECKMARK(module->mode.id == Rounder::QUANTIZE_P);
    quantizeEqModeItem->module = module;
    menu->addChild(quantizeEqModeItem);

    ModeItem *scanUni10 = new ModeItem;
    scanUni10->text = "Scan 0 – 10V";
    scanUni10->modeId = Rounder::UNI_10;
    scanUni10->rightText = CHECKMARK(module->mode.id == Rounder::UNI_10);
    scanUni10->module = module;
    menu->addChild(scanUni10);

    ModeItem *scanUni1 = new ModeItem;
    scanUni1->text = "Scan 0 – 1V";
    scanUni1->modeId = Rounder::UNI_1;
    scanUni1->rightText = CHECKMARK(module->mode.id == Rounder::UNI_1);
    scanUni1->module = module;
    menu->addChild(scanUni1);

    ModeItem *scanUni2 = new ModeItem;
    scanUni2->text = "Scan 0 – 2V";
    scanUni2->modeId = Rounder::UNI_2;
    scanUni2->rightText = CHECKMARK(module->mode.id == Rounder::UNI_2);
    scanUni2->module = module;
    menu->addChild(scanUni2);

    ModeItem *scanBi5 = new ModeItem;
    scanBi5->text = "Scan -5 – 5V";
    scanBi5->modeId = Rounder::BI_5;
    scanBi5->rightText = CHECKMARK(module->mode.id == Rounder::BI_5);
    scanBi5->module = module;
    menu->addChild(scanBi5);
  }
};

Model *modelRounder = createModel<Rounder, RounderWidget>("Rounder");
