#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int run_tests(int argc, char** argv) {
  return Catch::Session().run(argc, argv);
}
