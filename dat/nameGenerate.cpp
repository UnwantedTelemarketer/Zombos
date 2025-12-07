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
    inline static const std::vector<std::string> humanStart = {
        "Ar", "Ba", "Cal", "Dan", "El", "Fenn", "Garen", "Hal",
        "Is", "Jon", "Kell", "Lora", "Mar", "Ner", "Olin", "Pax",
        "Rin", "Sam", "Tor", "Var", "Wes", "Yen", "Zar",
        "Ada", "Bren", "Cor", "Dela", "Evan", "Har", "Jas"
    };

    inline static const std::vector<std::string> humanMiddle = {
        "a", "e", "i", "o", "u",
        "an", "el", "in", "or", "ar",
        "en", "ir", "on", "un", "al",
        "ia", "ea", "ai", "io", "ou",
        "rin", "mar", "ten", "len", "vor", "lan"
    };

    inline static const std::vector<std::string> humanEnd = {
        "a", "e", "i", "o", "u",
        "n", "r", "s", "l", "m", "t",
        "son", "el", "en", "or", "an",
        "in", "al", "as", "et", "is",
        "lin", "mar", "den", "ric", "tam",
        "vor", "len"
    };



    // Whimsical / post-apocalyptic style syllables
    inline static const std::vector<std::string> syllablesStart = {
        "Ark", "Brak", "Cinder", "Dra", "Eks", "Fyr", "Ghul", "Hax",
        "Ion", "Junk", "Krag", "Luma", "Murk", "Nex", "Ox", "Pyra",
        "Quell", "Rav", "Scrap", "Tarn", "Vox", "Wyr", "Yara", "Zer",
        "Ash", "Brot", "Cyr", "Drim", "Hex", "Flux"
    };

    inline static const std::vector<std::string> syllablesMiddle = {
        "a", "e", "i", "o", "u",
        "ash", "yr", "ox", "ek", "uk", "irr", "unn",
        "rax", "vek", "zir", "vyn", "ark", "usk",
        "io", "au", "ea", "ou",
        "scr", "tek", "mut", "slag", "wyr", "os"
    };


    inline static const std::vector<std::string> syllablesEnd = {
        "ak", "ek", "ik", "ok", "uk",
        "n", "r", "s", "st", "lk", "rk", "sk",
        "wyn", "ash", "orn", "usk", "yra", "ex",
        "os", "yr", "en", "or",
        "mite", "shard", "scrap", "vex", "dusk"
    };


    // Limbs
    inline static const std::vector<std::string> limbs = {
        "Leg", "Eye", "Arm", "Brain", "Skull", "Hand", "Chest", "Heart", "Teeth", "Lung", "Lungs", "Eyes"
    };

    // Materials
    inline static const std::vector<std::string> materials = {
        "Metal", "Unnatural", "Rotten", "Rock", "Missing", "Artificial", "Wire", "Slag", "Iron", "Rust"
    };

    // Articles / Determiners
    inline static const std::vector<std::string> articles = {
        "The"
    };

    // Ordinal / Number / Temporal descriptors
    inline static const std::vector<std::string> ordinals = {
        "First", "Second", "Third", "Fourth", "Fifth", "Last", "Forgotten", "Eternal", "True", "False"
    };

    // Adjectives / Descriptive words
    inline static const std::vector<std::string> adjectives = {
        "Grand", "Grim", "Iron", "Silent", "Broken",
        "Wandering", "Ancient", "Dead", "Unborn",
        "Shattered", "Strange", "Crimson", "Hidden",
        "Forgotten", "Eternal", "Hateful", "Violent",
        "Weakened", "Lost", "Fearful", "Horrid"
    };

    // Nouns / Faction types / Entities
    inline static const std::vector<std::string> factionNouns = {
        "Raiders", "Marauders", "People", "Children",
        "Nomads", "Brotherhood", "Cult", "Clan", 
        "Assemblers", "Seekers", "Reclaimers", 
        "Creators", "Wanderers", "Bandits",
        "Brothers", "Sisters"
    };

    // Nouns / Objects / Misc endings
    inline static const std::vector<std::string> objectNouns = {
        "Dawn", "Corpse", "One", "Year", "Behemoth",
        "Machine", "Killer", "Martyr", "Shattered",
        "Exiled", "Scrapper", "Explorer", "Madman",
        "King", "God-King"
    };

    // Pluralized Endings
    inline static const std::vector<std::string> pluralNouns = {
        "Dawn", "Corpses", "Behemoths",
        "Machines", "Killers", "Martyrs",
        "Scrappers", "Explorers", "Madmen",
        "Scavengers", "Loners"
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

        if (RandomInt(0, 3) == 3) {
            name += "-";
        }
        for (int i = 0; i < middleCount; ++i) {
            name += getRandomElement(syllablesMiddle);
        }
        if (RandomInt(0, 3) == 3) {
            name += "-";
        }
        name += getRandomElement(syllablesEnd);

        // Capitalize first letter
        if (!name.empty()) name[0] = static_cast<char>(std::toupper(name[0]));

        return name;
    }

    static std::string generateHumanName() {
        std::string name = getRandomElement(humanStart);
        int middleCount = std::uniform_int_distribution<>(0, 1)(getRNG()); // 1-2 middle syllables

        for (int i = 0; i < middleCount; ++i) {
            name += getRandomElement(humanMiddle);
        }
        
        name += getRandomElement(humanEnd);

        // Capitalize first letter
        if (!name.empty()) name[0] = static_cast<char>(std::toupper(name[0]));

        return name;
    }

    static std::string generateLeaderName() {
        std::string name = generateHumanName();
        int nameMode = RandomInt(1, 3);

        switch (nameMode) {

        //titled name
        case 1:
            name += ", ";
            if (RandomInt(0, 2) == 1) {
                name += "the ";
            }
            name += getRandomElement(adjectives) + " " + getRandomElement(objectNouns);
            break;

        //numbered name
        case 2:
            name += " the " + getRandomElement(ordinals) + " " + getRandomElement(objectNouns);
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

    static std::string generateUniqueName() {
        std::string name = generateHumanName();
        name += " the ";

        if (RandomInt(1, 2) == 2) {
            name += getRandomElement(adjectives);
        }
        else {
            name += getRandomElement(ordinals);
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
            name = getRandomElement(articles) + " " + getRandomElement(adjectives) + " " + getRandomElement(pluralNouns);
            break;
        case 5: // "[Name] Cult OR The Cult of [Name]
            if (RandomInt(1, 2) == 1) {
                name = generateFirstName() + " " + getRandomElement(factionNouns);
            }
            else {
                name = "The " + getRandomElement(factionNouns) + " of " + generateFirstName();
            }
            break;
        case 6: // [Adjective] [Noun] of [Noun]
            name = getRandomElement(adjectives) + " " + getRandomElement(factionNouns) + " of " + getRandomElement(pluralNouns);
            break;
        case 7: // [Number] [Adjective] [Noun]
            name = getRandomElement(ordinals) + " " + getRandomElement(adjectives) + " " + getRandomElement(factionNouns);
            break;
        case 8: // The [Noun] of [Adjective] [Noun]
            name = getRandomElement(articles) + " " + getRandomElement(factionNouns) + " of " + getRandomElement(adjectives) + " " + getRandomElement(pluralNouns);
            break;
        }

        return name;
    }
};
