#pragma once
#include "antibox/tools/noise.h"
#include "fastnoiselite/FastNoiseLite.h"
#include "antibox/core/antibox.h"
#include "antibox/managers/factory.h"

#include "items.h"
#include "inventory.h"
#include "antibox/objects/tokenizer.h"
#include <algorithm>
#include <cmath>
#include <thread>
#include "Chunk.h"

#define MAP_UP 1
#define MAP_DOWN 2
#define MAP_LEFT 3
#define MAP_RIGHT 4
#define MAP_HEIGHT 500
#define MAP_WIDTH 500

using namespace antibox;

struct World
{
	std::map<Vector2_I, std::shared_ptr<Chunk>> chunks;
};


enum biome { desert, ocean, forest, swamp, taiga,grassland, urban, jungle };
enum weather {clear, rainy, thunder};
class Map {
public:
	bool chunkUpdateFlag = false;
	Vector2_I c_glCoords{ 250, 250 };
	T_Chunk effectLayer; //is the original chunk with visual effects on it (lines, fire, liquid)
	World world; //keeps the original generated map so that tiles walked over wont be erased
	World underground; //map but underground, different map entirely
	bool isUnderground;
	std::vector<Vector2_I> line;
	weather currentWeather;
	int ticksUntilWeatherUpdate = 0;

	//x and y are global coords, z and w are local
	std::map<Vector4_I, Container> containers;

	int landSeed = 0, biomeSeed = 0;
	float tempMin = 0.5f, moistureMin = 0.5f;
	FastNoiseLite mapNoise, biomeTempNoise, biomeMoistureNoise;
	
	
	
	void CreateMap(int seed, int b_seed);
	void UpdateMemoryZone(Vector2_I coords);
	void ReadChunk(Vector2_I curChunk, std::string path);
	void SpawnChunkEntities(std::shared_ptr<Chunk> chunk);
	std::shared_ptr<Chunk> CurrentChunk();
	Tile* TileAtPos(Vector2_I coords);
	Tile* GetTileFromThisOrNeighbor(Vector2_I tilecoords);
	int EffectAtPos(Vector2_I coords);
	int GetEffectFromThisOrNeighbor(Vector2_I tilecoords);
	void SetEffectInThisOrNeighbor(Vector2_I tilecoords, int effect);
	biome GetBiome(Vector2_I coords);
	void MakeNewChunk(Vector2_I coords);
	void AttackEntity(Entity* curEnt, int damage, std::vector<std::string>* actionLog);
	void MovePlayer(int x, int y, Player* p, std::vector<std::string>* actionLog);
	void CheckBounds(Player* p);
	void CheckBounds(Entity* p, std::shared_ptr<Chunk> chunk);
	void BuildChunk(std::shared_ptr<Chunk> chunk);
	void PlaceBuilding(std::shared_ptr<Chunk> chunk);
	void GenerateTomb(std::shared_ptr<Chunk> chunk);
	std::vector<Vector2_I> GetLine(Vector2_I startTile, Vector2_I endTile, int limit);
	void DrawLine(std::vector<Vector2_I> list);
	void ClearLine();
	void ClearEntities(std::vector<Vector2_I> positions, std::shared_ptr<Chunk> chunk);
	void ClearChunkOfEnts(std::shared_ptr<Chunk> chunk);
	void ClearEffects();
	void CreateContainer(Vector2_I coordsLocal);
	void RemoveContainer(Vector2_I coordsLocal);
	void CreateContainer(Vector4_I coords);
	Container* ContainerAtCoord(Vector2_I localCoords);
	void PlaceEntities(std::shared_ptr<Chunk> chunk);
	void UpdateTiles(vec2_i coords);
	void DoTechnical(Tile* curTile, std::shared_ptr<Chunk> chunk, int x, int y);
	void FixEntityIndex(std::shared_ptr<Chunk> chunk);
	void floodFillUtil(int x, int y, float prevBrightness, float newBrightness, int max, bool twinkle, bool firstTile);
	void floodFill(Vector2_I centerTile, int distance, bool twinkle);
	void ResetLightValues();
	bool UpdateWeather();
	void SetWeather(weather we);
	void SetupNoise(int l_seed, int b_seed);
	std::shared_ptr<Chunk> GetProperChunk(Vector2_I coords);

	~Map();

};

void Map::SetupNoise(int l_seed, int b_seed) {
	if (l_seed == -1) {
		landSeed = Math::RandInt(1, 2147483647);
	}
	else {
		landSeed = l_seed;
	}
	if (b_seed == -1) {
		biomeSeed = Math::RandInt(1, 2147483647);
	}
	else {
		biomeSeed = b_seed;
	}
	mapNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	mapNoise.SetFrequency(0.85f);
	biomeTempNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	biomeTempNoise.SetFrequency(0.025f);
	biomeMoistureNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	biomeMoistureNoise.SetFrequency(0.020f);

	mapNoise.SetSeed(landSeed);
	biomeTempNoise.SetSeed(biomeSeed);
	biomeMoistureNoise.SetSeed(Math::RandInt(1, 2147483647));
}

void Map::CreateMap(int l_seed, int b_seed)
{
	SetupNoise(l_seed, b_seed);
	
	UpdateMemoryZone(c_glCoords);

	for (int i = 0; i < 30; i++)
	{
		for (int j = 0; j < 30; j++)
		{
			effectLayer.localCoords[i][j] = 0;
		}
	}

}

