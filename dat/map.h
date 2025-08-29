#pragma once
#include "antibox/tools/noise.h"
#include "fastnoiselite/FastNoiseLite.h"
#include "antibox/core/antibox.h"
#include "antibox/managers/factory.h"

#include "items.h"
#include "nameGenerate.cpp"
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


enum weather {clear, rainy, thunder};
class Map {
public:
	bool chunkUpdateFlag = false;
	Vector2_I c_glCoords{ 250, 250 };
	T_Chunk effectLayer; //is the visual effects (lines, fire, smoke, etc)
	World world; //keeps the original generated map so that tiles walked over wont be erased
	World underground; //map but underground, different map entirely
	bool isUnderground;
	std::vector<Vector2_I> line;
	weather currentWeather;
	int ticksUntilWeatherUpdate = 0;
	std::string currentSaveName;

	int landSeed = 0, biomeSeed = 0;
	float tempMin = 0.5f, moistureMin = 0.5f;
	FastNoiseLite mapNoise, biomeTempNoise, biomeMoistureNoise;
	
	
	
	void CreateMap(int seed, int b_seed);
	bool DoesChunkExistsOrMakeNew(Vector2_I coords);
	void UpdateMemoryZone(Vector2_I coords);
	void UnloadChunks(std::vector<Vector2_I> chunksToDelete);
	void ReadChunk(Vector2_I curChunk, std::string path);
	void SpawnChunkEntities(std::shared_ptr<Chunk> chunk);
	std::shared_ptr<Chunk> CurrentChunk();
	Tile* TileAtPos(Vector2_I coords);
	Tile* GetTileFromThisOrNeighbor(Vector2_I tilecoords, Vector2_I global_coords);
	Tile* GetTileFromThisOrNeighbor(Vector2_I tilecoords);
	Vector2_I GetNeighborChunkCoords(Vector2_I tilecoords, Vector2_I global_coords);
	int EffectAtPos(Vector2_I coords);
	int GetEffectFromThisOrNeighbor(Vector2_I tilecoords);
	void SetEffectInThisOrNeighbor(Vector2_I tilecoords, int effect);
	biome GetBiome(Vector2_I coords);
	biome GetBiome(Vector2_I coords, Vector2_I global_coords);
	void MakeNewChunk(Vector2_I coords);
	void AttackEntity(Entity* curEnt, int damage, std::vector<std::string>* actionLog, std::shared_ptr<Chunk> chunk);
	void MovePlayer(int x, int y, Player* p, std::vector<std::string>* actionLog, Inventory& pInv);
	void CheckBounds(Player* p);
	bool CheckBounds(Entity* p, std::shared_ptr<Chunk> chunk);
	void EmptyChunk(std::shared_ptr<Chunk> chunk);
	void BuildChunk(std::shared_ptr<Chunk> chunk);
	void PlaceBuilding(Vector2_I startingChunk);
	Vector2_I PlaceStartingBuilding();
	void PickStructure(Vector2_I startingChunk);
	void PlaceCampsite(Vector2_I startingChunk);
	void PlaceStructure(Vector2_I startingChunk, std::string structure, Vector2_I dimensions);
	void GenerateTomb(std::shared_ptr<Chunk> chunk);
	std::vector<Vector2_I> GetLine(Vector2_I startTile, Vector2_I endTile, int limit);
	std::vector<Vector2_I> GetSquare(Vector2_I centerTile, int size);
	std::vector<Vector2_I> GetSquareEdge(Vector2_I centerTile, int size);
	std::vector<Vector2_I> GetCircle(Vector2_I centerPoint, int size);
	std::vector<Vector2_I> GetCircleEdge(Vector2_I centerPoint, int size);
	void DrawLine(std::vector<Vector2_I> list);
	void ClearLine();
	void ClearEntities(std::shared_ptr<Chunk> chunk);
	void ClearChunkOfEnts(std::shared_ptr<Chunk> chunk);
	void ClearEffects();
	void PlaceEntities(std::shared_ptr<Chunk> chunk);
	void UpdateTiles(vec2_i coords, Player* p);
	void DoTechnical(Tile* curTile, std::shared_ptr<Chunk> chunk, int x, int y);
	void FixEntityIndex(std::shared_ptr<Chunk> chunk);
	void floodFillUtil(int x, int y, float prevBrightness, float newBrightness, int max, bool twinkle, bool firstTile);
	void floodFill(Vector2_I centerTile, int distance, bool twinkle);
	void ResetLightValues();
	bool UpdateWeather();
	void SetWeather(weather we);
	void SetupNoise(int l_seed, int b_seed);
	std::shared_ptr<Chunk> GetProperChunk(Vector2_I coords);
	void TempCheck(Player* p, Vector2_I coords);
	std::string GetCurrentSavePath() const { return "dat/saves/" + currentSaveName + "/"; }

