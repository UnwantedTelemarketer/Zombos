#ifndef GAME_H
#define GAME_H
#include "map.h"
#include "cosmetic.h"
#include "items.h"
#include <thread>
#include "filemanager.h"
#include <cmath>


using namespace antibox;

struct classes {
	std::string name;
	std::vector<std::string> items; 
	std::vector<int> itemCounts;
};

enum timeOfDay {day, night};

class GameManager {
private:
	float tickRate;
	float effectTickRate;
	double tickCount;
	double effectTickCount;
	bool forwardTime = true;
	std::vector<std::string> missMessages = { "blank", "You swing at nothing and almost fall over.", "You miss.", "You don't hit anything." };
	std::map<std::string, std::vector<std::string>> npcMessages;
public:
	timeOfDay time;
	float sfxvolume = 1.f;
	float musicvolume = 1.f;
	bool freeView = false;
	bool dormantMoon = false;
	CraftingSystem Crafter;
	Map mainMap;
	Player mPlayer;
	Inventory pInv;
	std::vector<std::string> actionLog;
	std::vector<std::string> possibleNames = {"John", "Zombie"};
	std::vector<Vector2_I> oldLocations;
	std::map<Faction, std::vector<Faction>> factionEnemies;
	std::map<int, std::string> tile_icons;
	std::map<std::string, std::string> item_icons;
	std::map<std::string, Vector3> item_colors;
	std::map<int, Vector3> tile_colors;
	vec3 mainBGcolor;
	vec3 bgColor;
	biome currentBiome, lerpingTo;
	float testTime;
	bool startedMusicNight = false;

	std::vector<std::string> sandWalk, grassWalk, rockWalk;

	vec3 BG_DESERT, BG_WATER, BG_FOREST, BG_TAIGA, BG_SWAMP;


	float worldTime = 6.f;
	float darkTime = 1.f;
	bool paused = false;

	bool isDark() { return ((worldTime >= 20.f || worldTime < 6.f) || mainMap.isUnderground); }
	double GetTick() { return (tickRate - tickCount); }
	float TickRate() { return tickRate; }
	void SetTick(float secs) { tickRate = secs * 1000; effectTickRate = tickRate / 10; }

	void AddRecipes();
	void Setup(int x, int y, float tick, int seed, int biome);

	void UpdateEntities(Vector2_I chunkCoords);
	void UpdateTick();
	void UpdateEffects();

	void RedrawEntities(Vector2_I chunkCoords);

	void DoBehaviour(Entity* ent, std::shared_ptr<Chunk> chunkInUse);

	void AttemptAttack(Entity* ent);

	bool PlayerNearby(Vector2_I coords);

	void SpawnEntity(Entity* curNPC);

	void MovePlayer(int dir);

	void SetTile(Vector2_I tile, int newTile);
	
	Entity* NearEnt();

	bool EnterCave();
	void LoadData();
	std::string GetWalkSound();

	std::string GetItemChar(Tile* tile);
	ImVec4 GetItemColor(Tile* tile, float intensity);
	std::string GetTileChar(Tile* tile);

	std::string GetTileChar(Vector2_I tile);

	ImVec4 GetTileColor(Tile* tile, float intensity);
	ImVec4 GetTileColor(Vector2_I tile, float intensity);
	ImVec4 GetPlayerColor();
	Tile* SelectTile(Vector2_I coords);
};


static void T_UpdateChunk(GameManager* gm, Vector2_I coords)
{
	if (coords.x >= MAP_WIDTH || coords.y >= MAP_HEIGHT || coords.x < 0 || coords.y < 0) { return; }
	gm->UpdateEntities(coords);
	gm->mainMap.UpdateTiles(coords, &gm->mPlayer);
}

void GameManager::LoadData() {
	OpenedData data;
	ItemReader::GetDataFromFile("dialogue.eid", "WANDERER_CALM", &data);
	npcMessages.insert({ "wanderer_calm", {} });
	for (size_t i = 1; i <= data.getInt("size"); i++)
	{
		npcMessages["wanderer_calm"].push_back(data.getString(std::to_string(i)));
	}

	OpenedData tileData;
	ItemReader::GetDataFromFile("tiles.eid", "TILES", &tileData);

	for (auto const& x : tileData.tokens) {
		tile_icons.insert({ stoi(tileData.getArray(x.first)[1]) , tileData.getArray(x.first)[0] });
	}
	lerpingTo = urban;
}