static void T_SaveChunks(std::shared_ptr<Chunk> chunk) {
	chunk->SaveChunk();
}

//Change which chunks are in memory at one time
void Map::UpdateMemoryZone(Vector2_I coords) {

	//add new chunks in our area around player
	for (int y = -1; y <= 1; y++)
	{
		for (int x = -1; x <= 1; x++)
		{
			vec2_i curChunk = { c_glCoords.x + x, c_glCoords.y + y };
			std::string filename = "dat/map/" + std::to_string(curChunk.x) + std::to_string(curChunk.y) + ".chunk";

			//Check if theres a saved chunk. if so, load it
			if (world.chunks[curChunk] == nullptr) {
				if (fileExists(filename.c_str())) {
					ReadChunk(curChunk, filename);
				}
				else {
					MakeNewChunk(curChunk);
				}
			}
		}
	}

	std::vector<Vector2_I> chunksToDelete;
	//find ones too far away
	for (const auto& pair : world.chunks) {
		if (pair.first.x > c_glCoords.x + 1 || pair.first.x < c_glCoords.x - 1 ||
			pair.first.y > c_glCoords.y + 1 || pair.first.y < c_glCoords.y - 1) {
			chunksToDelete.push_back(pair.first);
		}
	}

	if (chunksToDelete.size() > 0) {
		//Save all three chunks to file across 3 threads
		std::vector<std::thread> threads;
		for (int i = 0; i < chunksToDelete.size(); ++i) {
			threads.push_back(std::thread(T_SaveChunks, world.chunks[chunksToDelete[i]]));
		}

		for (auto& th : threads) {
			th.detach();
		}
	}

	//remove them
	for (auto& vec : chunksToDelete) {
		//world.chunks[vec]->SaveChunk();
		world.chunks.erase(vec);
	}
	
}

bool Map::UpdateWeather() {
	ticksUntilWeatherUpdate--;
	if (ticksUntilWeatherUpdate == 0) {
		weather newWeather = (weather)Math::RandInt(0, 2);
		while (newWeather == currentWeather) {
			newWeather = (weather)Math::RandInt(0, 2);
		}
		ticksUntilWeatherUpdate = Math::RandInt(600, 1000);
		SetWeather(newWeather);
		return true;
	}
	return false;
}

void Map::SetWeather(weather we) {
	currentWeather = we;
	switch (currentWeather) {
	case thunder:
	case rainy:
		Audio::PlayLoop("dat/sounds/rain.mp3", "rain");
		break;
	case clear:
		Audio::StopLoop("rain");
		break;
	}
}

void Map::ReadChunk(Vector2_I curChunk, std::string path) {
	Console::Log("Reading Chunk from " + path, text::green, __LINE__);
	std::shared_ptr<Chunk> tempChunk = std::make_shared<Chunk>();
	tempChunk->globalChunkCoord = curChunk;
	tempChunk->LoadChunk(curChunk);
	world.chunks[curChunk] = tempChunk;
}


void Map::MakeNewChunk(Vector2_I coords) {
	std::shared_ptr<Chunk> tempChunk = std::make_shared<Chunk>();
	tempChunk->globalChunkCoord = coords;

	BuildChunk(tempChunk);
	SpawnChunkEntities(tempChunk);
	world.chunks[tempChunk->globalChunkCoord] = tempChunk;
}

void Map::SpawnChunkEntities(std::shared_ptr<Chunk> chunk)
{
	for (int i = 0; i < Math::RandInt(0, 4); i++) //CHANGE THIS TO SPAWN ENTITIES
	{
		Entity* zomb;
		int num = Math::RandInt(1, 10);
		if (num >= 9) {
			zomb = new Entity{ 35, "Human", ID_HUMAN, Protective, false, Human, 10, 10, true, Math::RandInt(1, CHUNK_WIDTH), Math::RandInt(1, CHUNK_HEIGHT), true };
		}
		else if (num >= 5) {
			zomb = new Entity{ 15, "Zombie", ID_ZOMBIE, Aggressive, true, Zombie, 7, 8, false, Math::RandInt(1, CHUNK_WIDTH), Math::RandInt(1, CHUNK_HEIGHT) };

			zomb->inv.push_back(Items::GetItem("OLD_CLOTH"));
			zomb->inv.push_back(Items::GetItem("GUTS"));
		}
		else {
			zomb = new Entity{ 10, "Chicken", ID_CHICKEN, Wander, false, Wildlife, 5, 1, false, Math::RandInt(1, CHUNK_WIDTH), Math::RandInt(1, CHUNK_HEIGHT) };

			zomb->inv.push_back(Items::GetItem("MEAT"));
		}
		zomb->target = nullptr;
		chunk->entities.push_back(zomb);
	}
}

std::shared_ptr<Chunk> Map::CurrentChunk() {
	if (!isUnderground) {
		return world.chunks[c_glCoords];
	}
	else {
		return underground.chunks[{c_glCoords.x, c_glCoords.y}];
	}
}



std::shared_ptr<Chunk> Map::GetProperChunk(Vector2_I coords) {
	return !isUnderground
		? world.chunks[{coords.x, coords.y}]
		: underground.chunks[{coords.x, coords.y}];
}

