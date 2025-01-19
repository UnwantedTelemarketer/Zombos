#include "items.h"

#define CHUNK_WIDTH 30
#define CHUNK_HEIGHT 30

struct Saved_Chunk {
	Saved_Tile tiles[CHUNK_WIDTH][CHUNK_HEIGHT];
};

struct Chunk {
private:
	std::vector<Container*> containers;
public:
	bool beenBuilt = false;
	Vector2_I globalChunkCoord;
	Tile localCoords[CHUNK_WIDTH][CHUNK_HEIGHT];
	std::vector<Entity*> entities;

	Tile* GetTileAtCoords(int x, int y) {
		return GetTileAtCoords({ x, y });
	}
	Tile* GetTileAtCoords(Vector2_I pos) {
		bool xInvalid = false, yInvalid = false;
		if (pos.x >= CHUNK_WIDTH) { xInvalid = true; pos.x = CHUNK_WIDTH - 1; }
		if (pos.x < 0) { xInvalid = true; pos.x = 0; }
		if (pos.y >= CHUNK_HEIGHT) { yInvalid = true; pos.y = CHUNK_HEIGHT - 1; }
		if (pos.y < 0) { yInvalid = true; pos.y = 0; }
		if (xInvalid && yInvalid) { return nullptr; }

		return &localCoords[pos.x][pos.y];
	}

	void LoadChunk(Vector2_I coords) {

		Saved_Chunk sChunk;
		std::string filePath = ("dat/map/" + std::to_string(coords.x) + std::to_string(coords.y) + ".chunk");

		std::ifstream inFile(filePath, std::ios::binary);
		if (inFile.is_open()) {

			for (size_t i = 0; i < CHUNK_WIDTH; i++)
			{
				for (size_t j = 0; j < CHUNK_HEIGHT; j++)
				{
					sChunk.tiles[i][j].Deserialize(inFile);

				}
			}

			inFile.close();
		}

		globalChunkCoord = coords;

		//once deserialized, read the data into the tiles
		for (size_t i = 0; i < CHUNK_WIDTH; i++)
		{
			for (size_t j = 0; j < CHUNK_HEIGHT; j++)
			{
				Tiles::LoadTile(&localCoords[i][j], sChunk.tiles[i][j]);
				//Correctly load the correct color of tile if its grass
				if (localCoords[i][j].id == 1) 
				{
					switch (localCoords[i][j].biomeID) {
					case (short)swamp:
						localCoords[i][j].mainTileColor = { 0.5, 0.55, 0.35 };
						localCoords[i][j].mainTileColor.y += ((float)(Math::RandNum(30) - 15) / 100);
						break;
					case (short)taiga:
						localCoords[i][j].mainTileColor.y += ((float)(Math::RandNum(30) - 15) / 100);
						localCoords[i][j].mainTileColor.z = 0.35f;
						
						break;
					}
				}
				localCoords[i][j].ResetColor();
				if (sChunk.tiles[i][j].liquid != nothing) {
					localCoords[i][j].SetLiquid(sChunk.tiles[i][j].liquid, localCoords[i][j].liquidTime == -1);
				}


				if (sChunk.tiles[i][j].hasContainer) {
					localCoords[i][j].tileContainer = new Container;
					containers.push_back(localCoords[i][j].tileContainer);
					for (auto const& item : sChunk.tiles[i][j].cont.items) {
						localCoords[i][j].tileContainer->AddItem(Items::GetItem(item));
					}
				}
			}
		}
	}

	void SaveChunk() {
		std::string filePath = ("dat/map/" + std::to_string(globalChunkCoord.x) + std::to_string(globalChunkCoord.y) + ".chunk");
		std::ofstream outFile(filePath, std::ios::binary);

		Saved_Chunk *sChunk = new Saved_Chunk();
		for (size_t i = 0; i < CHUNK_WIDTH; i++)
		{
			for (size_t j = 0; j < CHUNK_HEIGHT; j++)
			{
				CreateSavedTile(&sChunk->tiles[i][j], localCoords[i][j]);
			}
		}

		for (size_t i = 0; i < CHUNK_WIDTH; i++)
		{
			for (size_t j = 0; j < CHUNK_HEIGHT; j++)
			{
				sChunk->tiles[i][j].Serialize(outFile);
			}
		}
		delete sChunk;
		for (auto const& cont : containers) {
			delete cont;
		}
		for (auto const& ent : entities) {
			delete ent;
		}
		outFile.close();
	}
};

static void CreateSavedChunk(Saved_Chunk* sChunk, Chunk ch) {
	for (size_t i = 0; i < CHUNK_WIDTH; i++)
	{
		for (size_t j = 0; j < CHUNK_HEIGHT; j++)
		{
			sChunk->tiles[i][j].id = ch.localCoords[i][j].id;
			sChunk->tiles[i][j].liquid = ch.localCoords[i][j].liquid;
			sChunk->tiles[i][j].burningFor = ch.localCoords[i][j].burningFor;
			sChunk->tiles[i][j].ticksPassed = ch.localCoords[i][j].ticksPassed;
			sChunk->tiles[i][j].ticksNeeded = ch.localCoords[i][j].ticksNeeded;
			sChunk->tiles[i][j].hasItem = ch.localCoords[i][j].hasItem;
			sChunk->tiles[i][j].itemName = ch.localCoords[i][j].itemName;
			sChunk->tiles[i][j].x = ch.localCoords[i][j].coords.x;
			sChunk->tiles[i][j].y = ch.localCoords[i][j].coords.y;
			sChunk->tiles[i][j].biomeID = ch.localCoords[i][j].biomeID;
		}
	}
}

struct T_Chunk //we dont need the current chunk copy to have an entity list
{
	int localCoords[CHUNK_WIDTH][CHUNK_HEIGHT];
};
