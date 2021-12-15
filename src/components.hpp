struct V1Port : rack::app::SvgPort {
  V1Port() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/V1Port.svg")));
  }
};