//std::vector<Tile> GetTileInRadius(vec2_i center) {

//}

int Map::EffectAtPos(Vector2_I coords) {
	return effectLayer.localCoords[coords.x][coords.y];
}

void Map::SetEffectInThisOrNeighbor(Vector2_I tilecoords, int effect) {

	if (tilecoords.x >= 30) {
		tilecoords.x -= 30;
	}
	else if (tilecoords.x < 0) {
		tilecoords.x += 30;
	}
	if (tilecoords.y >= 30) {
		tilecoords.y -= 30;
	}
	if (tilecoords.y < 0) {
		tilecoords.y += 30;
	}

	effectLayer.localCoords[tilecoords.x][tilecoords.y] = effect;

}

int Map::GetEffectFromThisOrNeighbor(Vector2_I tilecoords) {

	if (tilecoords.x >= 30) {
		tilecoords.x -= 30;
	}
	else if (tilecoords.x < 0) {
		tilecoords.x += 30;
	}
	if (tilecoords.y >= 30) {
		tilecoords.y -= 30;
	}
	if (tilecoords.y < 0) {
		tilecoords.y += 30;
	}

	return effectLayer.localCoords[tilecoords.x][tilecoords.y];
	
}

Tile* Map::TileAtPos(Vector2_I coords)
{
	return &CurrentChunk()->localCoords[coords.x][coords.y];
}

Tile* Map::GetTileFromThisOrNeighbor(Vector2_I tilecoords)
{
	Tile* curTile;
	vec2_i globalCoords = c_glCoords;
	if (tilecoords.x >= 30) {
		tilecoords.x -= 30;
		globalCoords.x += 1;
	}
	else if (tilecoords.x < 0) {
		tilecoords.x += 30;
		globalCoords.x -= 1;
	}
	if (tilecoords.y >= 30) {
		tilecoords.y -= 30;
		globalCoords.y += 1;
	}
	if (tilecoords.y < 0) {
		tilecoords.y += 30;
		globalCoords.y -= 1;
	}

	if (!isUnderground) {
		curTile = world.chunks[globalCoords]->GetTileAtCoords(tilecoords);
	}
	else {
		curTile = underground.chunks[globalCoords]->GetTileAtCoords(tilecoords);
	}
	return curTile;
}

biome Map::GetBiome(Vector2_I coords) 
{
	int bonusX = c_glCoords.x * CHUNK_WIDTH;
	int bonusY = c_glCoords.y * CHUNK_HEIGHT;
	float curTemp = biomeTempNoise.GetNoise((coords.x + bonusX) * 0.1, (coords.y + bonusY) * 0.1);
	float curMois = biomeMoistureNoise.GetNoise((coords.x + bonusX) * 0.1, (coords.y + bonusY) * 0.1);
	curTemp = (curTemp + 1) / 2;
	curMois = (curMois + 1) / 2;
	biome currentBiome;

	if (curTemp < tempMin) {
		if (curMois < moistureMin) { currentBiome = grassland; }
		else { currentBiome = taiga; }
	}
	else {
		if (curMois < moistureMin) { currentBiome = desert; }
		else { currentBiome = swamp; }
	}
	
	return currentBiome;
}

void Map::AttackEntity(Entity* curEnt, int damage, std::vector<std::string>* actionLog) {
	curEnt->health -= damage;

	std::string damageText = "";

	if (curEnt->health > 0) {
		damageText = (std::string)curEnt->name + " takes " + std::to_string(damage) + " damage.";
	}
	else {
		damageText = (std::string)curEnt->name + " dies.";
	}

	Vector2_I coords = curEnt->coords;

	Math::PushBackLog(actionLog, damageText);
	CurrentChunk()->localCoords[coords.x][coords.y].SetLiquid(blood);

	int bleedChance = Math::RandInt(1, 8);
	switch (bleedChance) {
	case 1:
		CurrentChunk()->localCoords[coords.x + 1][coords.y].SetLiquid(blood);
		break;
	case 2:
		CurrentChunk()->localCoords[coords.x][coords.y + 1].SetLiquid(blood);
		break;
	case 3:
		CurrentChunk()->localCoords[coords.x - 1][coords.y].SetLiquid(blood);
		break;
	case 4:
		CurrentChunk()->localCoords[coords.x][coords.y - 1].SetLiquid(blood);
		break;
	default:
		break;
	}
}

//T_Chunk& EntityLayer() { return entityLayer; }

