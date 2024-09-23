#pragma once
#include "antibox/objects/tokenizer.h"
#include <iostream>
#include <fstream>

enum ConsumeEffect { none = 0, heal = 1, quench = 2, saturate = 3, pierceDamage = 4, bluntDamage = 5, coverInLiquid };
enum Liquid { nothing, water, blood, fire };
enum Action { use, consume, combine };
enum Behaviour { Wander, Protective, Stationary, Aggressive };
enum Faction { Human, Zombie, Wildlife };
enum equipType { notEquip = 0, weapon = 1, hat = 2, shirt = 3, pants = 4};

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
	std::string name, section, description, cooks_into;
	std::string id = "none";
	bool stackable, holdsLiquid, consumable, cookable;
	int count = 1;
	std::string consumeTxt, useTxt;

	ActionEffect use = { {none, none}, {none, none} };

	float weight;
	float liquidAmount = 0.f;
	Liquid coveredIn = nothing; //liquids
	Liquid heldLiquid = nothing;
	int ticksUntilDry = 0;
	int initialTickTime = 0;
	equipType eType = notEquip;
	int mod = 0;

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
		//cookable = item.getBool("cookable");
		//cooks_into = item.getString("cooks_into");
		count = item.getInt("amount");
		consumeTxt = item.getString("consumeTxt");
		useTxt = item.getString("useTxt");
		eType = (equipType)item.getInt("equipType");


		std::vector<std::string> effects = item.getArray("effects");

		if(eType != none) mod = stoi(effects[4]);

		use = { {(ConsumeEffect)stoi(effects[0]), (float)stoi(effects[1])},
				{(ConsumeEffect)stoi(effects[2]), (float)stoi(effects[3])} };
	}
};

struct Container {
	Vector2_I localCoords;
	Vector2_I globalCoords;
	std::vector<Item> items;