void GameManager::Setup(int x, int y, float tick, int seed = -1, int biome = -1) {
	BG_DESERT = { 0.15, 0.15, 0 };
	BG_WATER = { 0, 0.1, 0.15 };
	BG_FOREST = { 0, 0.15, 0 };
	BG_TAIGA = { 0, 0.2, 0.15 };
	BG_SWAMP = { 0.1, 0.15, 0.05 };
	sandWalk = { "dat/sounds/movement/sand1.wav","dat/sounds/movement/sand2.wav", "dat/sounds/movement/sand3.wav" };
	grassWalk = { "dat/sounds/movement/grass1.wav","dat/sounds/movement/grass2.wav", "dat/sounds/movement/grass3.wav" };
	rockWalk = { "dat/sounds/movement/rock_walk1.wav","dat/sounds/movement/rock_walk2.wav", "dat/sounds/movement/rock_walk3.wav" };
	mainMap.SetWeather(clear);
	mainMap.ticksUntilWeatherUpdate = Math::RandInt(15, 600);

	//if(seed == -1) deleteAllFilesInDirectory();
	SetTick(tick);
	mPlayer.coords.x = x;
	mPlayer.coords.y = y;
	AddRecipes();
	mainMap.CreateMap(seed, biome);

	//Faction, Enemies
	factionEnemies = {
		{Human_W, {Zombie}},
		{Zombie, {Human_W, Wildlife}},
		{Wildlife, {}}
	};
	
	mainMap.isUnderground = false;
	Math::PushBackLog(&actionLog, "Press H to open the help menu.");
}

void GameManager::AddRecipes() {
	std::map<std::string, int> currentRecipe;
	OpenedData recipes;
	ItemReader::GetDataFromFile("items/crafting_recipes.eid", "SECTIONS", &recipes);
	for (auto const& x : recipes.getArray("sections")) {

		OpenedData sectionsData;
		ItemReader::GetDataFromFile("items/crafting_recipes.eid", x, &sectionsData);
		std::vector<std::string> curArr = sectionsData.getArray("recipe");
		for (size_t i = 0; i < curArr.size(); i+=2)
		{
			currentRecipe[curArr[i]] = stoi(curArr[i + 1]);
		}
		Crafter.addRecipe(sectionsData.section_name, currentRecipe);
		currentRecipe.clear();
	}
}

void GameManager::DoBehaviour(Entity* ent, std::shared_ptr<Chunk> chunkInUse)
{
	vec2_i oldCoords = ent->coords;
	std::vector<Vector2_I> path = mainMap.GetLine(ent->coords, mPlayer.coords, 20);
	std::vector<Vector2_I> entPath;
	Entity* tempTarget = nullptr;
	bool moved = false;
	ent->tempViewDistance = isDark() ? ent->viewDistance * 2 : ent->viewDistance;

	for (int i = 0; i < chunkInUse->entities.size(); i++) //loop through every entity
	{
		Entity* curEnt = chunkInUse->entities[i];

		if (curEnt == ent || curEnt->health <= 0) { continue; } //check that we arent looking at the same entity twice

		std::vector<Vector2_I> curPath = mainMap.GetLine(ent->coords, curEnt->coords, 10); //get line to other ent

		if (curPath.size() < entPath.size() || entPath.size() == 0) { //check that its long enough and target them
			entPath = curPath;
			tempTarget = chunkInUse->entities[i];
		}
	}

	//mainMap.DrawLine(entPath); //draw the path between entities (for debug)

	int dir = Math::RandInt(1, 10);

	Tile* tile = chunkInUse->GetTileAtCoords(ent->coords);

	switch (ent->b) //check the entities behaviour
	{
	case Follow:
		if (path.size() >= 3) {
			ent->coords = path[1];
			moved = true;
		}
		else {
			//do nothing
		}
		break;
	case Aggressive:
		//check if player is near
		if (path.size() < ent->viewDistance) {
			ent->targetingPlayer = true;
			if (ent->name == "Zombie") {
				Audio::Play("dat/sounds/zombie_angry.mp3");
			}
		}
		else {
			ent->targetingPlayer = false;
		}
	case Protective:
		//make sure theyre above health and nearby
		if (ent->health > 5 && (tempTarget != nullptr || ent->targetingPlayer)) {
			int isEnemies = 0;

			if (!ent->targetingPlayer) {
				isEnemies = std::find(factionEnemies[ent->faction].begin(), factionEnemies[ent->faction].end(), tempTarget->faction) != factionEnemies[ent->faction].end();
			}
			//if the player is targeted
			else if (ent->targetingPlayer) {
				if (mPlayer.coveredIn == guts && ent->name == "Zombie") { ent->targetingPlayer = false; }
				if (path.size() > 1) {
					if (chunkInUse->GetTileAtCoords(path[1])->walkable) {
						ent->coords = path[1];
						moved = true;
					}
					else {
						moved = false;
					}
				}
				else { 
					if (chunkInUse->GetTileAtCoords(path[0])->walkable) {
						ent->coords = path[0];
						moved = true;
					}
					else {
						moved = false;
					}
				}
				break;
			}

			//otherwise, if they are enemies go towards them
			if (isEnemies != 0) {
				if (entPath.size() < ent->viewDistance) { //if theyre within view distance, target them
					ent->target = tempTarget;
				}

				if (entPath.size() > 2) {
					if (chunkInUse->GetTileAtCoords(entPath[1])->walkable) {
						ent->coords = entPath[1];
						moved = true;
					}
					else {
						moved = false;
					}
					break;
				}
				//if theyre close enough, attack them
				else if (entPath.size() <= 2) {
					mainMap.AttackEntity(ent->target, ent->damage, &actionLog, chunkInUse);
					if (ent->target->health <= 0) {
						ent->target = nullptr;
						ent->aggressive = false;
					}
				}
			}
		}
		//if nothing else, drop the target
		else {
			ent->target = nullptr;
			ent->aggressive = false;
		}
	case Wander:
		//wander around, unless they can talk
		if (PlayerNearby(oldCoords)) {
			ent->talking = true;
			break;
		}
		else {
			ent->talking = false;
		}

		//run out of liquid
		if (tile != nullptr) {
			if (tile->liquid == water) {
				ent->coords.x--;
				break;
			}
		}
		else {
			moved = true;
			//pr do random direction 
			switch (dir)
			{
			case 1:
				ent->coords.x++;
				break;
			case 2:
				ent->coords.x--;
				break;
			case 3:
				ent->coords.y++;
				break;
			case 4:
				ent->coords.y--;
				break;
			case 5:
				ent->coords.x++;
				ent->coords.y++;
				break;
			case 6:
				ent->coords.x--;
				ent->coords.y++;
				break;
			case 7:
				ent->coords.x++;
				ent->coords.y--;
				break;
			case 8:
				ent->coords.x--;
				ent->coords.y--;
				break;
			}
		}
	}

	if (moved) { 
		if (mainMap.TileAtPos(ent->coords)->itemName == "BEAR_TRAP") {
			Audio::Play("dat/sounds/bear_trap.mp3");
			ent->health -= 35;
			mainMap.TileAtPos(ent->coords)->itemName = "BEAR_TRAP_2";
		}
	}
	if (ent->coords == mPlayer.coords || mainMap.GetTileFromThisOrNeighbor(ent->coords)->walkable == false) {
		ent->coords = oldCoords;
		return;
	}
	//cover entities in liquid if they step in it
	if (tile != nullptr) {
		if (tile->liquid != nothing) {
			ent->coveredIn = tile->liquid;
			ent->ticksUntilDry = Math::RandInt(10, 30);
		}
		else if (ent->coveredIn != nothing) {
			if (ent->coveredIn == fire) {
				ent->health -= 5;
			}
			tile->SetLiquid(ent->coveredIn);
		}
	}

	if (ent->ticksUntilDry > 0) {
		ent->ticksUntilDry -= 1;
	}
	else {
		ent->coveredIn = nothing;
	}

	//mainMap.CurrentChunk()->localCoords[oldCoords.x][oldCoords.y].entity = nullptr;
}

