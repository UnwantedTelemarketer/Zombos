#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <random>
#include <ctime>

class NameGenerator {
    // Whimsical / post-apocalyptic style syllables
    inline static const std::vector<std::string> syllablesStart = {
        "Ar", "Br", "Cal", "Da", "El", "Fi", "Ga", "Ha",
        "I", "Jo", "Ka", "Ly", "Mi", "Ni", "Or", "Pa",
        "Qu", "Ru", "So", "To", "Ve", "Wr", "Ya", "Za"
    };

    inline static const std::vector<std::string> syllablesMiddle = {
        "a", "e", "i", "o", "u", "an", "el", "in", "or", "ar",
        "en", "ir", "on", "ur", "yn", "al", "is", "os", "um",
        "ea", "ai", "ei", "io", "ou", "au", "ar", "er", "ir", "or", "ur"
    };

    inline static const std::vector<std::string> syllablesEnd = {
        "a", "e", "i", "o", "u", "n", "r", "s", "l", "k", "d",
        "on", "ar", "en", "is", "or", "um", "el", "ir", "os", "an",
        "in", "al", "as", "ek", "ik", "or", "us", "yr", "on"
    };

    static std::mt19937& getRNG() {
        static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
        return rng;
    }

    static std::string getRandomElement(const std::vector<std::string>& vec) {
        std::uniform_int_distribution<> dist(0, static_cast<int>(vec.size() - 1));
        return vec[dist(getRNG())];
    }

public:
    // Generate a first name using 2-3 syllables
    static std::string generateFirstName() {
        std::string name = getRandomElement(syllablesStart);
        int middleCount = 1 + std::uniform_int_distribution<>(0, 1)(getRNG()); // 1-2 middle syllables

        for (int i = 0; i < middleCount; ++i) {
            name += getRandomElement(syllablesMiddle);
        }

        name += getRandomElement(syllablesEnd);

        // Capitalize first letter
        if (!name.empty()) name[0] = static_cast<char>(std::toupper(name[0]));

        return name;
    }
};
