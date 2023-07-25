#include <limits>
#include "plugin.hpp"
#include "./components.hpp"

struct Comparator : Module {
  enum ParamId {
    A_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    A_INPUT,
    B_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    GREATER_OUTPUT,
    EQUAL_OUTPUT,
    LESS_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  float tolerance = std::numeric_limits<float>::min();

  Comparator() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(A_PARAM, -10.0f, 10.0f, 0.0f, "A", "V");
    configInput(A_INPUT, "A override");
    configInput(B_INPUT, "B");
    configOutput(LESS_OUTPUT, "A less than B");
    configOutput(EQUAL_OUTPUT, "A equal to B");
    configOutput(GREATER_OUTPUT, "A greater than B");
  }

  json_t *dataToJson() override {
    json_t *root = json_object();
    json_object_set_new(root, "tolerance", json_real(tolerance));
    return root;
  }

  void dataFromJson(json_t *root) override {
    json_t *toleranceJ = json_object_get(root, "tolerance");
    if (toleranceJ) {
      tolerance = json_number_value(toleranceJ);
    }
  }

  void process(const ProcessArgs &args) override {
    int channels = inputs[A_INPUT].getChannels();

    if (inputs[B_INPUT].getChannels() > channels) {
      channels = inputs[B_INPUT].getChannels();
    }

    outputs[LESS_OUTPUT].setChannels(channels);
    outputs[EQUAL_OUTPUT].setChannels(channels);
    outputs[GREATER_OUTPUT].setChannels(channels);

    float a = params[A_PARAM].getValue();
    bool aConnected = inputs[A_INPUT].isConnected();
    bool aMono = inputs[A_INPUT].isMonophonic();
    bool bMono = inputs[B_INPUT].isMonophonic();

    for (int c = 0; c < channels; c++) {
      outputs[LESS_OUTPUT].setVoltage(0.0f, c);
      outputs[EQUAL_OUTPUT].setVoltage(0.0f, c);
      outputs[GREATER_OUTPUT].setVoltage(0.0f, c);

      if (aConnected) {
        a = inputs[A_INPUT].getVoltage(aMono ? 0 : c);
      }

      float b = inputs[B_INPUT].getVoltage(bMono ? 0 : c);

      if (a < b - tolerance) {
        outputs[LESS_OUTPUT].setVoltage(10.0f, c);
      } else if (a > b + tolerance) {
        outputs[GREATER_OUTPUT].setVoltage(10.0f, c);
      } else {
        outputs[EQUAL_OUTPUT].setVoltage(10.0f, c);
      }
    }
  }
};

struct ToleranceQuantity : Quantity {
  float *srcTolerance = NULL;

  ToleranceQuantity(float *_srcTolerance) {
    srcTolerance = _srcTolerance;
  }

  void setValue(float value) override {
    *srcTolerance = math::clamp(value, getMinValue(), getMaxValue());
  }

  float getValue() override {
    return *srcTolerance;
  }

  float getMinValue() override {
    return std::numeric_limits<float>::min();
  }

  float getMaxValue() override {
    return 1.0f;
  }

  float getDefaultValue() override {
    return getMinValue();
  }

  std::string getDisplayValueString() override {
    return string::f("±%.3f", getDisplayValue());
  }

  std::string getUnit() override {
    return "V";
  }
};

struct ToleranceSlider : ui::Slider {
  ToleranceSlider(float *_srcTolerance) {
    quantity = new ToleranceQuantity(_srcTolerance);
  }

  ~ToleranceSlider() {
    delete quantity;
  }
};

struct ComparatorWidget : ModuleWidget {
  ComparatorWidget(Comparator *module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Comparator.svg")));

    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<LilacScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<LilacScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<LilacKnob>(mm2px(Vec(7.62, 23.284)), module, Comparator::A_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 36.463)), module, Comparator::A_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(7.62, 55.048)), module, Comparator::B_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 75.534)), module, Comparator::LESS_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 93.947)), module, Comparator::EQUAL_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(7.62, 112.359)), module, Comparator::GREATER_OUTPUT));
  }

  void appendContextMenu(Menu *menu) override {
    Comparator *module = dynamic_cast<Comparator *>(this->module);

    menu->addChild(new MenuSeparator());

    MenuLabel *toleranceLabel = new MenuLabel();
    toleranceLabel->text = "A = B tolerance";
    menu->addChild(toleranceLabel);

    ToleranceSlider *toleranceSlider = new ToleranceSlider(&module->tolerance);
    toleranceSlider->box.size.x = 180.0f;
    menu->addChild(toleranceSlider);
  }
};

Model *modelComparator = createModel<Comparator, ComparatorWidget>("Comparator");