bool GameManager::PlayerNearby(Vector2_I coords) {
	if (mPlayer.coords == vec2_i{coords.x + 1, coords.y} ||
		mPlayer.coords == vec2_i{coords.x - 1, coords.y} ||
		mPlayer.coords == vec2_i{coords.x, coords.y + 1} ||
		mPlayer.coords == vec2_i{coords.x, coords.y - 1} )
	{
		return true;
	}
	return false;
}


/*Entity* GameManager::NearEnt() {
	if (mPlayer.coords.x + 1 < CHUNK_WIDTH && mainMap.TileAtPos(vec2_i{ mPlayer.coords.x + 1, mPlayer.coords.y })->entity != nullptr) {
		return mainMap.TileAtPos(vec2_i{ mPlayer.coords.x + 1, mPlayer.coords.y })->entity;
	}
	else if (mPlayer.coords.x - 1 >= 1 && mainMap.TileAtPos(vec2_i{ mPlayer.coords.x - 1, mPlayer.coords.y })->entity != nullptr) {
		return mainMap.TileAtPos(vec2_i{ mPlayer.coords.x - 1, mPlayer.coords.y })->entity;
	}
	else if (mPlayer.coords.y + 1 < CHUNK_HEIGHT && mainMap.TileAtPos(vec2_i{ mPlayer.coords.x, mPlayer.coords.y + 1 })->entity != nullptr) {
		return mainMap.TileAtPos(vec2_i{ mPlayer.coords.x, mPlayer.coords.y + 1 })->entity;
	}
	else if (mPlayer.coords.y - 1 >= 1 && mainMap.TileAtPos(vec2_i{ mPlayer.coords.x, mPlayer.coords.y - 1 })->entity != nullptr) {
		return mainMap.TileAtPos(vec2_i{ mPlayer.coords.x, mPlayer.coords.y - 1 })->entity;
	}
	return nullptr;
}*/

Tile* GameManager::SelectTile(Vector2_I coords) {
	Tile* selTile = mainMap.GetTileFromThisOrNeighbor(coords);
	if (selTile->entity != nullptr && selTile->entity->canTalk) {
		selTile->entity->message = npcMessages["wanderer_calm"][Math::RandInt(0, 7)];
	}
	return mainMap.GetTileFromThisOrNeighbor(coords);
}

