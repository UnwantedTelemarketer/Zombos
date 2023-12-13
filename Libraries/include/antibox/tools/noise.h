#pragma once
#include <vector>

class PerlinNoise {
public:
    PerlinNoise(unsigned int seed = 0);

    double noise(double x, double y, double z, double frequency) const;
    double noise(double x, double y, double z, double frequency, int octaves, double amplitude) const;

private:
    double fade(double t) const;
    double lerp(double t, double a, double b) const;
    double grad(int hash, double x, double y, double z) const;

    std::vector<int> permutation;
};