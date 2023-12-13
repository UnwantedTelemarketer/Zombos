#pragma once
#include "antibox/objects/tokenizer.h"

enum ConsumeEffect { none = 0, heal = 1, quench = 2, saturate = 3, pierceDamage = 4, bluntDamage = 5, coverInLiquid };
enum Liquid { nothing, water, blood, fire };
enum Action { use, consume, combine };
enum Behaviour { Wander, Protective, Stationary, Aggressive };
enum Faction { Human, Zombie, Wildlife };

//What the effect is and how much it does.
struct EffectAmount {
	ConsumeEffect effect = none;
	float amount = 0.f;
};

//Consume, BodyUse
struct ActionEffect {
	EffectAmount onConsume;
	EffectAmount onBodyUse;
};

//Name, ID, Stackable, Holds Liquid, Consumable, Count, Consume Text, Use Text
struct Item {
	std::string name, section, description;
	std::string id = "none";
	bool stackable, holdsLiquid, consumable;
	int count = 1;
	std::string consumeTxt, useTxt;

	ActionEffect use = { {none, none}, {none, none} };

	float weight;
	float liquidAmount = 0.f;
	Liquid coveredIn = nothing; //liquids
	Liquid heldLiquid = nothing;
	int ticksUntilDry = 0;
	int initialTickTime = 0;

	void CoverIn(Liquid l, int ticks) {
		coveredIn = l;
		ticksUntilDry = ticks;
		initialTickTime = ticks;
	}

	void CreateFromData(OpenedData item) {
		name = item.getString("name");
		section = item.section_name;
		description = item.getString("description");
		id = item.getString("id");
		stackable = item.getBool("stackable");
		holdsLiquid = item.getBool("holdsLiquid");
		consumable = item.getBool("consumable");
		count = item.getInt("amount");
		consumeTxt = item.getString("consumeTxt");
		useTxt = item.getString("useTxt");

		std::vector<std::string> effects = item.getArray("effects");

		use = { {(ConsumeEffect)stoi(effects[0]), (float)stoi(effects[1])},
				{(ConsumeEffect)stoi(effects[2]), (float)stoi(effects[3])} };
	}
};

//Health, Name, ID, Behaviour, Aggressive
struct Entity {
	float health;
	const char* name;
	int entityID;
	Behaviour b;
	bool aggressive;
	Faction faction;
	int viewDistance;
	int damage;
	bool canTalk;

	Vector2_I coords;
	bool lootAlive = false;
	int index; //in entity list
	Liquid coveredIn = nothing;
	int ticksUntilDry = 0;

	bool targetingPlayer;
	bool talking;
	std::string message = "empty";
	Entity* target = nullptr;
	std::vector<Item> inv;

	bool targeting() { return target != nullptr || targetingPlayer; }
};

struct Player {
	float health = 100;
	float thirst = 100;
	float hunger = 100;
	int damage = 5;
	int ticksCovered = 0;
	int liquidLast = 50;
	bool aiming = false;
	std::string name = "Blank";
	Vector2_I coords;
	Vector2_I crosshair;
	Liquid coveredIn = nothing;

	void TakeDamage(ConsumeEffect type, int dmg) {
		health -= dmg;
		if (type == pierceDamage) {
			CoverIn(blood, 20);
		}
	}

	//Covers them in but does some minor checks to clean you off and stuff
	void CoverIn(Liquid liquid, int turns) {
		liquidLast = turns;
		if (coveredIn != nothing && coveredIn != water && liquid == water) { coveredIn = nothing; return; }
		coveredIn = liquid;
	}
};

struct Tile {
	int id = -1;
	Liquid liquid = nothing;
	Entity* entity = nullptr;
	bool collectible = false;
	int collectedReplacement = id;
	std::string itemName = "NULL";
	int burningFor = 0;
	bool walkable = true;