void GameManager::SpawnEntity(Entity* ent) {
	ent->health = Math::RandNum(100);
	ent->coords.x = 10;
	ent->coords.y = 15;
	//if(curNPC->name == "") curNPC->name = Math::RandString(possibleNames);
	mainMap.CurrentChunk()->entities.push_back(ent);
	ent->index = mainMap.CurrentChunk()->entities.size() - 1;
}

void GameManager::MovePlayer(int dir) {
	Tile* playerTile = mainMap.GetTileFromThisOrNeighbor(mPlayer.coords);
	if (playerTile->liquid == mud || 
		(playerTile->liquid == water && playerTile->liquidTime == -1)) {
		//hardcoding being able to walk through mud in leather boots lol
		if (Math::RandInt(0, 4) == 3 && !pInv.CurrentEquipMatches(boots, "LEATHER_BOOTS")) {
			Math::PushBackLog(&actionLog, "You get stuck in the liquid.");
			return;
		}
	}
	switch (dir) {
	case 1:
		mainMap.MovePlayer(mPlayer.coords.x - 1, mPlayer.coords.y, &mPlayer, &actionLog, pInv);
		break;
	case 2:
		mainMap.MovePlayer(mPlayer.coords.x + 1, mPlayer.coords.y, &mPlayer, &actionLog, pInv);
		break;
	case 3:
		mainMap.MovePlayer(mPlayer.coords.x, mPlayer.coords.y - 1, &mPlayer, &actionLog, pInv);
		break;
	case 4:
		mainMap.MovePlayer(mPlayer.coords.x, mPlayer.coords.y + 1, &mPlayer, &actionLog, pInv);
		break;
	default:
		break;
	}
			//entering a cave
	if (mainMap.TileAtPos(mPlayer.coords)->itemName == "BEAR_TRAP") {
		mPlayer.TakeDamage(pierceDamage, 35);
		Audio::Play("dat/sounds/bear_trap.mp3");
		mainMap.TileAtPos(mPlayer.coords)->itemName = "BEAR_TRAP_2";
	}
	if (mainMap.TileAtPos(mPlayer.coords)->id == 19) {
		if (EnterCave()) {
			freeView = false;
			//Audio::LerpMusic("ambient_day", "dat/sounds/wet-pizza-rat.mp3", "ambient_cave");
			Audio::StopLoop("ambient_day");
			Audio::PlayLoop("dat/sounds/music/wet-pizza-rat.mp3", "ambient_cave");
			Math::PushBackLog(&actionLog, "You enter a dark cave underground.");
		}
	}

	biome curBiome = mainMap.GetBiome(mPlayer.coords);
	if(!isDark())
	if (curBiome == desert) { 
		if (lerpingTo != desert) {
			Utilities::Lerp("bgColor", &bgColor, BG_DESERT, 0.5f);
			mainBGcolor = BG_DESERT;
			currentBiome = lerpingTo = desert;
		}
	}
	else if (curBiome == taiga) {
		if (lerpingTo != taiga) {
			Utilities::Lerp("bgColor", &bgColor, BG_TAIGA, 1.f);
			mainBGcolor = BG_TAIGA;
			currentBiome = lerpingTo = taiga;
		}
	}
	else if (curBiome == forest){
		if (lerpingTo != forest) {
			Utilities::Lerp("bgColor", &bgColor, BG_FOREST, 1.f);
			mainBGcolor = BG_FOREST;
			currentBiome = lerpingTo = forest;
		}
	}
	else if (curBiome == ocean) {
		if (lerpingTo != ocean) {
			Audio::Play("dat/sounds/movement/enter_water.wav");
			Utilities::Lerp("bgColor", &bgColor, BG_WATER, 1.f);
			mainBGcolor = BG_WATER;
			currentBiome = lerpingTo = ocean;
		}
	}
	else {
		if (lerpingTo != swamp) {
			Utilities::Lerp("bgColor", &bgColor, BG_SWAMP, 0.5f);
			mainBGcolor = BG_SWAMP;
			currentBiome = lerpingTo = swamp;
		}
	}

	//relic of the past
	//if (mainMap.NearNPC(player)) { Math::PushBackLog(&actionLog, "Howdy!"); }
}


