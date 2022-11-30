float quantize(std::vector<float> sources, float in) {
  int nearest = 0;
  float minDiff = std::numeric_limits<float>::max();
  for (size_t i = 0; i < sources.size(); i++) {
    float src = sources[i];
    if (fabs(in - src) < minDiff) {
      minDiff = fabs(in - src);
      nearest = i;
    }
  }
  return sources[nearest];
}

float quantizeBalanced(std::vector<float> sources, float in) {
  sort(sources.begin(), sources.end());
  float inc = (sources.back() - sources.front()) / (sources.size() - 1);
  float minDiff = std::numeric_limits<float>::max();
  int nearest = 0;
  for (size_t i = 0; i < sources.size(); i++) {
    float src = sources.front() + i * inc;
    if (fabs(in - src) < minDiff) {
      minDiff = fabs(in - src);
      nearest = i;
    }
  }
  return sources[nearest];
}

float quantizeSwitch(std::vector<float> sources, float inMin, float inMax, float in) {
  sort(sources.begin(), sources.end());
  float x = (((in - inMin) * (1 - 0)) / (inMax - inMin)) + 0;
  int idx = floor(x * sources.size());
  if (idx > sources.size()) {
    return sources.back();
  }
  if (idx < 0) {
    return sources.front();
  }
  return sources[idx];
}