	std::vector<std::string> getItemNames() {
		std::vector<std::string> names;
		for (size_t i = 0; i < items.size(); i++)
		{
			if (items[i].count > 1) { names.push_back(items[i].name + " x " + std::to_string(items[i].count)); }
			else {
				names.push_back(items[i].name);
			}
		}
		return names;
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

	std::vector<std::string> getItemNames() {
		std::vector<std::string> names;
		for (size_t i = 0; i < inv.size(); i++)
		{
			names.push_back(inv[i].name);
		}
		return names;
	}
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
	Item currentWeapon = {"Fists"};

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
struct Tile;

struct Saved_Tile {
	int id = -1;
	Liquid liquid = nothing;
	int burningFor = 0;
	int ticksPassed = 0;
	int ticksNeeded = 1;
	bool hasItem = false;
	std::string itemName = "NULL";
	int x, y = 0;
	bool walkable = false;

	void Serialize(std::ofstream& stream) {
		stream.write(reinterpret_cast<const char*>(&id), sizeof(id));
		stream.write(reinterpret_cast<const char*>(&liquid), sizeof(liquid));

		stream.write(reinterpret_cast<const char*>(&burningFor), sizeof(burningFor));
		stream.write(reinterpret_cast<const char*>(&ticksPassed), sizeof(ticksPassed));
		stream.write(reinterpret_cast<const char*>(&ticksNeeded), sizeof(ticksNeeded));
		stream.write(reinterpret_cast<const char*>(&hasItem), sizeof(hasItem));
		stream.write(reinterpret_cast<const char*>(&x), sizeof(x));
		stream.write(reinterpret_cast<const char*>(&y), sizeof(y));
		stream.write(reinterpret_cast<const char*>(&walkable), sizeof(walkable));

		size_t size = itemName.size();
		stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
		stream.write(itemName.data(), size);

	}
	void Deserialize(std::ifstream& stream) {
		// Read non-trivial members separately
		stream.read(reinterpret_cast<char*>(&id), sizeof(id));
		stream.read(reinterpret_cast<char*>(&liquid), sizeof(liquid));

		// Read other members
		stream.read(reinterpret_cast<char*>(&burningFor), sizeof(burningFor));
		stream.read(reinterpret_cast<char*>(&ticksPassed), sizeof(ticksPassed));
		stream.read(reinterpret_cast<char*>(&ticksNeeded), sizeof(ticksNeeded));
		stream.read(reinterpret_cast<char*>(&hasItem), sizeof(hasItem));
		stream.read(reinterpret_cast<char*>(&x), sizeof(x));
		stream.read(reinterpret_cast<char*>(&y), sizeof(y));
		stream.read(reinterpret_cast<char*>(&walkable), sizeof(walkable));

		size_t size = 0;
		stream.read(reinterpret_cast<char*>(&size), sizeof(size));

		// Read the string data
		itemName.resize(size);
		stream.read(&itemName[0], size);
	}
};

enum direction{up, down, left, right, still};

struct Tile {
	int id = -1;
	direction technical_dir = up;
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
	bool technical_update = false;
	vec2_i coords;
	bool double_size = false;

	void LoadTile(Saved_Tile tile) {
		id = tile.id;
		liquid = tile.liquid;
		burningFor = tile.burningFor;
		ticksPassed = tile.ticksPassed;
		ticksNeeded = tile.ticksNeeded;
		hasItem = tile.hasItem;
		itemName = tile.itemName;
		coords = { tile.x, tile.y };
		walkable = tile.walkable;
	}

	bool CanUpdate() {
		return ticksPassed >= ticksNeeded && changesOverTime;
	}
};

static void CreateSavedTile(Saved_Tile* sTile, Tile tile) {
	sTile->id = tile.id;
	sTile->liquid = tile.liquid;
	sTile->burningFor = tile.burningFor;
	sTile->ticksPassed = tile.ticksPassed;
	sTile->ticksNeeded = tile.ticksNeeded;
	sTile->hasItem = tile.hasItem;
	sTile->itemName = tile.itemName;
	sTile->x = tile.coords.x;
	sTile->y = tile.coords.y;
	sTile->walkable = tile.walkable;
}

class Inventory;

class CraftingSystem {
public:
	void addRecipe(std::string output, std::map<std::string, int> inputs) {
		recipes.insert({ output, inputs });
	}

	
	std::string AttemptCraft(std::string output, std::vector<Item>* inventory) {
		std::map<std::string, Item*> items;
		std::map<std::string, int> itemsToRemove;
		int componentsUsed = 0;
		for (size_t i = 0; i < inventory->size(); i++)
		{
			items.insert({(*inventory)[i].section, &(*inventory)[i]});
		}
		for (const auto& component : recipes[output]) {
			if (items.count(component.first) != 0) {
				if (items[component.first]->count >= component.second) {
					componentsUsed++;
					itemsToRemove.insert({ component.first, component.second });
				}
			}
		}

		if (componentsUsed >= recipes[output].size()) {
			for (const auto& item : itemsToRemove) {
				items[item.first]->count -= item.second;
			}
			return output;
		}
		return "none";
	}

	std::vector<std::string> getRecipeNames() {
		std::vector<std::string> recipeNames;

		for (const auto& name : recipes) {
			recipeNames.push_back(name.first);
		}

		return recipeNames;
	}

	std::vector<std::string> getRecipeComponents(std::string key) {
		std::vector<std::string> recipeComps;

		for (const auto& name : recipes[key]) {
			recipeComps.push_back(name.first + " x " + std::to_string(name.second));
		}

		return recipeComps;
	}

private:

	std::unordered_map<std::string, std::map<std::string, int>> recipes;
};


#define ENT_PLAYER "E"
#define ID_PLAYER 0

#define ENT_ZOMBIE "F"
#define ID_ZOMBIE 1

#define ENT_CHICKEN "L"
#define ID_CHICKEN 2

#define ENT_HUMAN "E"
#define ID_HUMAN 3


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


#define TILE_CONVEYOR_U "M"
#define ID_CONVEYOR_U 100

#define TILE_CONVEYOR_D "N"
#define ID_CONVEYOR_D 101

#define TILE_CONVEYOR_L "T"
#define ID_CONVEYOR_L 102

#define TILE_CONVEYOR_R "S"
#define ID_CONVEYOR_R 103

#define TILE_CONVEYOR_UL "O"
#define ID_CONVEYOR_UL 104

#define TILE_CONVEYOR_UR "P"
#define ID_CONVEYOR_UR 105

#define TILE_CONVEYOR_DL "Q"
#define ID_CONVEYOR_DL 106

#define TILE_CONVEYOR_DR "R"
#define ID_CONVEYOR_DR 107
#endif
