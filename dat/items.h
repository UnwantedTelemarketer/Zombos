#pragma once
#include "entities.h"
#include <map>
#include <thread>

#define ID_NULL -1

#define ID_GRASS 1

#define ID_DIRT 2

#define ID_TREE_BASE 11

#define ID_CACTUS_BASE 12

#define ID_FLOWER 3

#define ID_FIRE 4

#define ID_SCRAP 5

#define ID_STONE 6

#define ID_SAND 7

#define ID_CONTAINER 8

#define ID_CONTAINER_OPEN 9

#define ID_STICK 10


#define ID_CONVEYOR_U 100

#define ID_CONVEYOR_D 101

#define ID_CONVEYOR_L 102

#define ID_CONVEYOR_R 103

#define ID_CONVEYOR_UL 104

#define ID_CONVEYOR_UR 105

#define ID_CONVEYOR_DL 106

#define ID_CONVEYOR_DR 107

namespace EID {
	//ONLY REQUIRES FILENAME, AUTO NAVIGATES TO THE DAT FOLDER
	static Item MakeItem(std::string file, std::string header) {
		OpenedData data;
		ItemReader::GetDataFromFile(file, header, &data);
		Item item;
		item.CreateFromData(data);

		return item;
	}
	static Tile MakeTile (std::string file, std::string header) {
		OpenedData data;
		ItemReader::GetDataFromFile(file, header, &data);
		Tile tile;
		tile.CreateFromData(data);

		return tile;
	}
}

namespace Tiles {
	static std::map<std::string, Tile> list;
	static std::map<int, std::string> nameByID;

	Tile GetTile(std::string name) {
		return list[name];
	}

	Tile GetTileByID(int ID) {
		return list[nameByID[ID]];
	}

	Tile* GetTile(int ID) {
		//get the name by the id, and then get the item. Used for loading saved chunks to cut down on data that needss to be saved on disk vs ram
		return &list[nameByID[ID]];
	}

	static void LoadTiles(std::map<int, vec3>* tileColors) {
		Console::Log("Loading Tiles from files...", WARNING, __LINE__);
		std::vector<std::string> sections;
		OpenedData data;
		ItemReader::GetDataFromFile("tiles/tile_lists.eid", "LISTS", &data);
		for (auto const& x : data.tokens) {

			std::string fixedPath = x.first;
			//damn new lines
			if (fixedPath[0] == '\r') { fixedPath.erase(fixedPath.begin()); }
			Console::Log(fixedPath, text::white, __LINE__);

			OpenedData sectionsData;
			ItemReader::GetDataFromFile("tiles/" + fixedPath, "SECTIONS", &sectionsData);
			for (auto const& tileSection : sectionsData.getArray("sections")) {
				list[tileSection] = EID::MakeTile("tiles/" + fixedPath, tileSection);
				nameByID[list[tileSection].id] = tileSection;
				tileColors->insert({ list[tileSection].id,list[tileSection].tileColor });
			}
		}
		Console::Log("Loaded Tiles!", SUCCESS, __LINE__);
	}

	static std::thread LoadTilesFromFiles(std::map<int, vec3>* tileColors) {
		std::thread loadingTiles{ LoadTiles, tileColors };
		return loadingTiles;
	}

	static void LoadTile(Tile* createdTile, Saved_Tile tile) {
		createdTile->id = tile.id;
		createdTile->burningFor = tile.burningFor;
		createdTile->ticksPassed = tile.ticksPassed;
		createdTile->ticksNeeded = tile.ticksNeeded;
		createdTile->liquidTime = tile.liquidTicks;
		createdTile->itemName = tile.itemName == "_" ? "NULL" : tile.itemName;
		createdTile->hasItem = tile.itemName != "_";
		createdTile->biomeID = tile.biomeID;

		//load static data from the tile to save file size
		createdTile->walkable = Tiles::GetTile(tile.id)->walkable;
		createdTile->collectible = Tiles::GetTile(tile.id)->collectible;
		createdTile->collectibleName = Tiles::GetTile(tile.id)->collectibleName;
		createdTile->collectedReplacement = Tiles::GetTile(tile.id)->collectedReplacement;
		createdTile->mainTileColor = Tiles::GetTile(tile.id)->mainTileColor;
		createdTile->casts_shadow = Tiles::GetTile(tile.id)->casts_shadow;

	}
}

namespace Items {
	static std::map<std::string, Item> list;
	static std::map<std::string, Vector3> colors;
	static std::vector<equipType> EquipmentTypes = { weapon, hat, shirt, pants, boots, gloves, neck, back };

	static Item GetItem(std::string itemName) {
		return list[itemName];
	}

