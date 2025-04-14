#pragma once
#include "antibox/objects/tokenizer.h"
#include <iostream>
#include <fstream>
#include <set>

enum ConsumeEffect { none = 0, heal = 1, quench = 2, saturate = 3, pierceDamage = 4, bluntDamage = 5, coverInLiquid = 6};
enum biome { desert, ocean, forest, swamp, taiga, grassland, urban, jungle };
enum Liquid { nothing = 0, water = 1, blood = 2, fire = 3, guts = 4, mud = 5 , snow = 6};
enum Action { use, consume, combine };
enum Behaviour { Wander, Protective, Stationary, Aggressive };
		//	  wanderer | tribal
enum Faction { Human_W, Human_T, Dweller, Zombie, Wildlife, Takers };
enum equipType { notEquip = 0, weapon = 1, hat = 2, shirt = 3, pants = 4, boots = 5, gloves = 6};


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
	bool stackable, holdsLiquid, consumable, cookable;
	int count = 1;
	std::string consumeTxt, useTxt;

	ActionEffect use = { {none, none}, {none, none} };

	float weight = 0.f;
	float liquidAmount = 0.f;
	Liquid coveredIn = nothing; //liquids
	Liquid heldLiquid = nothing;
	int ticksUntilDry = 0;
	int initialTickTime = 0;
	int ticksUntilCooked = 16;
	equipType eType = notEquip;
	int mod = 0;
	std::string sprite = "#";
	int emissionDist = 0;
	Vector3 spriteColor = {1,1,1};
	float maxDurability = -1.f;
	float durability = 0.f;
	bool waterproof = false;

	void CoverIn(Liquid l, int ticks) {
		coveredIn = l;
		ticksUntilDry = ticks;
		initialTickTime = ticks;
	}

	void CreateFromData(OpenedData item) {
		
		name = item.getString("name");
		section = item.section_name;
		description = item.getString("description");
		//id = item.getString("id");
		stackable = item.getBool("stackable");
		holdsLiquid = item.getBool("holdsLiquid");
		consumable = item.getBool("consumable");
		count = item.getInt("amount");
		consumeTxt = item.getString("consumeTxt");
		useTxt = item.getString("useTxt");
		eType = (equipType)item.getInt("equipType");
		sprite = item.getString("sprite");

		try {
			maxDurability = item.getFloat("durability");
			durability = maxDurability;
		}//this is fine if it doesnt work, not all items are cookable and shouldnt need to specify
		catch (std::exception e) { maxDurability = -1.f; }

		try {
			waterproof = item.getBool("waterproof");
		}//this is fine if it doesnt work, not all items are waterproof and shouldnt need to specify
		catch (std::exception e) { waterproof = false; }

		try {
			cookable = item.getBool("cookable");
			cooks_into = item.getString("cooksInto");
		}//this is fine if it doesnt work, not all items are cookable and shouldnt need to specify
		catch (std::exception e) { cookable = false; }

		try {
			emissionDist = item.getInt("emissionDistance");
		}//this is fine if it doesnt work, not all items are emissive
		catch (std::exception e) { emissionDist = 0; }

		try {
			if (item.getArray("spriteColor").size() <= 0) {
				spriteColor = { 1,0,0 };
			}
			else {
				spriteColor.x = stof(item.getArray("spriteColor")[0]);
				spriteColor.y = stof(item.getArray("spriteColor")[1]);
				spriteColor.z = stof(item.getArray("spriteColor")[2]);
			}
		}
		catch (std::exception e) {
			Console::Log("ERROR: Couldn't find spriteColor parameter for item '" + name + "'.", text::red, __LINE__);
			spriteColor = { 1,0,0 };
		}

		std::vector<std::string> effects = item.getArray("effects");

		if(eType != none) mod = item.getInt("equipModifier");
		

		use = { {(ConsumeEffect)stoi(effects[0]), (float)stoi(effects[1])},
				{(ConsumeEffect)stoi(effects[2]), (float)stoi(effects[3])} };
	}
};

struct Container {
	std::vector<Item> items;
	int sizeLimit = 999;

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

	bool AddItem(Item item, int amount = 1) {

		if (items.size() >= sizeLimit) { return false; }
		
		Item it = item;
		it.count = amount;
		items.push_back(it);
		return true;
	}
};

//Health, Name, ID, Behaviour, Aggressive, Faction, View Distance, Damage, Can Talk
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
	int tempViewDistance;

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
	float visualTemp, bodyTemp = 98.5f;

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
		if (coveredIn == liquid) { ticksCovered -= turns; }
		ticksCovered = ticksCovered < 0 ? 0 : ticksCovered;
		coveredIn = liquid;
	}
};
struct Tile;