void Map::MovePlayer(int x, int y, Player* p, std::vector<std::string>* actionLog) {
	if (x > 0 && y > 0 && x < CHUNK_WIDTH && y < CHUNK_HEIGHT)
		if (CurrentChunk()->localCoords[x][y].entity != nullptr) {
			Entity* curEnt = CurrentChunk()->localCoords[x][y].entity;
			if (curEnt->health > 0) {
				AttackEntity(curEnt, p->currentWeapon.mod, actionLog);
			}
			else {
				vec2_i newCoords = { x - p->coords.x, y - p->coords.y };
				curEnt->coords += newCoords;
				CurrentChunk()->localCoords[x][y].entity = nullptr;
				CurrentChunk()->localCoords[curEnt->coords.x][curEnt->coords.y].entity = curEnt;
				chunkUpdateFlag = true;
			}
			return;
		}

		else if (CurrentChunk()->localCoords[x][y].walkable == false) {
			Math::PushBackLog(actionLog, "You can't walk there.");
			return;
		}

	Liquid l = CurrentChunk()->GetTileAtCoords(x, y)->liquid; //check if the tile has a liquid

	if (p->coveredIn != nothing && p->coveredIn != mud && l == nothing && Math::RandInt(1, 5) >= 3) {
		CurrentChunk()->GetTileAtCoords(p->coords.x, p->coords.y)->SetLiquid(p->coveredIn);
	}

	p->coords.x = x; p->coords.y = y;

	if (l != nothing && Math::RandInt(1, 5) >= 3)  //random check
	{ 
		p->CoverIn(l, 10); //cover the player in it
	}

	CheckBounds(p);
}

//check if player moves to the next chunk
void Map::CheckBounds(Player* p) {
	bool changed = false;
	if (p->coords.x > CHUNK_WIDTH - 1) {
		ClearChunkOfEnts(CurrentChunk());
		c_glCoords.x++;
		changed = true;
		p->coords.x = 0;
	}
	else if (p->coords.x < 0) {
		ClearChunkOfEnts(CurrentChunk());
		c_glCoords.x--;
		changed = true;
		p->coords.x = CHUNK_WIDTH - 1;
	}
	else if (p->coords.y > CHUNK_HEIGHT - 1) {
		ClearChunkOfEnts(CurrentChunk());
		c_glCoords.y++;
		changed = true;
		p->coords.y = 0;
	}
	else if (p->coords.y < 0) {
		ClearChunkOfEnts(CurrentChunk());
		c_glCoords.y--;
		changed = true;
		p->coords.y = CHUNK_HEIGHT - 1;
	}

	if (changed) {
		//ClearEffects();
		UpdateMemoryZone(c_glCoords);
	}

	//if the player would go offscreen, reset them
	//Evil goto statement >:)
offscreen:
	if (p->coords.x >= CHUNK_WIDTH) { p->coords.x = CHUNK_WIDTH - 1; }
	else if (p->coords.y >= CHUNK_HEIGHT) { p->coords.y = CHUNK_HEIGHT - 1; }
	else if (p->coords.x < 0) { p->coords.x = 0; }
	else if (p->coords.y < 0) { p->coords.y = 0; }
}


//check if entity moves to next chunk
void Map::CheckBounds(Entity* p, std::shared_ptr<Chunk> chunk) {
	bool changed = false;
	int oldIndex = p->index;
	if (p->coords.x > CHUNK_WIDTH - 1) { //check if they left the chunk
		//if (c_glCoords.x + 1 >= CHUNK_WIDTH) { p->coords.x = CHUNK_WIDTH - 1; return; } //dont let them leave world bounds
		changed = true;
		p->coords.x = 0; //move them to the other side of the screen
		world.chunks[{chunk->globalChunkCoord.x + 1,
			chunk->globalChunkCoord.y}]->entities.push_back(p); //put them in the vector of the next chunk over

		//p->index = world.chunks[{chunk->globalChunkCoord.x + 1,
		//	chunk->globalChunkCoord.y}]->entities.size() - 1; //change their index in the list
	}
	else if (p->coords.x < 0) {
		if (c_glCoords.x - 1 < 0) { p->coords.x = 0; return; }
		changed = true;
		p->coords.x = CHUNK_WIDTH - 1;
		world.chunks[{chunk->globalChunkCoord.x - 1,
			chunk->globalChunkCoord.y}]->entities.push_back(p);

		//p->index = world.chunks[{chunk->globalChunkCoord.x - 1,
		//	chunk->globalChunkCoord.y}]->entities.size() - 1;
	}
	else if (p->coords.y > CHUNK_HEIGHT - 1) {
		if (chunk->globalChunkCoord.y + 1 >= CHUNK_HEIGHT) { p->coords.y = CHUNK_HEIGHT - 1; return; }
		changed = true;
		p->coords.y = 0;
		world.chunks[{chunk->globalChunkCoord.x,
			chunk->globalChunkCoord.y + 1}]->entities.push_back(p);

		//p->index = world.chunks[{chunk->globalChunkCoord.x,
		//	chunk->globalChunkCoord.y + 1}]->entities.size() - 1;
	}
	else if (p->coords.y < 0) {
		if (chunk->globalChunkCoord.y - 1 < 0) { p->coords.y = 0; return; }
		changed = true;
		p->coords.y = CHUNK_HEIGHT - 1;
		world.chunks[{chunk->globalChunkCoord.x,
			chunk->globalChunkCoord.y - 1}]->entities.push_back(p);

		//p->index = world.chunks[{chunk->globalChunkCoord.x,
		//	chunk->globalChunkCoord.y - 1}]->entities.size() - 1;
	}
	if (changed) {
		chunk->entities.erase(chunk->entities.begin() + oldIndex);
		FixEntityIndex(chunk);
	}
}