	Entity* SpawnHuman(Vector2_I spawnCoords, Behaviour b, Faction f);

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

static void T_SaveChunks(std::shared_ptr<Chunk> chunk, std::string currentSaveName) {
	chunk->SaveChunk(currentSaveName);
}

bool Map::DoesChunkExistsOrMakeNew(Vector2_I coords) {
	std::string filename = "dat/saves/";
	filename += currentSaveName;
	filename += "/map/";
	filename += std::to_string(coords.x);
	filename += std::to_string(coords.y);
	filename += ".chunk";

	//Check if theres a saved chunk. if so, load it
	if (world.chunks[coords] == nullptr) {
		if (fileExists(filename.c_str())) {
			ReadChunk(coords, filename);
			return true;
		}
		else {
			MakeNewChunk(coords);
			return false;
		}
	}
	return true;
}

//Change which chunks are in memory at one time
void Map::UpdateMemoryZone(Vector2_I coords) {

	//add new chunks in our area around player
	for (int y = -1; y <= 1; y++)
	{
		for (int x = -1; x <= 1; x++)
		{
			vec2_i curChunk = { c_glCoords.x + x, c_glCoords.y + y };
			
			DoesChunkExistsOrMakeNew(curChunk);
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

	UnloadChunks(chunksToDelete);
	
}

void Map::UnloadChunks(std::vector<Vector2_I> chunksToDelete) {
	if (chunksToDelete.size() > 0) {
		//Save all three chunks to file across 3 threads
		std::vector<std::thread> threads;
		for (int i = 0; i < chunksToDelete.size(); ++i) {
			auto it = world.chunks.find(chunksToDelete[i]);
			if (it != world.chunks.end()) {
				threads.emplace_back(T_SaveChunks, it->second, currentSaveName);
			}
		}

		for (auto& th : threads) {
			th.join();
		}
	}

	//remove them
	for (auto& vec : chunksToDelete) {
		//world.chunks[vec]->SaveChunk();
		world.chunks.erase(vec);

		auto it = world.chunks.find(vec);
		if (it != world.chunks.end()) {
			std::cout << "Use count: " << it->second.use_count() << "\n";
		}
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
		Audio::SetVolumeLoop(Audio::GetVolume() / 2, "rain");
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
	tempChunk->LoadChunk(curChunk, currentSaveName);
	SpawnChunkEntities(tempChunk);
	world.chunks[curChunk] = tempChunk;
}


void Map::MakeNewChunk(Vector2_I coords) {
	std::shared_ptr<Chunk> tempChunk = std::make_shared<Chunk>();
	tempChunk->globalChunkCoord = coords;

	//build land
	BuildChunk(tempChunk);

	//add to world map
	world.chunks[tempChunk->globalChunkCoord] = tempChunk;

	//place old roads
	if (Math::RandInt(0, 2) == 1) {
		OpenedData dat;
		ItemReader::GetDataFromFile("structures/oldroads.eid", "NAMES", &dat);

		int leng = dat.getArray("names").size();
		OpenedData roadDat;
		ItemReader::GetDataFromFile("structures/oldroads.eid", dat.getArray("names")[Math::RandInt(0,leng-1)], &roadDat);
		PlaceStructure(coords, roadDat.getString("tiles"), { roadDat.getInt("width"), roadDat.getInt("height") });
	}

	//place structures
	if (Math::RandInt(1, 4) == 2) { PickStructure(coords); }



	//place entities
	SpawnChunkEntities(tempChunk);


}

Entity* Map::SpawnHuman(Vector2_I spawnCoords, Behaviour b, Faction f) {
	Entity* zomb = new Entity{ 35, "Human", ID_HUMAN, b, false, f, 10, 10, true, spawnCoords.x, spawnCoords.y, true };

	//Random chance to spawn previous npc
	if (Math::RandInt(0, 5) == 2) {
		std::string specialEntFilePath = (
			"dat/saves/"
			+ currentSaveName
			+ "/entities/");
		if (fileExists(specialEntFilePath.c_str())) {
			//read the list in
			OpenedData specialEnts;
			ItemReader::GetDataFromFile(specialEntFilePath + "names.eid", "NAMES", &specialEnts, false);

			//if theres any
			if (specialEnts.getArray("names").size() > 0) {
				//choose one from the list
				std::string nameChosen = specialEnts.getArray("names")[Math::RandInt(0, specialEnts.getArray("names").size() - 1)];

				//load all their data
				OpenedData entData;
				ItemReader::GetDataFromFile(specialEntFilePath + nameChosen + ".eid", nameChosen, &entData, false);
				if (entData.getInt("health") > 0) {
					zomb->name = nameChosen.c_str();
					zomb->health = entData.getInt("health");
					zomb->b = (Behaviour)entData.getInt("behaviour");
					zomb->faction = (Faction)entData.getInt("faction");
					zomb->damage = entData.getInt("damage");
					zomb->feelingTowardsPlayer.anger = entData.getFloat("anger");
					zomb->feelingTowardsPlayer.fear = entData.getFloat("fear");
					zomb->feelingTowardsPlayer.trust = entData.getFloat("trust");
					zomb->feelingTowardsPlayer.happy = entData.getFloat("happy");
				}
			}
		}
	}

	zomb->inv.push_back(Items::GetItem("BITS"));
	zomb->target = nullptr;
	if (Math::RandInt(0, 2) == 1) zomb->GenerateTrades();

	return zomb;
}

void Map::SpawnChunkEntities(std::shared_ptr<Chunk> chunk)
{
	for (int i = 0; i < Math::RandInt(0, 4); i++) //CHANGE THIS TO SPAWN ENTITIES
	{
		Entity* zomb;
		Vector2_I spawnCoords = { Math::RandInt(1, CHUNK_WIDTH), Math::RandInt(1, CHUNK_HEIGHT) };
		int num = Math::RandInt(1, 20);
		if (num >= 20) {
			zomb = new Entity{ 15, "Finder", ID_FINDER, Protective, true, Takers, 3, 25, true, spawnCoords.x, spawnCoords.y};

			zomb->inv.push_back(Items::GetItem("CRYSTAL"));
			zomb->inv.push_back(Items::GetItem("SCRAP"));
			zomb->inv.push_back(Items::GetItem("GUTS"));
		}
		else if (num >= 15) {
			//humans have a lot more logic than the rest
			chunk->entities.push_back(SpawnHuman(spawnCoords, Protective, Human_W));
			continue;
		}
		else if (num >= 10) {
			zomb = new Entity{ 15, "Zombie", ID_ZOMBIE, Aggressive, true, Zombie, 7, 8, false, spawnCoords.x, spawnCoords.y };

			zomb->inv.push_back(Items::GetItem("OLD_CLOTH"));
			zomb->inv.push_back(Items::GetItem("GUTS"));
		}
		
		else {
			switch (GetBiome(spawnCoords, chunk->globalChunkCoord)) {
			case swamp:
				zomb = new Entity{ 10, "Frog", ID_FROG, Wander, false, Wildlife, 5, 1, false, spawnCoords.x, spawnCoords.y };
				zomb->inv.push_back(Items::GetItem("LEATHER"));
				break;
			case taiga:
				zomb = new Entity{ 10, "Chicken", ID_CHICKEN, Wander, false, Wildlife, 5, 1, false, spawnCoords.x, spawnCoords.y };
				break;
			case grassland:
				zomb = new Entity{ 40, "Bull", ID_COW, Aggressive, false, Wildlife, 5, 15, false, spawnCoords.x, spawnCoords.y };
				zomb->inv.push_back(Items::GetItem("LEATHER"));
				zomb->inv.push_back(Items::GetItem("BULL_HORN"));
				break;
			case desert:
				zomb = new Entity{ 10, "Cat", ID_CAT, Wander, false, Wildlife, 5, 1, false, spawnCoords.x, spawnCoords.y };
				break;
			default:
				zomb = new Entity{ 10, "Chicken", ID_CHICKEN, Wander, false, Wildlife, 5, 1, false, spawnCoords.x, spawnCoords.y };
				break;
			}

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

Tile* Map::GetTileFromThisOrNeighbor(Vector2_I tilecoords, Vector2_I global_coords) {
	Tile* curTile;
	vec2_i globalCoords = global_coords;
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
	else if (tilecoords.y < 0) {
		tilecoords.y += 30;
		globalCoords.y -= 1;
	}

	if (world.chunks.contains(globalCoords)) {
		if (!isUnderground) {
			curTile = world.chunks[globalCoords]->GetTileAtCoords(tilecoords);
		}
		else {
			curTile = underground.chunks[globalCoords]->GetTileAtCoords(tilecoords);
		}
		return curTile;
	}
	else {
		return nullptr;
	}
}

Tile* Map::GetTileFromThisOrNeighbor(Vector2_I tilecoords)
{
	return GetTileFromThisOrNeighbor(tilecoords, c_glCoords);
}

Vector2_I Map::GetNeighborChunkCoords(Vector2_I coords, Vector2_I global_coords) {
	vec2_i globalCoords = global_coords;
	if (coords.x >= 30) {
		globalCoords.x += 1;
	}
	else if (coords.x < 0) {
		globalCoords.x -= 1;
	}
	if (coords.y >= 30) {
		globalCoords.y += 1;
	}
	else if (coords.y < 0) {
		globalCoords.y -= 1;
	}
	return globalCoords;
}


biome Map::GetBiome(Vector2_I coords, Vector2_I global_coords)
{
	int bonusX = global_coords.x * CHUNK_WIDTH;
	int bonusY = global_coords.y * CHUNK_HEIGHT;
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

biome Map::GetBiome(Vector2_I coords)
{
	return GetBiome(coords, c_glCoords);
}

void Map::AttackEntity(Entity* curEnt, int damage, std::vector<std::string>* actionLog, std::shared_ptr<Chunk> chunk) {
	curEnt->health -= damage;
	Audio::Play("dat/sounds/hit_alive.mp3");

	std::string damageText = "";

	if (curEnt->health > 0) {
		damageText = (std::string)curEnt->name + " takes " + std::to_string(damage) + " damage.";
	}
	else {
		damageText = (std::string)curEnt->name + " dies.";
	}

	Vector2_I coords = curEnt->coords;

	Math::PushBackLog(actionLog, damageText);
	chunk->localCoords[coords.x][coords.y].SetLiquid(blood);

	int bleedChance = Math::RandInt(1, 8);
	switch (bleedChance) {
	case 1:
		chunk->localCoords[coords.x + 1][coords.y].SetLiquid(blood);
		break;
	case 2:
		chunk->localCoords[coords.x][coords.y + 1].SetLiquid(blood);
		break;
	case 3:
		chunk->localCoords[coords.x - 1][coords.y].SetLiquid(blood);
		break;
	case 4:
		chunk->localCoords[coords.x][coords.y - 1].SetLiquid(blood);
		break;
	default:
		break;
	}
}

//T_Chunk& EntityLayer() { return entityLayer; }

void Map::MovePlayer(int x, int y, Player* p, std::vector<std::string>* actionLog, Inventory& pInv) {
	if (x > 0 && y > 0 && x < CHUNK_WIDTH && y < CHUNK_HEIGHT)
		if (GetTileFromThisOrNeighbor({ x,y })->entity != nullptr) {
			Entity* curEnt = GetTileFromThisOrNeighbor({ x,y })->entity;
			//check if they have a weapon equipped
			if (curEnt->health > 0 && pInv.CurrentEquipExists(weapon)){
				//attack entity
				AttackEntity(curEnt, pInv.equippedItems[weapon].mod, actionLog, CurrentChunk());
				if (curEnt->b == Protective || curEnt->b == Protective_Stationary) { curEnt->aggressive = true; curEnt->b = Aggressive; }

				//drop their reputation with player
				if (curEnt->entityID == ID_HUMAN) { curEnt->AddMemory(MemoryType::Attacked, ID_PLAYER, { 0.f,0.f,0.f,0.5f }); }

				//if it has durability
				if (pInv.equippedItems[weapon].maxDurability != -1) {
					//remove durability
					pInv.equippedItems[weapon].durability -= 1.f;
					//if its at 0, remove it
					if (pInv.equippedItems[weapon].durability <= 0) {
						std::string itemName = pInv.equippedItems[weapon].section;
						pInv.Unequip(weapon);
						pInv.RemoveItem(itemName);
					}
				}
			}
			else {
				vec2_i newCoords = { x - p->coords.x, y - p->coords.y };
				curEnt->coords += newCoords;
				GetTileFromThisOrNeighbor({ x,y })->entity = nullptr;
				GetTileFromThisOrNeighbor(curEnt->coords)->entity = curEnt;
				CheckBounds(curEnt, CurrentChunk());
				chunkUpdateFlag = true;
			}
			return;
		}

		else if (GetTileFromThisOrNeighbor({ x,y })->walkable == false) {
			//they cant walk on non walkable surfaces or deep water
			Math::PushBackLog(actionLog, "You can't walk there.");
			return;
		}

	Liquid l = GetTileFromThisOrNeighbor({ x,y })->liquid; //check if the tile has a liquid
	if (l == water && GetTileFromThisOrNeighbor({ x,y })->liquidTime == -1) {
		Audio::Play("dat/sounds/movement/enter_water.wav");
	}

	//if the player is covered in liquid, spread it
	if (p->coveredIn != nothing && p->coveredIn != mud && l == nothing && Math::RandInt(1, 5) >= 3) {
		GetTileFromThisOrNeighbor(p->coords)->SetLiquid(p->coveredIn);
	}

	p->coords.x = x; p->coords.y = y;

	if (l != nothing && Math::RandInt(1, 5) >= 3)  //random check
	{ 
		if (!pInv.Waterproof(boots)) {
			p->CoverIn(l, 10); //cover the player in it
		}
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
bool Map::CheckBounds(Entity* p, std::shared_ptr<Chunk> chunk) {
	bool changed = false;
	int oldIndex = p->index;
	if (p->coords.x > CHUNK_WIDTH - 1) { //check if they left the chunk
		if (c_glCoords.x + 1 >= 500) { p->coords.x = CHUNK_WIDTH - 1; return false; } //dont let them leave world bounds
		

		//if they attempt to enter a chunk that isnt loaded, then delete them.
		if(!(chunk->globalChunkCoord.x + 1 >= c_glCoords.x + 2)) {
			changed = true;
			p->coords.x = 0; //move them to the other side of the screen
			world.chunks[{chunk->globalChunkCoord.x + 1,
				chunk->globalChunkCoord.y}]->entities.push_back(p); //put them in the vector of the next chunk over
		}
		else { //prevent the error of them being on the outer bounds and not exiting the chunk,
			   // leaving them at an x or y value of 30 which is invalid bcus the chunks are 0-29
			p->coords.x = CHUNK_WIDTH - 1;
		}

	}
	else if (p->coords.x < 0) {
		if (c_glCoords.x - 1 < 0) { p->coords.x = 0; return false; }

		//if they attempt to enter a chunk that isnt loaded, then dont let them.
		if (!(chunk->globalChunkCoord.x - 1 <= c_glCoords.x - 2)) {
			changed = true;
			p->coords.x = CHUNK_WIDTH - 1;
			world.chunks[{chunk->globalChunkCoord.x - 1,
				chunk->globalChunkCoord.y}]->entities.push_back(p);
		}
		else {
			p->coords.x = 0;
		}
	}
	else if (p->coords.y > CHUNK_HEIGHT - 1) {
		if (chunk->globalChunkCoord.y + 1 >= 500) { p->coords.y = CHUNK_HEIGHT - 1; return false; }

		//if they attempt to enter a chunk that isnt loaded, then delete them.
		if (!(chunk->globalChunkCoord.y + 1 >= c_glCoords.y + 2)) {
			changed = true;
			p->coords.y = 0;
			world.chunks[{chunk->globalChunkCoord.x,
				chunk->globalChunkCoord.y + 1}]->entities.push_back(p);
		}
		else {
			p->coords.y = CHUNK_HEIGHT - 1;
		}
	}
	else if (p->coords.y < 0) {
		if (chunk->globalChunkCoord.y - 1 < 0) { p->coords.y = 0; return false; }

		//if they attempt to enter a chunk that isnt loaded, then delete them.
		if (!(chunk->globalChunkCoord.y - 1 <= c_glCoords.y - 2)) {
			changed = true;
			p->coords.y = CHUNK_HEIGHT - 1;
			world.chunks[{chunk->globalChunkCoord.x,
				chunk->globalChunkCoord.y - 1}]->entities.push_back(p);
		}
		else {
			p->coords.y = 0;
		}
	}

	
	if (changed) {
		int indexOfEnt = -1;

		for (int i = 0; i < chunk->entities.size(); i++)
		{
			if (chunk->entities[i] == p) {
				indexOfEnt = i;
				break;
			}
		}
		chunk->entities.erase(chunk->entities.begin() + indexOfEnt);
		FixEntityIndex(chunk);
		return true;
	}
	return false;
}

void Map::EmptyChunk(std::shared_ptr<Chunk> chunk) {

	for (int i = 0; i < CHUNK_WIDTH; i++) {
		for (int j = 0; j < CHUNK_HEIGHT; j++) {
			chunk->localCoords[i][j] = Tiles::GetTile("TILE_SAND");
		}
	}
}

void Map::BuildChunk(std::shared_ptr<Chunk> chunk) {
	
	bool entrance = false;
	float current = 0.f;
	biome currentBiome = ocean;
	int event = Math::RandInt(0, 15);
	float taiga_pond_height = Math::RandInt(80,99);
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
				chunk->localCoords[i][j] = Tiles::GetTile("TILE_CAVE_ENTRANCE");
				entrance = true;
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
						chunk->localCoords[i][j].itemName = "GLASS_SHARDS";
					}
				}
				break;
			case taiga:
				chunk->localCoords[i][j] = Tiles::GetTile("TILE_GRASS");
				if (currentTile < -(taiga_pond_height / 100)) {
					chunk->localCoords[i][j].biomeID = (short)currentBiome;
					chunk->localCoords[i][j].SetLiquid(water, true);
					chunk->localCoords[i][j].liquidTime = -1;
				}
				else if (currentTile < -0.10f && Math::RandInt(0, 4) >= 2) {
					chunk->localCoords[i][j] = Tiles::GetTile("TILE_TREE_BASE");
					chunk->localCoords[i][j].double_size = true;
				}

				else if (currentTile < 0.f) {
					chunk->localCoords[i][j] = Tiles::GetTile("TILE_TALLGRASS");
				}

				else {
					chunk->localCoords[i][j].mainTileColor.y += ((float)(Math::RandNum(30) - 15) / 100);
					chunk->localCoords[i][j].mainTileColor.z = 0.35f;
					chunk->localCoords[i][j].ResetColor();
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
				chunk->localCoords[i][j].mainTileColor.y += ((float)(Math::RandNum(30) - 15) / 100);
				if (currentTile < -0.10f) {
					chunk->localCoords[i][j] = Tiles::GetTile("TILE_TALLGRASS");
				}
				break;
			case swamp:
				chunk->localCoords[i][j] = Tiles::GetTile("TILE_GRASS");
				chunk->localCoords[i][j].mainTileColor = {0.5, 0.55, 0.35};
				chunk->localCoords[i][j].mainTileColor.y += ((float)(Math::RandNum(30) - 15) / 100);
				chunk->localCoords[i][j].ResetColor();
				if (current < -0.3f) {
					chunk->localCoords[i][j].biomeID = (short)currentBiome;
					chunk->localCoords[i][j].SetLiquid(water, true);
					chunk->localCoords[i][j].liquidTime = -1;
					if (Math::RandInt(0, 25) == 5) {
						chunk->localCoords[i][j].itemName = "LILYPAD";
						chunk->localCoords[i][j].hasItem = true;
					}
				}

				else if (currentTile < 0.1f && Math::RandInt(0, 2) == 1) {
					if (Math::RandInt(0, 5) == 2) {
						chunk->localCoords[i][j] = Tiles::GetTile("TILE_TREE_BASE");
					}
					else {
						chunk->localCoords[i][j] = Tiles::GetTile("TILE_CATTAIL");
					}
					chunk->localCoords[i][j].double_size = true;
				}
				else if (currentTile < 0.3f) {
					chunk->localCoords[i][j] = Tiles::GetTile("TILE_MUD");
					chunk->localCoords[i][j].SetLiquid(mud);
					chunk->localCoords[i][j].liquidTime = -1;
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

				break;
			}

			if (Math::RandInt(1, 125) >= 124 && chunk->localCoords[i][j].liquid != water) 
			{ 
				chunk->localCoords[i][j].hasItem = true;
				chunk->localCoords[i][j].itemName = "ROCK";
			}
			else if (Math::RandInt(1, 350) == 124 && !chunk->localCoords[i][j].hasItem)
			{
				chunk->localCoords[i][j].hasItem = true;
				chunk->localCoords[i][j].itemName = "SCRAP";
			}

			if (chunk->localCoords[i][j].ticksNeeded == 1) {
				chunk->localCoords[i][j].ticksNeeded = Math::RandInt(1, 10000);
			}

			chunk->localCoords[i][j].coords = { i, j };
			chunk->localCoords[i][j].biomeID = (short)currentBiome;
		}
	}
	//if(Math::RandInt(0, 15) == 14) {Place}
	chunk->beenBuilt = true;
}


void Map::PickStructure(Vector2_I startingChunk) {
	int randInt = Math::RandInt(0, 4);

	//chance for rare structure
	if (Math::RandInt(0, 35) == 2) {
		OpenedData dat;

		switch (randInt) {
		case 0:
		case 1:
			ItemReader::GetDataFromFile("structures/structs.eid", "MONKEY_BAR", &dat);
			PlaceStructure(startingChunk, dat.getString("tiles"), { dat.getInt("width"),dat.getInt("height") });
			break;
		case 2:
		case 3:
		case 4:
			ItemReader::GetDataFromFile("structures/structs.eid", "RADIO_SHACK", &dat);
			PlaceStructure(startingChunk, dat.getString("tiles"), { dat.getInt("width"),dat.getInt("height") });
			break;
		}
	}
	//otherwise set up camp or a random house
	else {
		switch (randInt) {
		case 0:
		case 1:
		case 2:
			PlaceBuilding(startingChunk);
			break;
		case 3:
		case 4:
			PlaceCampsite(startingChunk);
			break;
		}
	}

	
	
}

void Map::PlaceCampsite(Vector2_I startingChunk) {
	Vector2_I cornerstone = { Math::RandInt(0, CHUNK_WIDTH - 5), Math::RandInt(0, CHUNK_HEIGHT - 5) };
	std::vector<Vector2_I> buildingBlocks = GetSquare(cornerstone, 4);


	Vector2_I randomCoords1 = {
		buildingBlocks[Math::RandInt(0, buildingBlocks.size() - 1)].x,
		buildingBlocks[Math::RandInt(0, buildingBlocks.size() - 1)].y };

	world.chunks[startingChunk]->entities.push_back(SpawnHuman(randomCoords1, Protective_Stationary, Human_W));

	if (Math::RandInt(0, 10) == 5) {
		Vector2_I randomCoords2 = {
			buildingBlocks[Math::RandInt(0, buildingBlocks.size() - 1)].x,
			buildingBlocks[Math::RandInt(0, buildingBlocks.size() - 1)].y };

		world.chunks[startingChunk]->entities.push_back(SpawnHuman(randomCoords2, Protective_Stationary, Human_W));
	}



	//GetTileFromThisOrNeighbor(buildingBlocks[Math::RandInt(0, buildingBlocks.size() - 1)], startingChunk)->hasItem = true;
	//GetTileFromThisOrNeighbor(buildingBlocks[Math::RandInt(0, buildingBlocks.size() - 1)], startingChunk)->itemName = "TABLE";
	GetTileFromThisOrNeighbor(cornerstone, startingChunk)->hasItem = true;
	GetTileFromThisOrNeighbor(cornerstone, startingChunk)->itemName = "CAMPFIRE";
	GetTileFromThisOrNeighbor(cornerstone, startingChunk)->SetLiquid(fire);

}

void Map::PlaceStructure(Vector2_I startingChunk, std::string structure, Vector2_I dimensions) {
	Vector2_I cornerstone = { Math::RandInt(0, CHUNK_WIDTH - 5), Math::RandInt(0, CHUNK_HEIGHT - 5) };

	bool newChunk = false;
	std::vector<Vector2_I> chunksCoordsToDelete;


	for (int i = 0; i < dimensions.y; i++)
	{
		for (int x = 0; x < dimensions.x; x++)
		{
			Vector2_I curCoordsModified = { i + cornerstone.y,x + cornerstone.x };
			Tile* curTile = GetTileFromThisOrNeighbor(curCoordsModified, startingChunk);
			if (curTile == nullptr) {
				//continue;
				Vector2_I chunk_Coords = GetNeighborChunkCoords(curCoordsModified, startingChunk);
				if (!DoesChunkExistsOrMakeNew(chunk_Coords)) {
					newChunk = true;
					curTile = GetTileFromThisOrNeighbor(curCoordsModified, startingChunk);
					chunksCoordsToDelete.push_back(chunk_Coords);
				}
			}
			//if somehow its still null, just skip (it keeps crashing here)
			if (curTile == nullptr) {
				continue;
			}
			//								  --convert 1d coords into 2d--
			switch (structure[(x + (i * dimensions.x))]) {
			case '?':
				break;
			case 'w':
				*curTile = Tiles::GetTile("TILE_STONE");
				break;
			case 'f':
				*curTile = Tiles::GetTile("TILE_STONE_FLOOR");
				break; 
			case 'c':
				*curTile = Tiles::GetTile("TILE_CHAIN_FENCE");
				break;
			case '-':
				*curTile = Tiles::GetTile("TILE_STONE_FLOOR");
				curTile->hasItem = true;
				curTile->itemName = "TABLE";
				break;
			case '+':
				*curTile = Tiles::GetTile("TILE_STONE_FLOOR");

				world.chunks[startingChunk]->entities.push_back(SpawnHuman(curCoordsModified, Protective, Human_W));
				break;
			}
		}
	}

	if (newChunk) {
		UnloadChunks(chunksCoordsToDelete);
	}
}

Vector2_I Map::PlaceStartingBuilding() {
	Vector2_I cornerstone = { CHUNK_WIDTH / 2, CHUNK_HEIGHT / 2 };

	std::vector<Vector2_I> buildingBlocks = GetSquare(cornerstone, 5);
	std::vector<Vector2_I> wallBlocks = GetSquareEdge(cornerstone, 5);

	//draw the floors
	for (int i = 0; i < buildingBlocks.size(); i++)
	{

		Tile* curTile = GetTileFromThisOrNeighbor({ buildingBlocks[i].x,buildingBlocks[i].y }, c_glCoords);

		*curTile = Tiles::GetTile("TILE_STONE_FLOOR");
	}

	int door = 6;
	bool doorSpawned = false;
	//draw walls
	for (int i = 0; i < wallBlocks.size(); i++)
	{
		Tile* curTile = GetTileFromThisOrNeighbor(wallBlocks[i], c_glCoords);
		door--;
		if (door == 0) { continue; }

		*curTile = Tiles::GetTile("TILE_STONE");
	}

	Vector2_I center = buildingBlocks[buildingBlocks.size() / 2];

	//put down a campfire
	GetTileFromThisOrNeighbor({center.x - 1, center.y - 1}, c_glCoords)->hasItem = true;
	GetTileFromThisOrNeighbor({ center.x - 1, center.y - 1 }, c_glCoords)->itemName = "CAMPFIRE";
	GetTileFromThisOrNeighbor({ center.x - 1, center.y - 1 }, c_glCoords)->tileContainer = new Container;

	return center;
}

void Map::PlaceBuilding(Vector2_I startingChunk) {
	Vector2_I cornerstone = { Math::RandInt(0, CHUNK_WIDTH - 1), Math::RandInt(0, CHUNK_HEIGHT - 1) };

	//pick some corners to draw to
	int wallLength = Math::RandInt(4, 9);
	bool newChunk = false;
	std::vector<Vector2_I> chunksCoordsToDelete;

	std::vector<Vector2_I> buildingBlocks = GetSquare(cornerstone, wallLength);
	std::vector<Vector2_I> wallBlocks = GetSquareEdge(cornerstone, wallLength);

	int itemCounter = 0;
	int itemMax = 3;

	//use this for if we create a new chunk to place a building
	Vector2_I newChunkCoords = { -1,-1 };

	//draw the floors
	for (int i = 0; i < buildingBlocks.size(); i++)
	{

		Tile* curTile = GetTileFromThisOrNeighbor({ buildingBlocks[i].x,buildingBlocks[i].y }, startingChunk);

		//if the chunk we are trying to place the building in doesnt exists, create it anew and place
		//the building in there, then save it and close it
		if (curTile == nullptr) {
			//continue;
			Vector2_I chunk_Coords = GetNeighborChunkCoords({buildingBlocks[i].x, buildingBlocks[i].y}, startingChunk);
			if (!DoesChunkExistsOrMakeNew(chunk_Coords)) {
				newChunk = true;
				newChunkCoords = chunk_Coords;
				curTile = GetTileFromThisOrNeighbor(buildingBlocks[i], startingChunk);
				chunksCoordsToDelete.push_back(newChunkCoords);
			}
		}
		//if its somehow still nullptr, skip it
		if (curTile == nullptr) {
			continue;
		}
		
		*curTile = Tiles::GetTile("TILE_STONE_FLOOR");
		if (itemCounter <= itemMax && Math::RandInt(0, 15) == 12) {
			curTile->itemName = Items::GetRandomItemFromPool("house.eid");
			curTile->hasItem = true;
			itemCounter++;
		}
	}

	int door = Math::RandInt(0, wallBlocks.size());
	bool doorSpawned = false;
	//draw walls
	for (int i = 0; i < wallBlocks.size(); i++)
	{
		Tile* curTile = GetTileFromThisOrNeighbor(wallBlocks[i], startingChunk);
		
		//If the wall crosses to a new chunk, load/generate it, place blocks, then unload it at the end
		if (curTile == nullptr) {
			Vector2_I chunk_Coords = GetNeighborChunkCoords( wallBlocks[i], startingChunk);
			if (!DoesChunkExistsOrMakeNew(chunk_Coords)) {
				newChunk = true;
				newChunkCoords = chunk_Coords;
				curTile = GetTileFromThisOrNeighbor(buildingBlocks[i], startingChunk);
				chunksCoordsToDelete.push_back(newChunkCoords);
			}
		}

		door--;
		if (door <= 0 && (i + 1) % wallLength == 0 && !doorSpawned) { doorSpawned = true; continue; }
		*curTile = Tiles::GetTile("TILE_STONE");
	}

	if (newChunk) {
		UnloadChunks(chunksCoordsToDelete);
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
std::vector<Vector2_I> Map::GetSquareEdge(Vector2_I centerTile, int size) {
	std::vector<Vector2_I> squareList;
	int halfSize = size / 2;

	// Top and bottom edges
	for (int x = centerTile.x - halfSize; x <= centerTile.x + halfSize; x++) {
		// Top edge
		squareList.push_back({ x, centerTile.y - halfSize });
		// Bottom edge
		squareList.push_back({ x, centerTile.y + halfSize });
	}

	// Left and right edges (excluding corners already added)
	for (int y = centerTile.y - halfSize + 1; y < centerTile.y + halfSize; y++) {
		// Left edge
		squareList.push_back({ centerTile.x - halfSize, y });
		// Right edge
		squareList.push_back({ centerTile.x + halfSize, y });
	}

	return squareList;
}

std::vector<Vector2_I> Map::GetSquare(Vector2_I centerTile, int size) {
	std::vector<Vector2_I> squareList;
	int halfSize = size / 2;

	for (int x = centerTile.x - halfSize; x <= centerTile.x + halfSize; x++)
	{
		for (int y = centerTile.y - halfSize; y <= centerTile.y + halfSize; y++)
		{
			squareList.push_back({ x,y });
		}
	}
	return squareList;
}

std::vector<Vector2_I> Map::GetCircleEdge(Vector2_I centerTile, int size) {
	std::vector<Vector2_I> circleList;
	for (int r = 0; r <= floor(size * sqrt(0.5)); r++) {
		int d = floor(sqrt(size * size - r * r));
		circleList.push_back({ centerTile.x + d, centerTile.y + r });
		circleList.push_back({ centerTile.x - d, centerTile.y - r });
		circleList.push_back({ centerTile.x + d, centerTile.y - r });
		circleList.push_back({ centerTile.x - d, centerTile.y + r });
		circleList.push_back({ centerTile.x + r, centerTile.y - d });
		circleList.push_back({ centerTile.x + r, centerTile.y + d });
		circleList.push_back({ centerTile.x - r, centerTile.y - d });
		circleList.push_back({ centerTile.x - r, centerTile.y + d });
	}
	return circleList;
}

std::vector<Vector2_I> Map::GetCircle(Vector2_I centerTile, int size) {
	std::vector<Vector2_I> circleList;

	int top = ceil(centerTile.x - centerTile.y),
		bottom = floor(centerTile.x + size);

	for (int x = top; x <= bottom; x++) {
		int   dy = x - centerTile.x;
		float dx = sqrt(size * size - dy * dy);
		int left = ceil(centerTile.y - dx),
			right = floor(centerTile.y + dx);
		for (int y = left; y <= right; y++) {
			circleList.push_back({ x,y });
		}
	}

	return circleList;
}

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

	for (int i = 0; i < list.size(); i++)
	{
		effectLayer.localCoords[list[i].x][list[i].y] = 15;
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

void Map::ClearEntities(std::shared_ptr<Chunk> chunk)
{
	//go to the player and all entities and replace the original tile
	for (int i = 0; i < chunk->entities.size(); i++)
	{
		Vector2_I coord = chunk->entities[i]->coords;
		chunk->localCoords[coord.x][coord.y].entity = nullptr;
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
		try {
			chunk->localCoords
				[chunk->entities[i]->coords.x]
				[chunk->entities[i]->coords.y].entity = chunk->entities[i];
		}
		catch (std::exception e) { Console::Log(e.what(), ERROR, __LINE__); }
	}

}

void Map::TempCheck(Player* p, Vector2_I coords) {
	std::vector<Vector2_I> line = GetLine(coords, p->coords, 10);
	//if a player is near fire
	if (line.size() <= 2) {
		//add 0.25 temp unless theyre at 100 
		p->bodyTemp = p->bodyTemp >= 100 ? 100 : p->bodyTemp + 0.1f;
	}
	else if (line.size() <= 5) {
		//add even faster if theyre closer
		p->bodyTemp = p->bodyTemp >= 100 ? 100 : p->bodyTemp + 0.15f;
	}
}

void Map::UpdateTiles(vec2_i coords, Player* p) {
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
			
			//Crystals glows
			if (curTile->id == 14 ) { floodFill({x, y}, 3, true); }

			//glowing items glow
			int dist = Items::GetItem_NoCopy(curTile->itemName)->emissionDist;
			if (dist > 0) {
				floodFill({ x, y }, dist, true);
			}

			//spread fire to a list so we dont spread more than one tile per update
			if (curTile->liquid == fire) {


				if (Math::RandInt(0, 2) == 1) effectLayer.localCoords[x - 1][y] = 1;
				//If its a campfire, then let it burn but dont spread
				if (curTile->hasItem && curTile->itemName == "CAMPFIRE") {
					Container* curCont = curTile->tileContainer;
					if (curCont != nullptr) {
						for (size_t i = 0; i < curCont->items.size(); i++)
						{
							if (!curCont->items[i].cookable) { continue; }
							//cook an item, then if its done, add the cooked item
							curCont->items[i].ticksUntilCooked -= 1;
							if (curCont->items[i].ticksUntilCooked <= 0) {
								int amount = curCont->items[i].count;
								curCont->AddItem(Items::GetItem(curCont->items[i].cooks_into), amount);
								curCont->items.erase(curCont->items.begin() + i);
								Audio::Play("dat/sounds/cooked.mp3");
								i--;
							}
						}
					}
					floodFill({ x, y }, 5, false);
					TempCheck(p, { x,y });
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

				TempCheck(p, {x,y});
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

/*void Map::CreateContainer(Vector2_I coordsLocal) {
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
}*/

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