void GameManager::UpdateEffects() {

	//1 is smoke, 2 is rain
	int tempMap[30][30]{};

	if (mainMap.currentWeather == rainy || mainMap.currentWeather == thunder) {
		tempMap[0][Math::RandInt(0, 29)] = 2;
		tempMap[0][Math::RandInt(0, 29)] = 2;
	}

	for (int i = 0; i < CHUNK_HEIGHT; i++)
	{
		for (int j = 0; j < CHUNK_WIDTH; j++)
		{
			if (mainMap.effectLayer.localCoords[i][j] == 1) {
				if (Math::RandInt(0, 3) == 1) {
					int newJ = j + Math::RandInt(0, 2);
					int newI = i - Math::RandInt(0, 2);
					if ((newI >= CHUNK_WIDTH || newI < 0) || (newJ >= CHUNK_WIDTH || newJ < 0)) { continue; }
					tempMap[newI][newJ] = 1;
				}
				else {
					tempMap[i][j] = 1;
				}
			}
			if (mainMap.effectLayer.localCoords[i][j] == 2) {
				if (i + 2 >= CHUNK_WIDTH || i < 0) { tempMap[i][j] = 0; }

				else if (Math::RandInt(0, 30) == 1 && mainMap.TileAtPos({ i,j })->id != 13 && mainMap.TileAtPos({ i,j })->liquid == nothing)
				{ tempMap[i][j] = 0; mainMap.TileAtPos({ i,j })->SetLiquid(water); }

				else { tempMap[(int)std::floor((i + 1) * 1.1)][j] = 2; }
			}
		}
	}
	for (size_t i = 0; i < CHUNK_HEIGHT; i++)
	{
		for (size_t j = 0; j < CHUNK_WIDTH; j++)
		{
			mainMap.effectLayer.localCoords[i][j] = tempMap[i][j];
		}
	}
}

void GameManager::UpdateTick() {

	if (paused) { return; }
	tickCount += antibox::Engine::Instance().deltaTime();
	effectTickCount += antibox::Engine::Instance().deltaTime();

	if (mainMap.chunkUpdateFlag)
	{
		RedrawEntities(mainMap.c_glCoords);
		mainMap.chunkUpdateFlag = false;
	}

	if (effectTickCount >= effectTickRate) {
		UpdateEffects();
		effectTickCount = 0;
	}


	if (pInv.CurrentEquipExists(weapon)) {
		int dist = pInv.equippedItems[weapon].emissionDist;
		if (dist > 0) {
			mainMap.floodFill(mPlayer.coords, dist, false);
		}
	}

	if (tickCount >= tickRate)
	{
		float curTime = glfwGetTime();
		if (mainMap.UpdateWeather()) {
			switch (mainMap.currentWeather) {
			case rainy:
				Math::PushBackLog(&actionLog, "It begins to rain.");
				break;
			case clear:
				Math::PushBackLog(&actionLog, "The weather clears up.");
				break;
			case thunder:
				Math::PushBackLog(&actionLog, "Thunder booms in the distance.");
				break;
			}
		}

		tickCount = 0;
		//hunger and thirst
		mPlayer.thirst -= 0.075f;
		mPlayer.hunger -= 0.05f;
		if (mPlayer.thirst < 0) { mPlayer.health -= 0.5f; mPlayer.thirst = 0; }
		if (mPlayer.hunger < 0) { mPlayer.health -= 0.5f; mPlayer.hunger = 0; }
		if (mPlayer.coveredIn == fire) { mPlayer.health -= 1.f; }

		//Liquid spilling
		if (mPlayer.coveredIn != nothing) { mPlayer.ticksCovered++; }
		if (mPlayer.ticksCovered > mPlayer.liquidLast) { mPlayer.ticksCovered = 0; mPlayer.coveredIn = nothing; }

		//if player stands in liquid, soak em.
		Liquid tileLiquid = mainMap.CurrentChunk()->GetTileAtCoords(mPlayer.coords)->liquid;
		if (tileLiquid != nothing && tileLiquid != fire) {
			if (!pInv.Waterproof(boots)) {
				mPlayer.CoverIn(tileLiquid, Math::RandInt(10, 30));
			}
		}

		if (mainMap.currentWeather == rainy || mainMap.currentWeather == thunder) {
			if (Math::RandInt(1, 7) == 2)  //random check
			{
				if (!pInv.Waterproof(shirt) && mainMap.TileAtPos(mPlayer.coords)->id != 13) {
					mPlayer.CoverIn(water, 15); //cover the player in it
				}
			}
		}

		if (mPlayer.coveredIn == water || mPlayer.bodyTemp > 98.5f) {
			mPlayer.bodyTemp -= 0.025f;
		}

		else if (!isDark()) {
			if (mPlayer.bodyTemp < 98.5f) mPlayer.bodyTemp += 0.025f;
		}

		if (mPlayer.bodyTemp < 95.f) {
			mPlayer.health -= 1.f;
		}

		//soak items and tick down to dry them off in your inventory
		for (int i = 0; i < pInv.items.size(); i++)
		{
			//if the player is covered in something, chance to cover items
			if (mPlayer.coveredIn != nothing) {
				if (Math::RandInt(1, 7) == 5) {
					pInv.items[i].CoverIn(mPlayer.coveredIn, Math::RandInt(30, 120));
				}
			}

			//if the item is soaked, dry it off slowly
			if (pInv.items[i].ticksUntilDry > 0) {
				pInv.items[i].ticksUntilDry -= 1;
			}
			else{
				pInv.items[i].coveredIn = nothing;
			}
		}

		//time and brightness
		worldTime += 0.05f;
		if (worldTime > 24.f) { worldTime = 0.f; }

		if (!mainMap.isUnderground) {
			if (worldTime >= 20.f || worldTime < 6.f) {
				time = night;
				if (!startedMusicNight) {
					Audio::StopLoop("ambient_day");
					Audio::PlayLoop("dat/sounds/music/night_zombos.wav", "night_music");
					Audio::PlayLoop("dat/sounds/crickets.mp3", "crickets");
					startedMusicNight = true;
				}
				if (forwardTime) { darkTime = std::min(10.f, darkTime + 0.45f); }
				else { darkTime = std::max(1.f, darkTime - 0.5f); }
				if (worldTime >= 4.f && worldTime <= 5.f) { forwardTime = false; }
			}
			else if (worldTime > 6.f && worldTime < 6.05f) {
				time = day;
				startedMusicNight = false;
				mainMap.ResetLightValues();
				Audio::StopLoop("night_music");
				Audio::StopLoop("crickets");
				Audio::PlayLoop("dat/sounds/music/ambient12.wav", "ambient_day");
				Audio::SetVolumeLoop(musicvolume, "ambient_day");
			}
			else {
				forwardTime = true;
				darkTime = 1.f;
			}
		}
		else {
			darkTime = 4.f;
		}


		//Update surrounding chunks in separate threads for "super speed" >:)
		/*std::thread c1 = std::thread(T_UpdateChunk, this, Vector2_I{mainMap.c_glCoords.x + 1,  mainMap.c_glCoords.y});
		std::thread c2 = std::thread(T_UpdateChunk, this, Vector2_I{ mainMap.c_glCoords.x - 1,  mainMap.c_glCoords.y });
		std::thread c3 = std::thread(T_UpdateChunk, this, Vector2_I{ mainMap.c_glCoords.x,  mainMap.c_glCoords.y + 1 });
		std::thread c4 = std::thread(T_UpdateChunk, this, Vector2_I{ mainMap.c_glCoords.x,  mainMap.c_glCoords.y - 1 });

		c1.join();
		c2.join();
		c3.join();
		c4.join();*/

		//hardcoded version of updating surrounding chunks
		T_UpdateChunk(this, mainMap.c_glCoords);
		/*if (!mainMap.isUnderground) {
			T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x + 1,  mainMap.c_glCoords.y });
			T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x - 1,  mainMap.c_glCoords.y });
			T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x,  mainMap.c_glCoords.y + 1 });
			T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x,  mainMap.c_glCoords.y - 1 });
		}*/
		testTime = (glfwGetTime() - curTime) * 1000;

		//Utilities::Lerp("playertemp", &mPlayer.visualTemp, mPlayer.bodyTemp, 0.5f);
	}


}

