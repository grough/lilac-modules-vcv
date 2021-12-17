struct V1Port : rack::app::SvgPort {
  V1Port() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/V1Port.svg")));
  }
};

struct WarmKnob : RoundKnob {
  WarmKnob() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WarmKnob.svg")));
  }
};
