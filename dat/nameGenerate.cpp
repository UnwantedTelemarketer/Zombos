#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <random>
#include <ctime>


class NameGenerator {
private:
    inline static int RandomInt(int min, int max) { return min + rand() % (max - min + 1); }
public:
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

    // Limbs
    inline static const std::vector<std::string> limbs = {
        "Leg", "Eye", "Arm", "Brain", "Skull", "Hand", "Chest", "Heart", "Teeth"
    };

    // Materials
    inline static const std::vector<std::string> materials = {
        "Metal", "Unnatural", "Rotten", "Rock", "Missing", "Artificial"
    };

    // Articles / Determiners
    inline static const std::vector<std::string> articles = {
        "The", "Our"
    };

    // Ordinal / Number / Temporal descriptors
    inline static const std::vector<std::string> ordinals = {
        "First", "Second", "Third", "Fourth", "Fifth", "Last", "Forgotten", "Eternal", "True", "False"
    };

    // Adjectives / Descriptive words
    inline static const std::vector<std::string> adjectives = {
        "Grand", "Grim", "Iron", "Silent", "Broken", "Wandering", "Ancient", "Dead", "Unborn", "Shattered", "Strange", "Crimson", "Wild", "Bright", "Hidden", "Hopeful", "Forgotten", "Eternal"
    };

    // Nouns / Faction types / Entities
    inline static const std::vector<std::string> factionNouns = {
        "Raiders", "Marauders", "People", "Children", "Nomads", "Brotherhood", "Cult", "Clan", "Assemblers", "Seekers", "Reclaimers", "Creators", "Wanderers"
    };

    // Nouns / Objects / Misc endings
    inline static const std::vector<std::string> objectNouns = {
        "Day", "Corpse", "One", "Year", "Behemoth", "Staff", "Killer", "Martyr", "Shattered"
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

    static std::string generateUniqueName() {
        std::string name = generateFirstName();
        int nameMode = RandomInt(1, 3);

        switch (nameMode) {

        //titled name
        case 1:
            name += ", ";
            if (RandomInt(0, 3) == 3) {
                name += " the ";
            }
            name += getRandomElement(adjectives) + " " + getRandomElement(objectNouns);
            break;

        //numbered name
        case 2:
            name += " the " + getRandomElement(ordinals);
            break;

        //
        case 3:
            if (RandomInt(1, 2) == 2) {
                name += ", " + getRandomElement(materials);
            }
            else {
                name += ", " + getRandomElement(adjectives);
            }
            name += " " + getRandomElement(limbs);
            break;
        }
        return name;
    }

    static std::string generateFactionName() {
        int nameType = RandomInt(1, 5);
        std::string name;

        switch (nameType) {
        case 1: // "The [factionNoun] of the [adjective]"
            name = getRandomElement(articles) + " " + getRandomElement(factionNouns) + " of the ";
            if (RandomInt(0, 2) == 1) { // optional secondary adjective
                name += getRandomElement(adjectives) + " ";
            }
            name += getRandomElement(adjectives);
            break;

        case 2: // "[Ordinal] [factionNoun]"
            if (RandomInt(0, 2) == 1) { // optional "The" or "Our"
                name += getRandomElement(articles) + " ";
            }
            name += getRandomElement(ordinals) + " " + getRandomElement(factionNouns);
            break;

        case 3: // "[Adjective] [factionNoun]"
            name = getRandomElement(adjectives) + " " + getRandomElement(factionNouns);
            break;

        case 4: // "[Article] [adjective] [objectNoun]"
            name = getRandomElement(articles) + " " + getRandomElement(adjectives) + " " + getRandomElement(objectNouns);
            break;
        case 5:
            name = "";
            for (int i = 0; i < RandomInt(1, 3); ++i) {
                if (i > 0) name += " ";
                name += generateFirstName();
            }

            name += " " + getRandomElement(factionNouns);
            break;
        case 6:
            name = getRandomElement(adjectives) + " " + getRandomElement(factionNouns) + " of " + getRandomElement(objectNouns);
            break;
        case 7:
            name = getRandomElement(ordinals) + " " + getRandomElement(adjectives) + " " + getRandomElement(factionNouns);
            break;
        case 8:
            name = getRandomElement(articles) + " " + getRandomElement(factionNouns) + " of " + getRandomElement(adjectives) + " " + getRandomElement(objectNouns);
            break;
        }

        return name;
    }
};