void GameManager::RedrawEntities(Vector2_I chunkCoords) {
	std::shared_ptr<Chunk> usedChunk = mainMap.GetProperChunk(chunkCoords);
	mainMap.ClearChunkOfEnts(usedChunk);
}

void GameManager::UpdateEntities(Vector2_I chunkCoords) {

	std::shared_ptr<Chunk> usedChunk = mainMap.GetProperChunk(chunkCoords);

	mainMap.ClearLine();
	mainMap.ClearEntities(usedChunk);
	for (int i = 0; i < usedChunk->entities.size(); i++)
	{
		if (i >= usedChunk->entities.size()) { break; }
		if (usedChunk->entities[i]->health <= 0) { continue; }

		if (usedChunk->entities[i]->aggressive && usedChunk->globalChunkCoord == mainMap.CurrentChunk()->globalChunkCoord)
		{
			AttemptAttack(usedChunk->entities[i]);
		}

		if (Math::RandInt(1, 10) >= 4 && !usedChunk->entities[i]->targeting()) { continue; }

		DoBehaviour(usedChunk->entities[i], usedChunk);
		if (mainMap.CheckBounds(usedChunk->entities[i], usedChunk)) {
			i--;
		}
	}

	mainMap.PlaceEntities(usedChunk);
}

void GameManager::AttemptAttack(Entity* ent)
{
	if (Vector2_I{ ent->coords.x + 1, ent->coords.y } == mPlayer.coords ||
		Vector2_I{ ent->coords.x - 1, ent->coords.y } == mPlayer.coords ||
		Vector2_I{ ent->coords.x, ent->coords.y + 1 } == mPlayer.coords ||
		Vector2_I{ ent->coords.x, ent->coords.y - 1 } == mPlayer.coords)
	{
		if (std::count(mainMap.CurrentChunk()->entities.begin(), mainMap.CurrentChunk()->entities.end(), ent))
		{
			Audio::Play("dat/sounds/human damaged.mp3");
			std::string attackmsg = " hits you for ";
			Math::PushBackLog(&actionLog, ent->name + attackmsg + std::to_string(ent->damage) + " damage!");
			mPlayer.TakeDamage(pierceDamage, ent->damage);
			bgColor = { 0.5f,0,0 };
			Utilities::Lerp("bgColor", &bgColor, mainBGcolor, 0.5f);
		}
	}
}

