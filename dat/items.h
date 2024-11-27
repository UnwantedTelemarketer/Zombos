#pragma once
#include "entities.h"

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
}

namespace Items {
	static std::map<std::string, Item> list;

	static void LoadItemsFromFiles() {
		std::vector<std::string> sections;
		OpenedData data;
		ItemReader::GetDataFromFile("items/item_lists.eid", "LISTS", &data);
		for (auto const& x : data.tokens) {
			Console::Log(x.first, text::blue, __LINE__);

			OpenedData sectionsData;
			ItemReader::GetDataFromFile("items/" + x.first, "SECTIONS", &sectionsData);
			Console::Log(sectionsData.getArray("sections"), text::green, __LINE__);
			for (auto const& itemNames : sectionsData.getArray("sections")) {
				sections.push_back(itemNames);
			}


			/*OpenedData currentItemFile;
			ItemReader::GetDataFromFile("items/" + x.first, "_ALL", &data);
			for (auto const& x : currentItemFile.tokens) {
				list[]
			}*/
		}
	}
}


Tile Tile_Template =
{
	-1, // Starting block
	still, //technical direction (for conveyors)
	nothing, //Liquid
	nullptr, //Entity
	false, // Collectible
	-1, // Block that it becomes after being collected
	"NULL", //Item name
	0, // how long its burn lasts
	true, //walkable
	false, //changes over time
	-1 //what it becomes after a time limit
};

Tile Tile_Container =
{
	ID_CONTAINER, //Starting Block
	still,
	nothing,  //Liquid
	nullptr,  //Entity
	true,	  //Collectible
	ID_CONTAINER_OPEN, //Block after collection
	"BAD_PISTOL",
	0,
	false,
	false
};

Tile Tile_Container_Open =
{
	ID_CONTAINER_OPEN, //Starting Block
	still,
	nothing,  //Liquid
	nullptr,  //Entity
	false,	  //Collectible
	ID_CONTAINER_OPEN, //Block after collection
	"NULL",
	0,
	false,
	false
};

Tile Tile_Grass =
{
	ID_GRASS, //Starting Block
	still,
	nothing,  //Liquid
	nullptr,  //Entity
	false,	  //Collectible
	ID_GRASS, //Block after collection
	"NULL",
	0,
	true,
	true,
	ID_FLOWER
};

Tile Tile_TallGrass =
{
	ID_FLOWER,
	still,
	nothing,
	nullptr,
	true,
	ID_GRASS,
	"GRASS"
};

Tile Tile_Stick =
{
	10,
	still,
	nothing,
	nullptr,
	true,
	ID_GRASS,
	"STICK"
};

Tile Tile_Dirt =
{
	ID_DIRT,
	still,
	nothing,
	nullptr,
	false,
	ID_DIRT,
	"NULL",
	-1,
	true,
	true,
	ID_GRASS
};

Tile Tile_Sand =
{
	ID_SAND,
	still,
	nothing,
	nullptr,
	false,
	ID_SAND,
	"NULL",
	-1,
	true,
	false
};

Tile Tile_Scrap =
{
	ID_SCRAP,
	still,
	nothing,
	nullptr,
	true,
	ID_FLOWER,
	"SCRAP",
	-1
};

Tile Tile_Stone =
{
	ID_STONE,
	still,
	nothing,
	nullptr,
	false,
	ID_NULL,
	"NULL",
	-1,
	false
};

Tile Tile_Tree_Base =
{
	ID_TREE_BASE,
	still,
	nothing,
	nullptr,
	false,
	ID_NULL,
	"NULL",
	0,
	true
};

Tile Tile_Cactus_Base =
{
	ID_CACTUS_BASE,
	still,
	nothing,
	nullptr,
	false,
	ID_NULL,
	"NULL",
	0,
	false
};

Tile Tile_Stone_Floor =
{
	13, // Starting block ID
	still, //technical direction (for conveyors)
	nothing, //Liquid
	nullptr, //Entity
	false, // Collectible
	-1, // Block that it becomes after being collected
	"NULL", //Item name
	0, // how long its burn lasts
	true, //walkable
	false, //changes over time
	-1 //what it becomes after a time limit
}; 

Tile Tile_Crystal =
{
	14, // Starting block ID
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

//============================  CONVEYOR BELTS ==============================


static std::unordered_map<int, Tile> tileByID = 
{
	{ID_GRASS, Tile_Grass},
	{ID_FLOWER, Tile_TallGrass},
	{ID_DIRT, Tile_Dirt},
	{ID_SCRAP, Tile_Scrap},
	{ID_STONE, Tile_Stone},
	{ID_SAND, Tile_Sand},
	{ID_CONTAINER, Tile_Container},
	{ID_CONVEYOR_U, {ID_CONVEYOR_U, up}},
	{ID_CONVEYOR_D, {ID_CONVEYOR_D, down}},
	{ID_CONVEYOR_L, {ID_CONVEYOR_L, left}},
	{ID_CONVEYOR_R, {ID_CONVEYOR_R, right}},
	{ID_CONVEYOR_UL, {ID_CONVEYOR_UL}},
	{ID_CONVEYOR_UR, {ID_CONVEYOR_UR}},
	{ID_CONVEYOR_DL, {ID_CONVEYOR_DL}},
	{ID_CONVEYOR_DR, {ID_CONVEYOR_DR}},
	{10, Tile_Stone}
};