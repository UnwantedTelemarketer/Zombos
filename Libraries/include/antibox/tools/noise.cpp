#include "noise.h"

#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#include <numeric>

PerlinNoise::PerlinNoise(unsigned int seed) {
    permutation.resize(512);
    std::iota(permutation.begin(), permutation.end(), 0);
    std::default_random_engine engine(seed);
    std::shuffle(permutation.begin(), permutation.end(), engine);
    permutation.insert(permutation.end(), permutation.begin(), permutation.end());
}

double PerlinNoise::noise(double x, double y, double z, double frequency) const {
    x *= frequency;
    y *= frequency;
    z *= frequency;

    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    int A = permutation[X] + Y;
    int AA = permutation[A] + Z;
    int AB = permutation[A + 1] + Z;
    int B = permutation[X + 1] + Y;
    int BA = permutation[B] + Z;
    int BB = permutation[B + 1] + Z;

    return lerp(w, lerp(v, lerp(u, grad(permutation[AA], x, y, z),
        grad(permutation[BA], x - 1, y, z)),
        lerp(u, grad(permutation[AB], x, y - 1, z),
            grad(permutation[BB], x - 1, y - 1, z))),
        lerp(v, lerp(u, grad(permutation[AA + 1], x, y, z - 1),
            grad(permutation[BA + 1], x - 1, y, z - 1)),
            lerp(u, grad(permutation[AB + 1], x, y - 1, z - 1),
                grad(permutation[BB + 1], x - 1, y - 1, z - 1))));
}

double PerlinNoise::noise(double x, double y, double z, double frequency, int octaves, double amplitude) const {
    double totalNoise = 0.0;
    double maxAmplitude = 0.0;

    for (int i = 0; i < octaves; ++i) {
        double currentFrequency = frequency * pow(2, i);
        double currentAmplitude = amplitude * pow(0.5, i);

        totalNoise += currentAmplitude * noise(x * currentFrequency, y * currentFrequency, z * currentFrequency, frequency);
        maxAmplitude += currentAmplitude;
    }

    // Normalize the result to ensure it's within the range [-1, 1]
    return totalNoise / maxAmplitude;
}

double PerlinNoise::fade(double t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double t, double a, double b) const {
    return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y, double z) const {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}