void GameManager::SetTile(Vector2_I tile, int newTile) {
	mainMap.CurrentChunk()->localCoords[tile.x][tile.y].id = newTile;
}

bool GameManager::EnterCave() {
	if (!mainMap.isUnderground)
	{
		if (mainMap.underground.chunks.count({ mainMap.c_glCoords }) == 0) {
			lerpingTo = urban;
			mainBGcolor = { 0,0,0 };
			Utilities::Lerp("bgColor", &bgColor, mainBGcolor, 0.5f);
			std::shared_ptr<Chunk> tempChunk = std::make_shared<Chunk>();
			tempChunk->globalChunkCoord = mainMap.c_glCoords;
			mainMap.GenerateTomb(tempChunk);

			mainMap.underground.chunks[{mainMap.c_glCoords}] = tempChunk;
		}
		mainMap.isUnderground = true;
		return true;
	}
	return false;
}

std::string GameManager::GetItemChar(Tile* tile) {
	if (!tile->hasItem || item_icons.count(tile->itemName) == 0) {
		return "!";
	}
	return item_icons[tile->itemName];
}

ImVec4 GameManager::GetItemColor(Tile* tile, float intensity = -1.f) {
	if (!tile->hasItem) {
		return {1, 0, 0, 1};
	}
	if (intensity == -1.f) { intensity = tile->brightness; }
	if (tile->liquid == fire) { return Cosmetic::FireColor(); }
	vec3 color = Items::GetItemColor(tile->itemName);
	color.x /= (darkTime * intensity);
	color.y /= (darkTime * intensity);
	color.z /= (darkTime * intensity);
	return {color.x, color.y, color.z, 1};

}

std::string GameManager::GetTileChar(Tile* tile) {

	if (tile->entity != nullptr)
	{
		switch (tile->entity->entityID)
		{
		case ID_ZOMBIE:
			return ENT_ZOMBIE;
		case ID_CHICKEN:
			return ENT_CHICKEN;
		case ID_HUMAN:
			return ENT_HUMAN;
		case ID_FROG:
			return ENT_FROG;
		case ID_CAT:
			return ENT_CAT;
		case ID_COW:
			return ENT_COW;
		case ID_FINDER:
			return ENT_FINDER;
		case ID_TAKER:
			return ENT_TAKER;
		}
	}
	//if (tile.hasItem) {
		//return item_icons[tile.itemName];
	//}
	if (tile->liquid == water && tile->liquidTime == -1) {
		return "A";
	}
	return tile_icons[tile->id];
}

std::string GameManager::GetTileChar(Vector2_I tile) {
	return GameManager::GetTileChar(&mainMap.CurrentChunk()->localCoords[tile.x][tile.y]);
}

std::string GameManager::GetWalkSound(){

	if(mainMap.isUnderground) return rockWalk[Math::RandInt(0, 2)];

	switch (currentBiome) {
	case ocean:
	case desert:
		return sandWalk[Math::RandInt(0, 2)];
		break;
	case forest:
	case taiga:
	case swamp:
	case grassland:
		return grassWalk[Math::RandInt(0, 2)];
		break;
	default:
		return "dat/sounds/bfxr/walk1.wav";
	}
}

ImVec4 GameManager::GetPlayerColor() {
	Vector3 end_color = { pInv.clothes.x, pInv.clothes.y, pInv.clothes.z};
	int amount_of_colors = 1;
	/*if (mainMap.CurrentChunk()->localCoords[mPlayer.coords.x + 1][mPlayer.coords.y].double_size)
	{
		end_color += { 0, 0.35, 0 };
		amount_of_colors++;
	}*/
	switch (mPlayer.coveredIn) {
	case water:
		end_color += { 0, 0, 0.8 };
		amount_of_colors++;
		break;
	case blood:   
		end_color += {0.45, 0, 0};
		amount_of_colors++;
		break;
	default:
		break;
	}

	end_color /= amount_of_colors;

	return ImVec4{ end_color.x, end_color.y, end_color.z, 1};
}

