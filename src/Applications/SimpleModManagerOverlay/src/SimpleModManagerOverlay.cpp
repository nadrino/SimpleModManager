//
// Created by Adrien Blanchet on 18/04/2023.
//

// Needed before including tesla header in multiple files
#define TESLA_INIT_IMPL

//#include "OverlayGuiLoader.h"
#include "ExampleGui.h"

#include <tesla.hpp>    // The Tesla Header


int main(int argc, char **argv) {
//  return tsl::loop<OverlayGuiLoader>(argc, argv);
  return tsl::loop<OverlayTest>(argc, argv);
}
