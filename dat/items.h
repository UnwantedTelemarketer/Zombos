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

	Tile* GetTile(int ID) {
		//get the name by the id, and then get the item. Used for loading saved chunks to cut down on data that needss to be saved on disk vs ram
		return &list[nameByID[ID]];
	}

	static void LoadTiles(std::map<int, vec3>* tileColors) {
		Console::Log("Loading Items from files...", text::white, __LINE__);
		std::vector<std::string> sections;
		OpenedData data;
		ItemReader::GetDataFromFile("tiles/tile_lists.eid", "LISTS", &data);
		for (auto const& x : data.tokens) {

			OpenedData sectionsData;
			ItemReader::GetDataFromFile("tiles/" + x.first, "SECTIONS", &sectionsData);
			for (auto const& tileSection : sectionsData.getArray("sections")) {
				list[tileSection] = EID::MakeTile("tiles/" + x.first, tileSection);
				nameByID[list[tileSection].id] = tileSection;
				tileColors->insert({ list[tileSection].id,list[tileSection].tileColor });
			}
		}
		Console::Log("Loaded Items!", text::white, __LINE__);
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
		createdTile->hasItem = tile.hasItem;
		createdTile->itemName = tile.itemName;
		createdTile->coords = { tile.x, tile.y };
		createdTile->walkable = Tiles::GetTile(tile.id)->walkable;
		createdTile->mainTileColor = Tiles::GetTile(tile.id)->mainTileColor;

		createdTile->tileColor = createdTile->mainTileColor;
		createdTile->SetLiquid(tile.liquid);
	}
}

namespace Items {
	static std::map<std::string, Item> list;
	static std::map<std::string, Vector3> colors;

	static Item GetItem(std::string itemName) {
		return list[itemName];
	}

	static Item* GetItem_NoCopy(std::string itemName) {
		return &list[itemName];
	}

	static Vector3 GetItemColor (std::string itemName) {
		return colors[itemName];
	}

	static void LoadItems(std::map<std::string, std::string>* icons) {
		Console::Log("Loading Tiles from files...", text::white, __LINE__);
		std::vector<std::string> sections;
		OpenedData data;
		ItemReader::GetDataFromFile("items/item_lists.eid", "LISTS", &data);
		for (auto const& x : data.tokens) {
			//Console::Log(x.first, text::blue, __LINE__);

			OpenedData sectionsData;
			ItemReader::GetDataFromFile("items/" + x.first, "SECTIONS", &sectionsData);
			for (auto const& itemName : sectionsData.getArray("sections")) {
				list[itemName] = EID::MakeItem("items/" + x.first, itemName);
				colors[itemName] = list[itemName].spriteColor;
				icons->insert({ list[itemName].section , list[itemName].sprite });
			}
		}
		Console::Log("Loaded Tiles!", text::white, __LINE__);
	}

	static std::thread LoadItemsFromFiles(std::map<std::string, std::string>* icons) {
		std::thread loadingItems{ LoadItems, icons };
		return loadingItems;
	}
}



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


static std::unordered_map<int, Tile> tileByID = 
{
	{ID_CONVEYOR_U, {ID_CONVEYOR_U, direction::up}},
	{ID_CONVEYOR_D, {ID_CONVEYOR_D, direction::down}},
	{ID_CONVEYOR_L, {ID_CONVEYOR_L, direction::left}},
	{ID_CONVEYOR_R, {ID_CONVEYOR_R, direction::right}},
	{ID_CONVEYOR_UL, {ID_CONVEYOR_UL}},
	{ID_CONVEYOR_UR, {ID_CONVEYOR_UR}},
	{ID_CONVEYOR_DL, {ID_CONVEYOR_DL}},
	{ID_CONVEYOR_DR, {ID_CONVEYOR_DR}},
};