ImVec4 GameManager::GetTileColor(Tile* tile, float intensity) {
	ImVec4 color;
	//check for entitites
	if (tile->entity != nullptr)
	{
		if (tile->entity->health <= 0) {
			color = { 1,0,0,1 };
			goto dimming;
		}
		switch (tile->entity->entityID)
		{
		case ID_ZOMBIE:
			color = { 0,1,0,1 };
			goto dimming;
			break;
		case ID_CHICKEN:
			color = { 1,1,1,1 };
			goto dimming;
			break;
		case ID_HUMAN:
			color = { 1,1,1,1 };
			goto dimming;
			break;
		case ID_FROG:
			color = { 0,0.7,0,1 };
			goto dimming;
			break;
		case ID_CAT:
		case ID_COW:
			color = { 1,1,1,1 };
			goto dimming;
			break;
		}
	}
	if (tile->liquid == fire) {
		color = Cosmetic::FireColor();
		goto dimming;
	}

	//check if its burnt
	if (tile->burningFor > 0) {
		color = { 0.45, 0.45, 0.45, 1 };
		goto dimming;
	}

	color = { tile->tileColor.x, tile->tileColor.y, tile->tileColor.z, 1 };
	//regular tile color
	/*switch (tile.id) {
	case 0:
		color = { 0.75, 0.75, 0.75, 1 };
		break;
	case 2: //dirt
		color = { 1, 0.45, 0, 1 };
		break;
	case 3: //flower
		color = { 0.65, 1, 0.1, 1 };
		break;
	case 5: //scrap
		color = { 1, 0.5, 0, 1 };
		break;
	case 100://conveyor belts
	case 101:
	case 102:
	case 103:
	case 104:
	case 105:
	case 106:
	case 107:
	case 6: //stone
		color = { 0.75, 0.75, 0.75, 1 };
		break;
	case 7: //sand
		color = { 1, 1, 0.5, 1 };
		break;
	case 11:
		color = { 0,0.35,0,1 };
		break;
	case 13:
		color = { 0.23,0.23,0.23,1 };
		break;
	case 14: //crystal
		color = { 0.95,0.5,0.f,1 };
		break;
	case 15: //big rock
		color = { 0.35,0.35,0.35,1 };
		break;
	case 16: //snow
		color = { 0.85,0.85,0.9,1 };
		break;
	case 17: //mud
		color = { 0.35,0.15,0.1,1 };
		break;
	default:
		color = { 0, 0.65, 0, 1 };
		break;
	}
	*/
dimming:
	//if its night time
	if ((worldTime >= 20.f || worldTime < 6.f) || mainMap.isUnderground) {
		if ((darkTime >= 10.f && intensity >= 1.f) || (mainMap.isUnderground && intensity >= 1.f)) {
			if(dormantMoon) color = { 0.075f,0.075f,0.075f,1 };
			else color = { 0.f,0.f,0.f,1 };
		}
		else {
			color.x /= (darkTime * intensity);
			color.y /= (darkTime * intensity);
			color.z /= (darkTime * intensity);
		}
	}

	return color;
}


ImVec4 GameManager::GetTileColor(Vector2_I tile, float intensity) {
	return GameManager::GetTileColor(&mainMap.CurrentChunk()->localCoords[tile.x][tile.y], intensity);
}

class Commands {
private:
	std::vector<std::string> previous_commands;
public:
	void RunCommand(std::string input, GameManager* game);
	std::string GetOldCommand(int* index);
	int prevCommandSize() { return previous_commands.size(); }
};

void Commands::RunCommand(std::string input, GameManager* game) {
	std::vector<std::string> tokens = Tokenizer::getTokens(input);
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "give") {
			if (tokens.size() < 3) { return; }
			if (Items::list.count(tokens[i + 1]) == 0) { 
				LAZY_LOG("Item \"" + tokens[i+1] + "\" cannot be found.")
					return; 
			}
			game->pInv.AddItemByID(tokens[i + 1], stoi(tokens[i + 2]));
		}
		else if (tokens[i] == "set")
		{
			if (tokens.size() < 3) { return; }
			if (tokens[i + 1] == "health") {
				game->mPlayer.health = stoi(tokens[i + 2]);
			}
			else if (tokens[i + 1] == "thirst") {
				game->mPlayer.thirst = stoi(tokens[i + 2]);
			}
			else if (tokens[i + 1] == "hunger") {
				game->mPlayer.hunger = stoi(tokens[i + 2]);
			}
			else if (tokens[i + 1] == "weather") {
				if(tokens[i + 2] == "clear") {
					game->mainMap.SetWeather(clear);
				}else if (tokens[i + 2] == "rain") {
					game->mainMap.SetWeather(rainy);
				}else if (tokens[i + 2] == "thunder") {
					game->mainMap.SetWeather(thunder);
				}
			}
		}
		else if (tokens[i] == "help") {
			std::string helpstring =
						  "\ngive {item name} {amount} - gives item\n";
			helpstring += "set weather {clear / rain / thunder} - sets the weather\n";
			helpstring += "set {health / hunger / thirst} {number} - sets attribute\n";
			helpstring += "help - this\n";

			Console::Log(helpstring, text::white, -1);
		}
	}
	if (previous_commands.size() > 5) {
		previous_commands.erase(previous_commands.begin());
	}
	//if the previous commands already has the command we run, so we dont have duplicates
	if (std::find(previous_commands.begin(), previous_commands.end(), input) == previous_commands.end()) {
		previous_commands.push_back(input);
	}

}
std::string Commands::GetOldCommand(int* index) {
	*index += 1;
	if (*index >= previous_commands.size()) { *index = 0; }
	return previous_commands[*index];
}

#endif