void Map::BuildChunk(std::shared_ptr<Chunk> chunk) {
	
	bool entrance = false;
	float current = 0.f;
	biome currentBiome = ocean;
	int event = Math::RandInt(0, 15);
	for (int i = 0; i < CHUNK_WIDTH; i++) {
		for (int j = 0; j < CHUNK_HEIGHT; j++) {

			int bonusX = chunk->globalChunkCoord.x * CHUNK_WIDTH;
			int bonusY = chunk->globalChunkCoord.y * CHUNK_HEIGHT;
			current = mapNoise.GetNoise((i + bonusX) * 0.1, (j + bonusY) * 0.1);

			float curTemp = biomeTempNoise.GetNoise((i + bonusX) * 0.1, (j + bonusY) * 0.1);
			float curMois = biomeMoistureNoise.GetNoise((i + bonusX) * 0.1, (j + bonusY) * 0.1);
			curTemp = (curTemp + 1) / 2;
			curMois = (curMois + 1) / 2;

			if (curTemp < tempMin) {
				if (curMois < moistureMin) { currentBiome = grassland; }
				else { currentBiome = taiga; }
			}
			else {
				if (curMois < moistureMin) { currentBiome = desert; }
				else { currentBiome = swamp; }
			}

			float currentTile = current;

			//desert biome
			switch (currentBiome) {
			case desert:
				if (Math::RandInt(0, 500) == 25 && !entrance) {
				chunk->localCoords[i][j] = Tiles::GetTile("TILE_STONE");
				entrance = true;
				chunk->localCoords[i][j].walkable = true;
				chunk->localCoords[i][j].coords = { i, j };
				continue;
				}

				if (Math::RandInt(0, 35) == 25) {
					chunk->localCoords[i][j] = Tiles::GetTile("TILE_CACTUS_BASE");
					chunk->localCoords[i][j].double_size = true;
				}

				else
				{
					chunk->localCoords[i][j] = Tiles::GetTile("TILE_SAND");
					if (Math::RandInt(1, 300) == 255) {
						chunk->localCoords[i][j].hasItem = true;
						chunk->localCoords[i][j].itemName = "SCRAP";
					}
				}
				break;
			case taiga:
				if (currentTile < -0.10f && Math::RandInt(0, 4) >= 2) {
					chunk->localCoords[i][j] = Tiles::GetTile("TILE_TREE_BASE");
					chunk->localCoords[i][j].double_size = true;
				}

				else if (currentTile < 0.f) {
					chunk->localCoords[i][j] = Tiles::GetTile("TILE_TALLGRASS");
				}

				else {
					chunk->localCoords[i][j] = Tiles::GetTile("TILE_GRASS");
					chunk->localCoords[i][j].tileColor.y += ((float)(Math::RandNum(30) - 15) / 100);
					chunk->localCoords[i][j].tileColor.z = 0.35f;
					if (Math::RandInt(1, 35) == 34) {
						chunk->localCoords[i][j].hasItem = true;
						chunk->localCoords[i][j].itemName = "STICK";
					}
					else if (Math::RandInt(1, 175) == 34) {
						chunk->localCoords[i][j].hasItem = true;
						chunk->localCoords[i][j].itemName = "BASIC_FLOWER";
					}

				}
				break;
			case grassland:
				chunk->localCoords[i][j] = Tiles::GetTile("TILE_GRASS");
				chunk->localCoords[i][j].tileColor.y += ((float)(Math::RandNum(30) - 15) / 100);
				if (currentTile < -0.10f) {
					chunk->localCoords[i][j] = Tiles::GetTile("TILE_TALLGRASS");
				}
				break;
			case swamp:
				chunk->localCoords[i][j] = Tiles::GetTile("TILE_MUD");
				if (current < -0.2f) {
					chunk->localCoords[i][j].SetLiquid(water, { 0.f, 0.5f, 0.4f });
					chunk->localCoords[i][j].liquidTime = -1;
					if (Math::RandInt(0, 25) == 5) {
						chunk->localCoords[i][j].itemName = "LILYPAD";
						chunk->localCoords[i][j].hasItem = true;
					}
				}

				else if (currentTile < 0.2f && Math::RandInt(0, 3) >= 2) {
					chunk->localCoords[i][j] = Tiles::GetTile("TILE_TREE_BASE");
					chunk->localCoords[i][j].double_size = true;
				}

				else if (Math::RandInt(0, 35) == 5) {
					if (Math::RandInt(0, 35) == 5) {
						chunk->localCoords[i][j].itemName = "GLOWING_MUSHROOM";
						chunk->localCoords[i][j].hasItem = true;
					}
					else {
						chunk->localCoords[i][j].itemName = "FLAT_MUSHROOM";
						chunk->localCoords[i][j].hasItem = true;
					}
				}
				else {
					chunk->localCoords[i][j].SetLiquid(mud);
					chunk->localCoords[i][j].liquidTime = -1;
				}

				break;
			}

			if (Math::RandInt(1, 125) >= 124 && chunk->localCoords[i][j].liquid != water) 
			{ 
				chunk->localCoords[i][j].hasItem = true;
				chunk->localCoords[i][j].itemName = "ROCK";
			}

			if (chunk->localCoords[i][j].ticksNeeded == 1) {
				chunk->localCoords[i][j].ticksNeeded = Math::RandInt(1, 10000);
			}

			chunk->localCoords[i][j].coords = { i, j };
		}
	}
	//if(Math::RandInt(0, 15) == 14) {Place}
	if (Math::RandInt(1,4) == 2) { PlaceBuilding(chunk); }
	chunk->beenBuilt = true;
}


