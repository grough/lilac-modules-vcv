#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "quantize.hpp"

TEST_CASE("Quantize", "[]") {
  std::vector<float> sources = {-5.f, 4.f, 5.f};
  REQUIRE(quantize(sources, -1.f) == Approx(-5.f));
  REQUIRE(quantize(sources, 0.f) == Approx(4.f));
  REQUIRE(quantize(sources, 4.51f) == Approx(5.f));
}

TEST_CASE("Quantize (Proportional)", "[]") {
  std::vector<float> sources = {0.f, 10.f, 8.f};
  REQUIRE(quantizeProportional(sources, 0.f) == Approx(0.f));
  REQUIRE(quantizeProportional(sources, 2.49f) == Approx(0.f));
  REQUIRE(quantizeProportional(sources, 2.51f) == Approx(8.0f));
  REQUIRE(quantizeProportional(sources, 7.49f) == Approx(8.0f));
  REQUIRE(quantizeProportional(sources, 7.51f) == Approx(10.0f));
  REQUIRE(quantizeProportional(sources, 10.1f) == Approx(10.0f));
}

TEST_CASE("Scan", "[]") {
  std::vector<float> sources = {1.23f, 2.34f, 3.45f, 4.56f};
  REQUIRE(scan(sources, 0.f, 1.f, 0.f) == Approx(1.23f));
  REQUIRE(scan(sources, 0.f, 1.f, 0.24f) == Approx(1.23f));
  REQUIRE(scan(sources, 0.f, 1.f, 0.26f) == Approx(2.34f));
  REQUIRE(scan(sources, 0.f, 1.f, 0.49f) == Approx(2.34f));
  REQUIRE(scan(sources, 0.f, 1.f, 0.51f) == Approx(3.45f));
  REQUIRE(scan(sources, 0.f, 1.f, 0.74f) == Approx(3.45f));
  REQUIRE(scan(sources, 0.f, 1.f, 0.76f) == Approx(4.56f));
  REQUIRE(scan(sources, 0.f, 1.f, 0.99f) == Approx(4.56f));

  REQUIRE(scan(sources, 0.f, 10.f, -1.f) == Approx(1.23f)); // failing
  REQUIRE(scan(sources, 0.f, 10.f, 0.f) == Approx(1.23f));
  REQUIRE(scan(sources, 0.f, 10.f, 02.4f) == Approx(1.23f));
  REQUIRE(scan(sources, 0.f, 10.f, 02.6f) == Approx(2.34f));
  REQUIRE(scan(sources, 0.f, 10.f, 04.9f) == Approx(2.34f));
  REQUIRE(scan(sources, 0.f, 10.f, 05.1f) == Approx(3.45f));
  REQUIRE(scan(sources, 0.f, 10.f, 07.4f) == Approx(3.45f));
  REQUIRE(scan(sources, 0.f, 10.f, 07.6f) == Approx(4.56f));
  REQUIRE(scan(sources, 0.f, 10.f, 09.9f) == Approx(4.56f));
  REQUIRE(scan(sources, 0.f, 1.f, 100.f) == Approx(4.56f));
}