struct Saved_Container {
	int x, y;
	std::vector<std::string> items;
	void Serialize(std::ofstream& stream) {
		// Write the number of strings
		size_t numStrings = items.size();
		stream.write(reinterpret_cast<const char*>(&numStrings), sizeof(numStrings));

		// Write each string
		for (const auto& str : items) {
			size_t length = str.size();
			stream.write(reinterpret_cast<const char*>(&length), sizeof(length)); // Write string length
			stream.write(str.data(), length); // Write string data
		}
		stream.write(reinterpret_cast<const char*>(&x), sizeof(x));
		stream.write(reinterpret_cast<const char*>(&y), sizeof(y));
	}

	void Deserialize(std::ifstream& stream){
		// Read the number of strings
		size_t numStrings;
		stream.read(reinterpret_cast<char*>(&numStrings), sizeof(numStrings));

		// Read each string
		std::vector<std::string> strings;
		for (size_t i = 0; i < numStrings; ++i) {
			size_t length;
			stream.read(reinterpret_cast<char*>(&length), sizeof(length)); // Read string length

			std::string str(length, '\0'); // Allocate memory for the string
			stream.read(&str[0], length); // Read string data
			items.push_back(std::move(str));
		}
		stream.read(reinterpret_cast<char*>(&x), sizeof(x)); 
		stream.read(reinterpret_cast<char*>(&y), sizeof(y)); 

	}
};

struct Saved_Tile {
	int id = -1;
	Liquid liquid = nothing;
	int burningFor = 0;
	int ticksPassed = 0;
	int ticksNeeded = 1;
	int liquidTicks = 0;
	bool hasItem = false;
	std::string itemName = "NULL";
	int x, y = 0;
	short biomeID;
	bool hasContainer = false;
	Saved_Container cont;

