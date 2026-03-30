#pragma once

#include <array>

class Screenshot {
public:
    int width, height;

    Screenshot();

    std::array<unsigned char, 3> get_rgb(int x, int y) const;

private:
    std::vector<unsigned char> pixels;
};
