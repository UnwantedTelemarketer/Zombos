#pragma once
#include "entities.h"

#define ITEM_CONTAINER "container"
#define ITEM_BANDAGE "bandage"
#define ITEM_GRASS "grass"
#define ITEM_SCRAP "scrap"
#define ITEM_STICK "stick"
#define ITEM_KNIFE "knife"
#define ITEM_MONEY "bits"
#define ITEM_ROPE "rope"
#define ITEM_ROCK "rock"

namespace EID {
	static Item MakeItem(std::string file, std::string header) {
		OpenedData data;
		ItemReader::GetDataFromFile(file, header, &data);
		Item item;
		item.CreateFromData(data);

		return item;
	}
}

Tile Tile_Template =
{
	-1, // Starting block
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
	nothing,
	nullptr,
	true,
	ID_GRASS,
	"GRASS"
};

Tile Tile_Stick =
{
	10,
	nothing,
	nullptr,
	true,
	ID_GRASS,
	"STICK"
};

Tile Tile_Dirt =
{
	ID_DIRT,
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
	nothing,
	nullptr,
	false,
	ID_NULL,
	"NULL",
	-1,
	false
};

static std::unordered_map<int, Tile> tileByID = 
{
	{ID_GRASS, Tile_Grass},
	{ID_FLOWER, Tile_TallGrass},
	{ID_DIRT, Tile_Dirt},
	{ID_SCRAP, Tile_Scrap},
	{ID_STONE, Tile_Stone},
	{ID_SAND, Tile_Sand},
	{ID_CONTAINER, Tile_Container},
	{10, Tile_Stone}
};