	void Serialize(std::ofstream& stream) {
		stream.write(reinterpret_cast<const char*>(&id), sizeof(id));
		stream.write(reinterpret_cast<const char*>(&liquid), sizeof(liquid));

		stream.write(reinterpret_cast<const char*>(&burningFor), sizeof(burningFor));
		stream.write(reinterpret_cast<const char*>(&ticksPassed), sizeof(ticksPassed));
		stream.write(reinterpret_cast<const char*>(&ticksNeeded), sizeof(ticksNeeded));
		stream.write(reinterpret_cast<const char*>(&liquidTicks), sizeof(liquidTicks));
		stream.write(reinterpret_cast<const char*>(&hasItem), sizeof(hasItem));
		stream.write(reinterpret_cast<const char*>(&x), sizeof(x));
		stream.write(reinterpret_cast<const char*>(&y), sizeof(y));
		stream.write(reinterpret_cast<const char*>(&biomeID), sizeof(biomeID));
		stream.write(reinterpret_cast<const char*>(&hasContainer), sizeof(hasContainer));
		if (hasContainer) {
			cont.Serialize(stream);
		}

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
		stream.read(reinterpret_cast<char*>(&liquidTicks), sizeof(liquidTicks));
		stream.read(reinterpret_cast<char*>(&hasItem), sizeof(hasItem));
		stream.read(reinterpret_cast<char*>(&x), sizeof(x));
		stream.read(reinterpret_cast<char*>(&y), sizeof(y));
		stream.read(reinterpret_cast<char*>(&biomeID), sizeof(biomeID));
		stream.read(reinterpret_cast<char*>(&hasContainer), sizeof(hasContainer));
		if (hasContainer) {
			cont.Deserialize(stream);
		}

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
	direction technical_dir = direction::up;
	Liquid liquid = nothing;
	Entity* entity = nullptr;
	Container* tileContainer = nullptr;
	bool collectible = false;
	std::string collectibleName = "NULL";
	std::string collectedReplacement;
	std::string itemName = "NULL";
	int burningFor = 0;
	bool walkable = true;

	bool changesOverTime = false;
	std::string timedReplacement;
	int ticksPassed = 0;
	int ticksNeeded = 1;
	bool hasItem = false;
	float brightness = 1.f;
	int liquidTime = 0;
	bool visited = false;
	bool technical_update = false;
	vec2_i coords;
	bool double_size = false;
	vec3 tileColor;
	vec3 mainTileColor;
	short biomeID;

	bool CanUpdate() {
		return ticksPassed >= ticksNeeded && changesOverTime;
	}

	void ResetColor() {
		tileColor = mainTileColor;
	}

	void SetLiquid(Liquid l, bool perm = false) {
		liquid = l;
		liquidTime = perm ? -1 : 0;
		switch (l) {
		case water:// check if the water placed is like permanent like a body of water, and choose the color based on biome
			//I HOPE THIS NEVER BREAKS AGAIN BUT IM SURE IT WILL
			//BECAUSE THIS IS SO GROSSLY HARDCODED AND I CANNOT FIGURE OUT ANOTHER WAY SO IM DONE AND IT WORKS
			if (perm) {
				switch (biomeID) {
				case taiga:
					tileColor = mainTileColor;
					tileColor = { 0,0.5,1 };
					break;
				case swamp:
					tileColor = mainTileColor;
					tileColor = { 0.f, 0.5f, 0.4f };
				}
			}
			else {
				tileColor = mainTileColor;
				tileColor += { 0, 0.75, 1.5 };
				tileColor /= 2.5;
			}
			break;
		case guts:
			tileColor = { 0.45, 0, 0 };
			break;
		case blood:
			tileColor = { 1, 0, 0 };
			break;
		case nothing:
			Utilities::Lerp("tileColor" + std::to_string(Math::RandInt(1, 50000)), &tileColor, mainTileColor, 0.5f);
			break;
		}
		 
	}


	void CreateFromData(OpenedData data) {
		id = data.getInt("id");
		technical_dir = (direction)data.getInt("direction");
		liquid = (Liquid)data.getInt("liquid");
		collectible = data.getBool("collectible");
		collectedReplacement = data.getString("replacement");
		itemName = data.getString("itemName");
		hasItem = data.getBool("hasItem");
		walkable = data.getBool("walkable");
		changesOverTime = data.getBool("changesOverTime");
		timedReplacement = data.getString("timedReplacement");
		double_size = data.getBool("double_size");

		if (data.getArray("color").size() <= 0) {
			tileColor = { 1,0,0 };
		}
		else {
			tileColor.x = stof(data.getArray("color")[0]);
			tileColor.y = stof(data.getArray("color")[1]);
			tileColor.z = stof(data.getArray("color")[2]);
		}
		try { //not everything is collectible, dont require it
			collectibleName = data.getString("collectibleName");
		} catch (std::exception e) { }

		mainTileColor = tileColor;
	}
};

static void CreateSavedTile(Saved_Tile* sTile, Tile tile) {
	sTile->id = tile.id;
	sTile->liquid = tile.liquid;
	sTile->burningFor = tile.burningFor;
	sTile->ticksPassed = tile.ticksPassed;
	sTile->ticksNeeded = tile.ticksNeeded;
	sTile->liquidTicks = tile.liquidTime;
	sTile->hasItem = tile.hasItem;
	sTile->itemName = tile.itemName;
	sTile->x = tile.coords.x;
	sTile->y = tile.coords.y;
	sTile->biomeID = tile.biomeID;

	if (tile.tileContainer != nullptr) {
		sTile->hasContainer = true;
		sTile->cont.x = tile.coords.x;
		sTile->cont.y = tile.coords.y;
		for (auto const& item : tile.tileContainer->items) {
			sTile->cont.items.push_back(item.section);
		}
	}
}

class Inventory;

class CraftingSystem {
public:
	void addRecipe(std::string output, std::map<std::string, int> inputs) {
		for (auto const& input_item : inputs) {
			if (recipes_by_item.count(input_item.first) == 0) { recipes_by_item[input_item.first] = {}; }
			recipes_by_item[input_item.first].push_back(output);
		}
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

	std::vector<std::string> getRecipesByItem(std::string itemName) {
		bool isRecipe = false;
		if (recipes.count(itemName) != 0) { isRecipe = true; }
		else if (recipes_by_item.count(itemName) == 0 && !isRecipe) { return { }; }

		std::vector<std::string> recipeList = recipes_by_item[itemName];
		if (isRecipe) { recipeList.insert(recipeList.begin(), itemName); }

		return recipeList;
	}

	void SaveRecipe(std::string recName) {
		savedRecipes.insert(recName);
	}
	void UnsaveRecipe(std::string recName) {
		savedRecipes.erase(recName);
	}

	std::set<std::string> savedRecipes;
private:

	std::unordered_map<std::string, std::map<std::string, int>> recipes;

	//				  --item_name   --list of recipes using the item
	std::unordered_map<std::string, std::vector<std::string>> recipes_by_item;
};


#define ENT_PLAYER "A"
#define ID_PLAYER 0

#define ENT_ZOMBIE "B"
#define ID_ZOMBIE 1

#define ENT_CHICKEN "D"
#define ID_CHICKEN 2

#define ENT_HUMAN "C"
#define ID_HUMAN 3

#define ENT_FROG "F"
#define ID_FROG 4

#define ENT_CAT "G"
#define ID_CAT 5

#define ENT_COW "a"
#define ID_COW 6

#define ENT_FINDER "H"
#define ID_FINDER 7

#define ENT_TAKER "I"
#define ID_TAKER 8


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