	static Item* GetItem_NoCopy(std::string itemName) {
		return &list[itemName];
	}

	static Vector3 GetItemColor (std::string itemName) {
		return colors[itemName];
	}

	static Item GetRandomItem() {
		static std::random_device rd;
		static std::mt19937 gen(rd());

		std::uniform_int_distribution<size_t> dist(0, list.size() - 1);

		size_t index = dist(gen);
		auto it = list.begin();
		std::advance(it, index);

		return it->second;
	}

	static std::string GetRandomItemFromPool(std::string filename) {
		OpenedData dat;
		int rarity = Math::RandInt(0, 100);
		if (rarity >= 92) {
			ItemReader::GetDataFromFile("loot_tables/" + filename, "VERY_RARE", &dat);
		}
		else if (rarity >= 72) {
			ItemReader::GetDataFromFile("loot_tables/" + filename, "RARE", &dat);
		}
		else {
			ItemReader::GetDataFromFile("loot_tables/" + filename, "COMMON", &dat);
		}
		int randItem = Math::RandInt(0, dat.getArray("items").size() - 1);
		return dat.getArray("items")[randItem];
	}

	static void LoadItems(std::map<std::string, std::string>* icons) {
		Console::Log("Loading Items from files...", WARNING, __LINE__);
		std::vector<std::string> sections;
		OpenedData data;
		ItemReader::GetDataFromFile("items/item_lists.eid", "LISTS", &data);
		for (auto const& x : data.tokens) {
			//Console::Log(x.first, text::blue, __LINE__);

			OpenedData sectionsData;
			std::string fixedName = x.first;
			std::string fixedPath = "items/";

			//damn new lines
			if (fixedName[0] == '\r') { fixedName.erase(fixedName.begin()); }
			fixedPath.append(fixedName);

			Console::Log(fixedPath, text::white, __LINE__);

			ItemReader::GetDataFromFile(fixedPath, "SECTIONS", & sectionsData);
			for (auto const& itemName : sectionsData.getArray("sections")) {
				list[itemName] = EID::MakeItem("items/" + x.first, itemName);
				colors[itemName] = list[itemName].spriteColor;
				icons->insert({ list[itemName].section , list[itemName].sprite });
			}
		}
		Console::Log("Loaded Items!", SUCCESS, __LINE__);
	}

	static std::thread LoadItemsFromFiles(std::map<std::string, std::string>* icons) {
		std::thread loadingItems{ LoadItems, icons };
		return loadingItems;
	}
}

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
			items.insert({ (*inventory)[i].section, &(*inventory)[i] });
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
			recipeComps.push_back(Items::GetItem_NoCopy(name.first)->name + " x " + std::to_string(name.second));
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



/*
Tile Tile_Big_Rock =
{
	15, // Starting block ID
	still, //technical direction (for conveyors)
	nothing, //Liquid
	nullptr, //Entity
	false, // Collectible
	-1, // Block that it becomes after being collected
	"NULL", //Item name
	0, // how long its burn lasts
	false, //walkable
	false, //changes over time
	-1 //what it becomes after a time limit
};

Tile Tile_Snow =
{
	16, // Starting block ID
	still, //technical direction (for conveyors)
	snow, //Liquid
	nullptr, //Entity
	true, // Collectible
	ID_DIRT, // Block that it becomes after being collected
	"SNOW", //Item name
	0, // how long its burn lasts
	true, //walkable
	false, //changes over time
	-1 //what it becomes after a time limit
};

Tile Tile_Mud =
{
	17, // Starting block ID
	still, //technical direction (for conveyors)
	mud, //Liquid
	nullptr, //Entity
	false, // Collectible
	ID_DIRT, // Block that it becomes after being collected
	"MUD", //Item name
	0, // how long its burn lasts
	true, //walkable
	false, //changes over time
	-1 //what it becomes after a time limit
};*/

//============================  CONVEYOR BELTS ==============================


/*static std::unordered_map<int, Tile> tileByID =
{
	{ID_CONVEYOR_U, {ID_CONVEYOR_U, direction::up}},
	{ID_CONVEYOR_D, {ID_CONVEYOR_D, direction::down}},
	{ID_CONVEYOR_L, {ID_CONVEYOR_L, direction::left}},
	{ID_CONVEYOR_R, {ID_CONVEYOR_R, direction::right}},
	{ID_CONVEYOR_UL, {ID_CONVEYOR_UL}},
	{ID_CONVEYOR_UR, {ID_CONVEYOR_UR}},
	{ID_CONVEYOR_DL, {ID_CONVEYOR_DL}},
	{ID_CONVEYOR_DR, {ID_CONVEYOR_DR}},
};*/