	bool changesOverTime = false;
	int timedReplacement = id;
	int ticksPassed = 0;
	int ticksNeeded = 1;
	bool hasItem = false;
	float brightness = 1.f;
	bool visited = false;
	vec2_i coords;


	bool CanUpdate() {
		return ticksPassed >= ticksNeeded && changesOverTime;
	}
};





struct Recipe {
	std::vector<std::string> inputs;
	Item output;
};

std::string hash(const std::vector<std::string>& items) {
	std::vector<std::string> sortedIds;
	for (const auto& item : items) {
		sortedIds.push_back(item);
	}
	std::sort(sortedIds.begin(), sortedIds.end());
	std::string ss;
	for (const auto& id : sortedIds) {
		ss += id + ",";
	}
	return ss;
}

//I dont really understand this very well im gonna be honest, thanks chatgpt
//Best i can tell is it generates a hash based on item inputs and checks the recipe hash to see if it matches
class CraftingSystem {
public:
	void addRecipe(const Recipe& recipe) {

		// Compute the hash code for the input items
		std::string key = hash(recipe.inputs);
		// Add the recipe to the hash table
		recipes_[key].push_back(recipe);
	}

	Item craft(const std::vector<std::string>& input, std::string expectedOutput) {
		// Compute the hash code for the input items
		std::string key = hash(input);
		// Look up the matching recipes in the hash table
		auto it = recipes_.find(key);
		if (it != recipes_.end()) {
			// Craft the first matching recipe found
			for (const auto& recipe : it->second) {

				Console::Log(recipe.output.id, ERROR, __LINE__);
				if (canCraft(recipe.inputs, input) ) {
					return recipe.output;
				}
			}
		}
		// No matching recipe found
		return Item();
	}

private:
	bool canCraft(const std::vector<std::string>& required, const std::vector<std::string>& available) {
		// Check if the available items contain all the required items
		for (const auto& item : required) {
			bool found = false;
			for (const auto& other : available) {
				if (item == other) {
					found = true;
					break;
				}
			}
			if (!found) {
				return false;
			}
		}
		return true;
	}

	std::unordered_map<std::string, std::vector<Recipe>> recipes_;
};



#define ENT_PLAYER "E"
#define ID_PLAYER 0

#define ENT_ZOMBIE "F"
#define ID_ZOMBIE 1

#define ENT_CHICKEN "L"
#define ID_CHICKEN 2

#define ENT_HUMAN "E"
#define ID_HUMAN 3

#define ICON_FONT

#ifdef REGULAR_FONT
#define TILE_LIQUID "~"

#define TILE_NULL "?"
#define ID_NULL -1

#define TILE_GRASS ";"
#define ID_GRASS 1

#define TILE_DIRT "."
#define ID_DIRT 2

#define TILE_TREE "^"
#define ID_TREE 999

#define TILE_FLOWER "#"
#define ID_FLOWER 3

#define TILE_FIRE "&"
#define ID_FIRE 4

#define TILE_SCRAP "%%"
#define ID_SCRAP 5

#define TILE_STONE "8"
#define ID_STONE 6

#define TILE_STICK "/"
#define ID_STICK 10
#endif

#ifdef ICON_FONT
#define TILE_LIQUID "A"

#define TILE_NULL "?"
#define ID_NULL -1

#define TILE_GRASS "B"
#define ID_GRASS 1

#define TILE_DIRT "C"
#define ID_DIRT 2

#define TILE_TREE "Z"
#define ID_TREE 999

#define TILE_FLOWER "H"
#define ID_FLOWER 3

#define TILE_FIRE "G"
#define ID_FIRE 4

#define TILE_SCRAP "D"
#define ID_SCRAP 5

#define TILE_STONE "J"
#define ID_STONE 6

#define TILE_SAND "C"
#define ID_SAND 7

#define TILE_CONTAINER "J"
#define ID_CONTAINER 8

#define TILE_CONTAINER_OPEN "J"
#define ID_CONTAINER_OPEN 9

#define TILE_STICK "I"
#define ID_STICK 10
#endif