void Map::PlaceBuilding(std::shared_ptr<Chunk> chunk) {
	Vector2_I cornerstone = { Math::RandInt(6, CHUNK_WIDTH - 6), Math::RandInt(6, CHUNK_HEIGHT - 6) };
	Vector2_I corner = cornerstone;

	//pick some corners to draw to
	int wallLength = Math::RandInt(3, 8);

	int chestCounter = 0;
	int whenDoesChestSpawn = 10;

	//draw the walls
	for (int i = 0; i < wallLength; i++)
	{
		for (int j = 0; j < wallLength; j++)
		{
			if (i == 0 && j == 1) { continue; }
			if (i == 0 || i == wallLength - 1 || j == 0 || j == wallLength - 1) {
				chunk->localCoords[corner.x + i][corner.y + j] = Tiles::GetTile("TILE_STONE");
			}
			else {
				chunk->localCoords[corner.x + i][corner.y + j] = Tiles::GetTile("TILE_STONE_FLOOR");
				if (chestCounter == whenDoesChestSpawn) {
					chunk->GetTileAtCoords(corner.x + i, corner.y + j)->itemName = "CHEST";
					chunk->GetTileAtCoords(corner.x + i, corner.y + j)->hasItem = true;
					//CreateContainer({ chunk->globalChunkCoord.x, chunk->globalChunkCoord.y, corner.x + i, corner.y + j });

					/*containers[{
						chunk->globalChunkCoord.x,
						chunk->globalChunkCoord.y,
							corner.x + i,
							corner.y + j }].AddItem(Items::GetItem("MACHETE"));*/
				}
				chestCounter++;

			}
		}
	}
}

void Map::GenerateTomb(std::shared_ptr<Chunk> chunk) {

	for (int i = 0; i < CHUNK_WIDTH; i++) {
		for (int j = 0; j < CHUNK_HEIGHT; j++) {
			if (i <= 1 || i >= CHUNK_WIDTH - 2 || j <= 1 || j >= CHUNK_HEIGHT-2) {
				chunk->localCoords[i][j] = Tiles::GetTile("TILE_STONE");
			}
			else if (Math::RandInt(0, 75) == 25) {
				chunk->localCoords[i][j] = Tiles::GetTile("TILE_CRYSTAL");
			}
			else {
				chunk->localCoords[i][j] = Tiles::GetTile("TILE_STONE_FLOOR");
			}
		}
	}

	int entx = Math::RandInt(3, 26);
	int enty = Math::RandInt(3, 26);
	Entity* zomb = new Entity{ 35, "Zombie", ID_ZOMBIE, Aggressive, true, Zombie, 10, 8, false, enty, entx };
	chunk->entities.push_back(zomb);
}

/*void Map::ClearEffects() {
	for (int i = 0; i < CHUNK_WIDTH; i++) {
		for (int j = 0; j < CHUNK_HEIGHT; j++) {
			effectLayer.[i][j] = 0;
		}
	}
}*/

std::vector<Vector2_I> Map::GetLine(Vector2_I startTile, Vector2_I endTile, int limit) {
	std::vector<Vector2_I> lineList;
	int x = startTile.x;
	int y = startTile.y;
	int w = endTile.x - x;
	int h = endTile.y - y;
	int dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
	if (w < 0) dx1 = -1; else if (w > 0) dx1 = 1;
	if (h < 0) dy1 = -1; else if (h > 0) dy1 = 1;
	if (w < 0) dx2 = -1; else if (w > 0) dx2 = 1;
	int longest = abs(w);
	int shortest = abs(h);
	if (!(longest > shortest))
	{
		longest = abs(h);
		shortest = abs(w);
		if (h < 0) dy2 = -1; else if (h > 0) dy2 = 1;
		dx2 = 0;
	}
	int numerator = longest >> 1;
	int limited = std::min(longest, limit);
	for (int i = 0; i <= limited; i++)
	{
		//if (GetTileAtPos(new Vector2(x, y)) == null) return tilesLine;

		lineList.push_back({ x, y });

		numerator += shortest;
		if (!(numerator < longest))
		{
			numerator -= longest;
			x += dx1;
			y += dy1;
		}
		else
		{
			x += dx2;
			y += dy2;
		}
	}
	return lineList;
}

//Start tile, End tile
void Map::DrawLine(std::vector<Vector2_I> list)
{
	line.insert(line.end(), list.begin(), list.end());

	for (int i = 0; i < line.size(); i++)
	{
		effectLayer.localCoords[line[i].x][line[i].y] = 15;
	}
}

void Map::ClearLine()
{
	for (int i = 0; i < line.size(); i++)
	{
		effectLayer.localCoords[line[i].x][line[i].y] = 0;
	}
	line.clear();
}

void Map::ClearEntities(std::vector<Vector2_I> positions, std::shared_ptr<Chunk> chunk)
{
	//go to the player and all entities and replace the original tile
	for (int i = 0; i < positions.size(); i++)
	{
		chunk->localCoords[positions[i].x][positions[i].y].entity = nullptr;
	}
}

