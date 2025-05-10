#include <iostream>
#include <random>
#include <string>
#include <ctime>

class Generation {

    // Generates a random lowercase letter
    static char getRandomLetter(std::mt19937& rng) {
        std::uniform_int_distribution<> dist('a', 'z');
        return static_cast<char>(dist(rng));
    }

    // Capitalizes the first letter of the name
    static std::string capitalize(const std::string& name) {
        if (name.empty()) return name;
        std::string result = name;
        result[0] = static_cast<char>(std::toupper(result[0]));
        return result;
    }

    // Generates a random first name of given length
    static std::string generateFirstName(int length) {
        static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
        std::string name;
        for (int i = 0; i < length; ++i) {
            name += getRandomLetter(rng);
        }
        return capitalize(name);
    }
public:
    static std::string GenerateName() {
        int length = 6;  // You can change this
        std::string firstName = generateFirstName(length);
        return firstName;
    }

};