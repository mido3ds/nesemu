#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int testMain(int argc, char** argv) {
  return Catch::Session().run(argc, argv);
}