void Map::ClearChunkOfEnts(std::shared_ptr<Chunk> chunk)
{
	//go to the player and all entities and replace the original tile
	for (int x = 0; x < CHUNK_WIDTH; x++)
	{
		for (int y = 0; y < CHUNK_HEIGHT; y++)
		{
			chunk->localCoords[x][y].entity = nullptr;
		}
	}
	PlaceEntities(chunk);
}

void Map::PlaceEntities(std::shared_ptr<Chunk> chunk)
{
	//place the player and all entities on top of the tiles
	for (int i = 0; i < chunk->entities.size(); i++)
	{
		chunk->localCoords
		[chunk->entities[i]->coords.x]
		[chunk->entities[i]->coords.y].entity = chunk->entities[i];
	}

}

void Map::UpdateTiles(vec2_i coords) {
	std::vector<Vector2_I> tilesToBurn;
	std::shared_ptr<Chunk> chunk = GetProperChunk(coords);

	for (int x = 0; x < CHUNK_HEIGHT; x++) {
		for (int y = 0; y < CHUNK_WIDTH; y++) {
			Tile *curTile = &chunk->localCoords[x][y];

			DoTechnical(curTile, chunk, x, y);

			//Something something flood fill
			if (!curTile->visited) { curTile->brightness = 1.f; }

			curTile->visited = false;
			//If can update over time
			if (curTile->changesOverTime) {
				curTile->ticksPassed++;

				//update if the tile can update (like grass growing)
				if (curTile->CanUpdate()) {
					if (curTile->liquid == nothing) {
						*curTile = Tiles::GetTile(curTile->timedReplacement);
					}
					else {
						curTile->ticksPassed -= 1;
					}
				}
			}
			//change grass color
			//if (curTile->id == 1) { curTile->tileColor = { 0, 0.65f + ((Math::RandNum(2) - 1) / 10), 0}; }
			
			//Crystals glow
			if (curTile->id == 14 || curTile->itemName == "GLOWING_MUSHROOM") { floodFill({x, y}, 3, true); }

			//spread fire to a list so we dont spread more than one tile per update
			if (curTile->liquid == fire) {

				if (Math::RandInt(0, 2) == 1) effectLayer.localCoords[x - 1][y] = 1;
				//If its a campfire, then let it burn but dont spread
				if (curTile->hasItem && curTile->itemName == "CAMPFIRE") {
					Container* curCont = ContainerAtCoord(curTile->coords);
					for (size_t i = 0; i < curCont->items.size(); i++)
					{
						if (!curCont->items[i].cookable) { continue; }
						//cook an item, then if its done, add the cooked item
						curCont->items[i].ticksUntilCooked -= 1;
						if (curCont->items[i].ticksUntilCooked <= 0) {
							int amount = curCont->items[i].count;
							curCont->AddItem(Items::GetItem(curCont->items[i].cooks_into), amount);
							curCont->items.erase(curCont->items.begin() + i);
							i--;
						}
					}
					floodFill({ x, y }, 5, false);
					continue;
				}

				floodFill({ x, y }, 5, false);
				curTile->burningFor++;
				switch (Math::RandInt(1, 10)) {
				case 1:
					tilesToBurn.push_back({ x + 1, y });
					break;
				case 2:
					tilesToBurn.push_back({ x, y + 1 });
					break;
				case 3:
					tilesToBurn.push_back({ x - 1, y });
					break;
				case 4:
					tilesToBurn.push_back({ x, y - 1 });
					break;
				default:
					break;
				}
			}

			//if its been burned recently, dont let it catch again
			if (curTile->burningFor >= 5) {
				curTile->burningFor++;
				if (curTile->burningFor >= 100) { curTile->burningFor = 0; }
				curTile->SetLiquid(nothing);
			}
			if (curTile->liquid != nothing && curTile->liquidTime != -1) {
				curTile->liquidTime++;
				if (curTile->liquidTime >= 100) {
					curTile->SetLiquid(nothing);
				}
			}
			//logic for upwards facing flashlight pyramid
			curTile->technical_update = false;
		}
	}
	//set fire to tiles
	for (int i = 0; i < tilesToBurn.size(); i++)
	{
		if (chunk->localCoords[tilesToBurn[i].x][tilesToBurn[i].y].burningFor > 5) { continue; }
		else if (chunk->localCoords[tilesToBurn[i].x][tilesToBurn[i].y].burningFor < 0) { continue; }
		chunk->localCoords[tilesToBurn[i].x][tilesToBurn[i].y].SetLiquid(fire);
		floodFill({ tilesToBurn[i].x, tilesToBurn[i].y }, 5, false);
	}


}

