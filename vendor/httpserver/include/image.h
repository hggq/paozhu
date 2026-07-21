#pragma once
#include <cstdint>
#include <vector>
#include <string>


namespace image {

struct ColorRGB {
    uint8_t r = 0, g = 0, b = 0;
};

struct ColorRGBA {
  uint8_t r = 0, g = 0, b = 0, a = 255;
};

}