//============ Conveyor Belts ============
void Map::DoTechnical(Tile* curTile, std::shared_ptr<Chunk> chunk, int x, int y) {
	//Conveyors pass items to the next spot
	if (curTile->id >= 100 && curTile->id <= 107 && curTile->hasItem && curTile->technical_update == false) {

		//Get a vector2 of which direction to go to next
		Vector2_I nextTileCoord = { 0, 0 };
		if (curTile->technical_dir == direction::up) { nextTileCoord = { -1, 0 }; }
		else if (curTile->technical_dir == direction::down) { nextTileCoord = { 1, 0 }; }
		else if (curTile->technical_dir == direction::left) { nextTileCoord = { 0, -1 }; }
		else if (curTile->technical_dir == direction::right) { nextTileCoord = { 0, 1 }; }

		Tile* nextTile = &chunk->localCoords[x + nextTileCoord.x][y + nextTileCoord.y];

		//move the item
		curTile->hasItem = false;
		nextTile->hasItem = true;
		nextTile->itemName = curTile->itemName;
		nextTile->technical_update = true;
		
		switch (curTile->id) {
			case 100: //up
				curTile->technical_dir = direction::up;
				//if it gets pushed onto an up left turn
				if (nextTile->id == 104) { nextTile->technical_dir = direction::right; }
				//if it gets pushed onto an up right turn
				if (nextTile->id == 105) { nextTile->technical_dir = direction::left; }
				break;
			case 101: //down
				curTile->technical_dir = direction::down;
				//if it gets pushed onto a down left turn
				if (nextTile->id == 106) { nextTile->technical_dir = direction::right; }
				//if it gets pushed onto a down right turn
				if (nextTile->id == 107) { nextTile->technical_dir = direction::left; }
				break;
			case 102: //left
				curTile->technical_dir = direction::left;
				if (nextTile->id == 106) { nextTile->technical_dir = direction::up; }
				if (nextTile->id == 104) { nextTile->technical_dir = direction::down; }
				break;
			case 103: //right
				curTile->technical_dir = direction::right;
				if (nextTile->id == 107) { nextTile->technical_dir = direction::up; }
				if (nextTile->id == 105) { nextTile->technical_dir = direction::down; }
				break;
		}
	}
}

//Reset the indices of the entities
void Map::FixEntityIndex(std::shared_ptr<Chunk> chunk) {
	for (int i = 0; i < chunk->entities.size(); i++)
	{
		chunk->entities[i]->index = i;
	}
}

void Map::CreateContainer(Vector2_I coordsLocal) {
	containers[{ c_glCoords.x, c_glCoords.y, coordsLocal.x, coordsLocal.y }] =
	{ { c_glCoords.x, c_glCoords.y}, {coordsLocal.x, coordsLocal.y }, {} };
}

void Map::RemoveContainer(Vector2_I coordsLocal) {
	containers.erase({ c_glCoords.x, c_glCoords.y, coordsLocal.x, coordsLocal.y });
}

void Map::CreateContainer(Vector4_I coords) {
	containers[{ coords.x, coords.y, coords.z, coords.w }] =
	{ { coords.x, coords.y}, {coords.z, coords.w }, {}};
}

Container* Map::ContainerAtCoord(Vector2_I localCoords) {
	if (containers.count({ c_glCoords.x, c_glCoords.y, localCoords.x, localCoords.y }) != 0) {
		return &containers[{ c_glCoords.x, c_glCoords.y, localCoords.x, localCoords.y }];
	}
	return nullptr;
}

// Recursive helper function for flood fill algorithm
void Map::floodFillUtil(int x, int y, float prevBrightness, float newBrightness, int max, bool twinkle, bool firstTile)
{
	// Check if current tile is within the image boundaries
	if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT) {
		return;
	}

	// Check if current tile has already been visited
	if (CurrentChunk()->localCoords[x][y].visited) {
		return;
	}

	// Calculate new brightness value for the current tile
	float currentBrightness = std::max(prevBrightness + newBrightness, 0.1f);

	if (!CurrentChunk()->localCoords[x][y].walkable && !firstTile) {
		CurrentChunk()->localCoords[x][y].brightness = std::min(currentBrightness, 1.f);
		return;
	}

	if (max <= 0) {
		return;
	}

	float extraFlare = 0;
	// Update the current tile's brightness and visited flag
	if (twinkle) { extraFlare = -((float)Math::RandInt(3, 5) / 100); }
	CurrentChunk()->localCoords[x][y].brightness = (std::min(currentBrightness, 1.f) + extraFlare);
	CurrentChunk()->localCoords[x][y].visited = true;

	// Recursively call floodFillUtil for the four adjacent tiles
	floodFillUtil(x, y + 1, currentBrightness, newBrightness, max - 1, twinkle, false);
	floodFillUtil(x, y - 1, currentBrightness, newBrightness, max - 1, twinkle, false);
	floodFillUtil(x + 1, y, currentBrightness, newBrightness, max - 1, twinkle, false);
	floodFillUtil(x - 1, y, currentBrightness, newBrightness, max - 1, twinkle, false);
}

// Flood fill algorithm for 2D array of tiles with brightness value
void Map::floodFill(Vector2_I centerTile, int distance, bool twinkle)
{
	int prevBrightness = 0.1f;
	floodFillUtil(centerTile.x, centerTile.y, prevBrightness, 0.1f, distance, twinkle, true);
}

void Map::ResetLightValues() {
	//go to the player and all entities and replace the original tile
	for (int x = 0; x < CHUNK_WIDTH; x++)
	{
		for (int y = 0; y < CHUNK_HEIGHT; y++)
		{
			CurrentChunk()->localCoords[x][y].brightness = 1.f;
			CurrentChunk()->localCoords[x][y].visited = false;
		}
	}
}


Map::~Map()
{
	/*for (int x = 0; x < MAP_WIDTH; x++)
	{
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			for (int i = 0; i < world.chunks[x][y].entities.size(); i++)
			{
				delete(&world.chunks[x][y].entities[i]);
			}
		}
	}*/
}