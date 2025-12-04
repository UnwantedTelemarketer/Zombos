#ifndef GAME_H
#define GAME_H
#include "map.h"
#include "cosmetic.h"
#include "items.h"
#include <thread>
#include "filemanager.h"
#include "glyphmanager.h"
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
	
public:
	std::map<std::string, std::vector<std::string>> npcMessages;
	timeOfDay time;
	float sfxvolume = 1.f;
	float musicvolume = 1.f;
	bool freeView = false;
	bool dormantMoon = false;
	CraftingSystem Crafter;
	Map mainMap;
	Player mPlayer;
	Inventory pInv;
	std::vector<std::string> actionLog, consoleLog;
	std::vector<std::string> possibleNames = {"John", "Zombie"};
	std::vector<std::string> quietWalkSounds = {"dat/sounds/movement/grass1_quiet.wav","dat/sounds/movement/grass2_quiet.wav" ,"dat/sounds/movement/grass3_quiet.wav" };
	std::vector<Vector2_I> oldLocations;
	std::map<std::string, std::string> item_icons;
	std::map<std::string, Vector3> item_colors;
	std::map<int, Vector3> tile_colors;
	GlyphManager glyphs;
	vec3 mainBGcolor;
	vec3 bgColor;
	biome currentBiome, lerpingTo;
	float testTime;
	bool startedMusicNight = false;
	float reg_font_size = 16.f;

	std::vector<std::string> sandWalk, grassWalk, rockWalk;

	vec3 BG_DESERT, BG_WATER, BG_FOREST, BG_TAIGA, BG_SWAMP;


	float darkTime = 1.f;
	bool paused = false;

	bool isDark() { return ((mainMap.worldTimeTicks >= 2850 || mainMap.worldTimeTicks < 900) || mainMap.playerLevel != 0); }
	bool isNight() { return (mainMap.worldTimeTicks >= 2850 || mainMap.worldTimeTicks < 900); }
	double GetTick() { return (tickRate - tickCount); }
	float TickRate() { return tickRate; }
	void SetTick(float secs) { tickRate = secs * 1000; effectTickRate = tickRate / 10; }

	void AddRecipes();
	void Setup(int x, int y, float tick, int seed, int biome, int moisture);

	void UpdateEntities(Vector2_I chunkCoords);
	void UpdateTick();
	void UpdateEffects();

	void RedrawEntities(Vector2_I chunkCoords);

	void DoBehaviour(Entity* ent, std::shared_ptr<Chunk> chunkInUse);
	void RunTasks(Entity* ent, std::shared_ptr<Chunk> chunkInUse);

	void AttemptAttack(Entity* ent);

	bool PlayerNearby(Vector2_I coords);
	void LoadMessages();
	void LoadGlyphs();

	void SpawnEntity(Entity* curNPC);

	void MovePlayer(int dir);

	void SetTile(Vector2_I tile, int newTile);
	
	Entity* NearEnt();

	bool EnterCave();
	void LoadData();
	std::string GetWalkSound();

	void PlayWalkSound();

	void Restart();
	void CreateWorldFactions();

	std::string GetItemChar(Tile* tile);
	ImVec4 GetItemColor(Tile* tile, float intensity);
	std::string GetTileChar(Tile* tile);
	ImVec4 GetTileCharColor(Tile* tile, float intensity);

	std::string GetTileChar(Vector2_I tile);

	ImVec4 GetTileColor(Tile* tile, float intensity, bool shadows);
	ImVec4 GetTileColor(Vector2_I tile, float intensity, bool shadows);
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
	lerpingTo = urban;

	Console::Log("Loading dialogue...", WARNING, __LINE__);
	LoadMessages();

	Console::Log("Loading glyphs...", WARNING, __LINE__);
	glyphs.LoadGlyphs();

	factions.Create(FACTION_HUMAN);
	factions.Create(FACTION_ZOMBIE);
	factions.Create(FACTION_WILDLIFE);
	factions.Create(FACTION_BANDIT);
	factions.Create("Takers");
}

//Load All NPC dialogue into the game
void GameManager::LoadMessages() {
	std::vector<std::string> messageTypes = { 
		"CALM_FARMER",
		"CALM_WANDERER",
		"HAPPY_WANDERER",
		"ANGRY_WANDERER",
		"AFRAID_WANDERER",
		"BRAVE_WANDERER", 
		"TRUST_WANDERER",
		"UNTRUST_WANDERER"};

	for (uint32_t i = 0; i < messageTypes.size(); i++)
	{
		OpenedData data;
		switch(i){
		case 0:
		case 1:
			ItemReader::GetDataFromFile("dialogue/dialogue_calm.eid", messageTypes[i], &data);
			break;
		case 2:
		case 3:
			ItemReader::GetDataFromFile("dialogue/dialogue_happy.eid", messageTypes[i], &data);
			break;
		case 4:
		case 5:
			ItemReader::GetDataFromFile("dialogue/dialogue_fear.eid", messageTypes[i], &data);
			break;
		case 6:
		case 7:
			ItemReader::GetDataFromFile("dialogue/dialogue_trust.eid", messageTypes[i], &data);
			break;
		}
		Console::Log(messageTypes[i], LOG, __LINE__);
		npcMessages.insert({ messageTypes[i], {} });
		const int messagesSize = data.getInt("size");


		if (messagesSize <= 0 || messageTypes.size() <= 0) {
			Console::Log("'" + messageTypes[i] + "' size is invalid. (" + std::to_string(messagesSize) + ")", ERROR, __LINE__);
			continue;
		}
		else if (messagesSize >= 50 || messageTypes.size() >= 50) {
			Console::Log("'" + messageTypes[i] + "' size is too large. (" + std::to_string(messagesSize) + ")", ERROR, __LINE__);
			continue;
		}

		for (uint32_t x = 1; x <= messagesSize; x++)
		{
			npcMessages[messageTypes[i]].push_back(data.getString(std::to_string(x)));
		}
	}
	Console::Log("Finished loading dialogue!", SUCCESS, __LINE__);
}

void GameManager::Setup(int x, int y, float tick, int seed = -1, int biome = -1, int moisture = -1) {
	BG_DESERT = { 0.15, 0.15, 0 };
	BG_WATER = { 0, 0.1, 0.15 };
	BG_FOREST = { 0, 0.15, 0 };
	BG_TAIGA = { 0, 0.2, 0.15 };
	BG_SWAMP = { 0.1, 0.15, 0.05 };
	sandWalk = { "dat/sounds/movement/sand1.wav","dat/sounds/movement/sand2.wav", "dat/sounds/movement/sand3.wav" };
	grassWalk = { "dat/sounds/movement/grass1.wav","dat/sounds/movement/grass2.wav", "dat/sounds/movement/grass3.wav" };
	rockWalk = { "dat/sounds/movement/rock_walk1.wav","dat/sounds/movement/rock_walk2.wav", "dat/sounds/movement/rock_walk3.wav" };
	mainMap.SetWeather(clear);
	mainMap.ticksUntilWeatherUpdate = Math::RandInt(600, 800);
	mainMap.worldTimeTicks = 1950; // 1pm

	//if(seed == -1) deleteAllFilesInDirectory();
	SetTick(tick);
	AddRecipes();
	mainMap.CreateMap(seed, biome, moisture);

	//Faction, Enemies
	factions.list[FACTION_HUMAN].AddEnemy(FACTION_ZOMBIE);
	factions.list[FACTION_HUMAN].AddEnemy(FACTION_BANDIT);


	factions.list[FACTION_ZOMBIE].AddEnemy(FACTION_HUMAN);
	factions.list[FACTION_ZOMBIE].AddEnemy(FACTION_WILDLIFE);

	factions.list[FACTION_BANDIT].AddEnemy(FACTION_HUMAN);
	factions.list[FACTION_BANDIT].AddEnemy(FACTION_ZOMBIE);
	
	//mainMap.playerLevel = 0;
	Math::PushBackLog(&actionLog, "Press H to open the help menu.");
}

void GameManager::Restart() {
	mainMap.Restart();
}

void GameManager::CreateWorldFactions()
{
	//std::vector<std::string> generatedFactions;
	for (size_t i = 0; i < 3; i++)
	{
		std::string factName = NameGenerator::generateFactionName();

		factions.Create(factName);

		//create faction name
		factions.list[factName].leaderName;

		std::string leaderName = NameGenerator::generateUniqueName();

		//create faction leader
		factions.list[factName].leaderName = leaderName;

		Entity* leaderEnt = new Entity();

		leaderEnt->faction = &factions.list[factName];
		leaderEnt->health = Math::RandInt(20,100);
		leaderEnt->factionLeader = true;
		leaderEnt->damage = Math::RandInt(5, 15);
		leaderEnt->name = leaderName;

		factName.erase(std::remove_if(factName.begin(), factName.end(), ::isspace), factName.end());

		std::string specialEntFilePath = (
			"dat/saves/"
			+ mainMap.currentSaveName
			+ "/entities/");

		leaderEnt->SaveToFile(specialEntFilePath);

		delete leaderEnt;
	}
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
	if (ent->smart) {
		ent->UpdateMemories();
		ent->UpdateMood();
	}

	vec2_i oldCoords = ent->coords;
	std::vector<Vector2_I> path;
	bool crossChunk = false;

	if (chunkInUse->globalChunkCoord == mainMap.c_glCoords) {
		path = mainMap.GetLine(ent->coords, mPlayer.coords, 20);
	}
	else {
		Vector2_I offset = mainMap.c_glCoords - chunkInUse->globalChunkCoord;
		offset *= 30;
		Vector2_I newPlayerCoords = mPlayer.coords + offset;
		path = mainMap.GetLine(ent->coords, newPlayerCoords, 20);
		crossChunk = true;
	}

	std::vector<Vector2_I> entPath;
	Entity* tempTarget = nullptr;
	bool moved = false;
	ent->tempViewDistance = isDark() ? ent->viewDistance * 2 : ent->viewDistance;

	if (chunkInUse->globalChunkCoord == mainMap.c_glCoords) {
		if (ent->idleSounds.size() > 0) {
			if (Math::RandInt(0, 25) == 1) {
				Audio::Play(ent->RandomIdleSound());
			}
		}
	}

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
	case Tasks:
		RunTasks(ent, chunkInUse);
		break;
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
			if (chunkInUse->globalChunkCoord == mainMap.c_glCoords) {
				if (ent->name == "Zombie" && Math::RandInt(0, 20) == 1) {
					Audio::Play("dat/sounds/zombie_angry.mp3");
				}
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
				isEnemies = factions.AreEnemies(tempTarget->faction->name, ent->faction->name);
			}
			//if the player is targeted
			else if (ent->targetingPlayer) {
				if (mPlayer.coveredIn == guts && ent->name == "Zombie") { ent->targetingPlayer = false; }
				std::vector<Vector2_I> pathToPlayer;
				if (crossChunk) {
					pathToPlayer = path;
				}
				else {
					//pathfind to player with astar if they are chasing
					pathToPlayer = AStar(chunkInUse, ent->coords, mPlayer.coords);
				}

				if (pathToPlayer.size() > 1) {
					if (mainMap.GetTileFromThisOrNeighbor(pathToPlayer[1], chunkInUse->globalChunkCoord)->walkable) {
						ent->coords = pathToPlayer[1];
						moved = true;
					}
					else {
						moved = false;
					}
				}
				else {
					if(pathToPlayer.size() == 1)
					{
						if (mainMap.GetTileFromThisOrNeighbor(pathToPlayer[0], chunkInUse->globalChunkCoord)->walkable) {
							ent->coords = pathToPlayer[0];
							moved = true;
						}
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
	}

	
	if (ent->coords == mPlayer.coords || mainMap.GetTileFromThisOrNeighbor(ent->coords)->walkable == false) {
		ent->coords = oldCoords;
		return;
	}

	tile = mainMap.GetTileFromThisOrNeighbor(ent->coords, chunkInUse->globalChunkCoord);

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

	if (moved && chunkInUse->globalChunkCoord == mainMap.c_glCoords) {
		auto tileCheck = chunkInUse->GetTileAtCoords(ent->coords);
		if (tileCheck && tileCheck->itemName == "BEAR_TRAP") {
			Audio::Play("dat/sounds/bear_trap.mp3");
			ent->health -= 35;
			tileCheck->itemName = "BEAR_TRAP_C";
		}
		Audio::Play(quietWalkSounds[Math::RandInt(0, 2)]);
	}
}

void GameManager::RunTasks(Entity* ent, std::shared_ptr<Chunk> chunkInUse) {

	//read through the list of tasks it can do
	for (auto& t : ent->taskList)
	{
		//if its not being fulfilled, check if it can be
		if (t.currentlyFulfilling == false) {
			switch (t.task) {
			case TaskType::collectItem:
				for (const auto& checkTile : t.taskArea)
				{
					Tile& curTile = *mainMap.GetTileFromThisOrNeighbor(checkTile);
					if (curTile.hasItem && curTile.itemName == t.taskModifierString)
					{
						t.currentlyFulfilling = true;
						t.currentObjective = checkTile;

						//if it can, add it to the queue
						ent->taskQueue.emplace(&t);
						break;
					}
				}
				break;
			case TaskType::dropItem:
				for (const auto& checkTile : t.taskArea)
				{
					Tile& curTile = *mainMap.GetTileFromThisOrNeighbor(checkTile);
					if (curTile.id == t.taskModifierID && !curTile.hasItem)
					{
						t.currentlyFulfilling = true;
						t.currentObjective = checkTile;

						//if it can, add it to the queue
						ent->taskQueue.emplace(&t);
						break;
					}
				}
				break;
			case TaskType::smoke:
				if (Math::RandInt(0, 35) == 1) {
					t.currentlyFulfilling = true;
					t.currentObjective = zero;
					t.taskTimer = 20;
					ent->taskQueue.emplace(&t);
				}
				break;
			}
		}
	}


	//Now run through the task queue and do tasks
	for (auto& t : ent->taskQueue)
	{
		//If they have something to do with their task
		if (t->currentlyFulfilling) {
			//walk towards the objective
			std::vector<Vector2_I> pathToObj = mainMap.GetLine(ent->coords, t->currentObjective, 10);
			if (pathToObj.size() > 2) {
				ent->coords = pathToObj[1];

				//if they are busy, break out of the loop
				break;
			}
			//reached tile
			else {
				Tile& objTile = *mainMap.GetTileFromThisOrNeighbor(t->currentObjective);
				switch (t->task) {
				case TaskType::collectItem:

					ent->inv.push_back(Items::GetItem(objTile.itemName));
					objTile.hasItem = false;
					objTile.itemName = "nthng";
					break;

				case TaskType::dropItem:

					objTile.hasItem = true;
					objTile.itemName = t->taskModifierString;
					objTile.ticksPassed = 0;
					break;
				}
				t->currentlyFulfilling = false;
				t->currentObjective = zero;
				ent->taskQueue.erase(t);
			}
			/*switch (t->task) {
			case TaskType::smoke:
				if (Math::RandInt(0, 5) == 2) {
					mainMap.effectLayer.localCoords[ent->coords.x][ent->coords.y] = 1;
				}
				t->taskTimer -= 1;

				if (t->taskTimer <= 0) {
					t->currentlyFulfilling = false;
					t->currentObjective = zero;
					t->taskTimer = 0;
					ent->taskQueue.erase(t);
					mainMap.GetTileFromThisOrNeighbor(ent->coords)->itemName = "CIGARETTE";
				}
				break;
			}
			*/
		}
	}
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
	if (selTile->entity != nullptr && selTile->entity->canTalk && selTile->entity->health > 0) {
		selTile->entity->SelectMessage(npcMessages);
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

	if (mPlayer.stunned) { return; }
	if (pInv.Encumbered()) { 
		mPlayer.stunned = true;
		Utilities::SetVarInSeconds("stun", &mPlayer.stunned, 0.5f);
	}

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

	if (mainMap.TileAtPos(mPlayer.coords)->itemName == "BEAR_TRAP") {
		mPlayer.TakeDamage(pierceDamage, 35);
		Audio::Play("dat/sounds/bear_trap.mp3");
		mainMap.TileAtPos(mPlayer.coords)->itemName = "BEAR_TRAP_C";
	}

	//moving up stairs
	if (mainMap.TileAtPos(mPlayer.coords)->id == 25) {
		mainMap.playerLevel += 1;
		Audio::StopLoop("ambient_day");

		OpenedData atticDat;

		ItemReader::GetDataFromFile("structures/houses.eid", "HOUSE_ATTIC", &atticDat);
		mainMap.PlaceStructure(mainMap.c_glCoords, atticDat.getString("tiles"), { atticDat.getInt("width"), atticDat.getInt("height") }, { 15, 14 });
	}
	//moving down stairs
	else if (mainMap.TileAtPos(mPlayer.coords)->id == 33) {
		Audio::PlayLoop("dat/sounds/music/ambient12.wav", "ambient_day");
		Audio::SetVolumeLoop(musicvolume, "ambient_day");
		mainMap.playerLevel -= 1;
	}
	/*if (mainMap.TileAtPos(mPlayer.coords)->id == 19) {
		if (EnterCave()) {
			freeView = false;
			//Audio::LerpMusic("ambient_day", "dat/sounds/wet-pizza-rat.mp3", "ambient_cave");
			Audio::StopLoop("ambient_day");
			Audio::PlayLoop("dat/sounds/music/wet-pizza-rat.mp3", "ambient_cave");
			Math::PushBackLog(&actionLog, "You enter a dark cave underground.");
		}
	}*/

	if (mainMap.TileAtPos(mPlayer.coords)->id == 23) {
		if (!mPlayer.indoors) {
			mPlayer.indoors = true;
			Math::PushBackLog(&actionLog, "You enter a building.");
			//Utilities::Lerp("fontsize", &reg_font_size, 32.f, 1.f);
		}
	}
	else {
		if (mPlayer.indoors) {
			mPlayer.indoors = false;
			Math::PushBackLog(&actionLog, "You exit the building.");
			//Utilities::Lerp("fontsize", &reg_font_size, 16.f, 1.f);
		}
	}

	biome curBiome = mainMap.GetBiome(mPlayer.coords);
	if (!isDark()) {
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
		else if (curBiome == forest) {
			if (lerpingTo != forest) {
				Utilities::Lerp("bgColor", &bgColor, BG_FOREST, 1.f);
				mainBGcolor = BG_FOREST;
				currentBiome = lerpingTo = forest;
			}
		}
		else if (curBiome == ocean) {
			if (lerpingTo != ocean) {
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
	}
	else {
		mainBGcolor = { 0,0,0 };
	}

	//relic of the past
	//if (mainMap.NearNPC(player)) { Math::PushBackLog(&actionLog, "Howdy!"); }
}


void GameManager::UpdateEffects() {

	//1 is smoke, 2 is rain, 20 is lightning
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
			if (mainMap.effectLayer.localCoords[i][j] >= 20) {
				tempMap[i][j] = mainMap.effectLayer.localCoords[i][j] + 1;
			}
			if (mainMap.effectLayer.localCoords[i][j] == 24) {
				tempMap[i][j] = 0;
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

	pInv.ResetItemNames();

	//Each tick
	if (tickCount >= tickRate)
	{
		tickCount = 0;
		float curTime = glfwGetTime();
		mainMap.worldTimeTicks++;

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


		//PlayerStatus (bleeding, sickness, hunger, thirst, drying off)
		mPlayer.CheckStatus();

		//if player stands in liquid, soak em.
		Liquid tileLiquid = mainMap.CurrentChunk()->GetTileAtCoords(mPlayer.coords)->liquid;
		if (tileLiquid != nothing && tileLiquid != fire) {
			if (!pInv.Waterproof(boots)) {
				mPlayer.CoverIn(tileLiquid, Math::RandInt(10, 30));
			}
		}

		if (mainMap.currentWeather == rainy || mainMap.currentWeather == thunder) {
			if (Math::RandInt(1, 7) == 2)  //random check
			{									//									id 13 is indoors
				if (!pInv.Waterproof(shirt) && !mainMap.TileAtPos(mPlayer.coords)->hasRoof) {
					mPlayer.CoverIn(water, 15); //cover the player in it
				}
			}
		}

		if (mainMap.currentWeather == thunder) {
			if (Math::RandInt(0, 65) == 15) {
				Vector2_I lightningTile = { Math::RandInt(2, 29), Math::RandInt(2, 29) };
				mainMap.DrawLine(mainMap.GetLine({ 0, Math::RandInt(0,29) }, lightningTile, 20), 20);

				bgColor = { 1,1,1 };
				Utilities::Lerp("BackgroundColor", &bgColor, mainBGcolor, 0.3f);
				Tile* struckTile = mainMap.GetTileFromThisOrNeighbor(lightningTile);

				if (struckTile->liquid != water) {
					struckTile->SetLiquid(fire);
				}
				if (struckTile->entity != nullptr) {
					struckTile->entity->health -= 100;
				}
				if (!struckTile->hasItem && Math::RandInt(1, 45)) {
					struckTile->hasItem = true;
					struckTile->itemName = "FLAT_MUSHROOM";
				}

				Audio::Play("dat/sounds/lightning.mp3");
			}
		}

		if (mPlayer.coveredIn == water || mPlayer.bodyTemp > 98.5f) {
			mPlayer.bodyTemp -= 0.025f;
		}

		else if (!isDark()) {
			if (mPlayer.bodyTemp < 98.5f) mPlayer.bodyTemp += 0.025f;
		}

		if (mPlayer.bodyTemp < 95.f) {
			if (mPlayer.sicknessLevel <= 0) {
				mPlayer.sicknessLevel = 1;
			}
		}

		if (mPlayer.bodyTemp < 94.f) {
			mPlayer.bodyTemp = 94.f;
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
		if (mainMap.worldTimeTicks > 3600) { mainMap.worldTimeTicks = 0; }

		if (mainMap.playerLevel == 0) {
			//
			if (mainMap.worldTimeTicks >= 2850 || mainMap.worldTimeTicks <= 900) {
				time = night;
				if (!startedMusicNight) {
					Audio::StopLoop("ambient_day");
					Audio::PlayLoop("dat/sounds/crickets.mp3", "crickets");
					Audio::PlayLoop("dat/sounds/music/night_zombos.wav", "night_music");
					Audio::SetVolumeLoop(musicvolume, "night_music");
					Audio::SetVolumeLoop(sfxvolume, "crickets");
					startedMusicNight = true;
					Utilities::Lerp("background", &bgColor, {0,0,0}, 2.f);
				}

				if (forwardTime) { darkTime = std::min(10.f, darkTime + 0.05f); }
				else { darkTime = std::max(1.f, darkTime - 0.05f); }
				if (mainMap.worldTimeTicks > 600 && mainMap.worldTimeTicks <= 750) { forwardTime = false; }
			}
			else if (mainMap.worldTimeTicks == 900) {
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
		if (mainMap.playerLevel == 0) {
			T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x + 1,  mainMap.c_glCoords.y });
			T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x - 1,  mainMap.c_glCoords.y });
			T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x,  mainMap.c_glCoords.y + 1 });
			T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x,  mainMap.c_glCoords.y - 1 });
		}
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

/*bool GameManager::EnterCave() {
	if (mainMap.playerLevel == 0)
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
		mainMap.playerLevel = -1;
		return true;
	}
	return false;
}*/

std::string GameManager::GetItemChar(Tile* tile) {
	if (!tile->hasItem || item_icons.count(tile->itemName) == 0) {
		return "!";
	}
	return glyphs.getGlyph(item_icons[tile->itemName]);
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
			return glyphs.getGlyph("ent_zombie");
		case ID_CHICKEN:
			return glyphs.getGlyph("ent_chicken");
		case ID_HUMAN:
			if (tile->entity->faction->name == FACTION_BANDIT) {
				return glyphs.getGlyph("ent_bandit");
			}
			return glyphs.getGlyph("ent_human");
		case ID_FROG:
			return glyphs.getGlyph("ent_frog");
		case ID_CAT:
			return glyphs.getGlyph("ent_cat");
		case ID_COW:
			return glyphs.getGlyph("ent_cow");
		case ID_FINDER:
			return glyphs.getGlyph("ent_finder");
		case ID_TAKER:
			return glyphs.getGlyph("ent_taker");
		}
	}
	if (tile->liquid == water && tile->liquidTime == -1) {
		return glyphs.getGlyph("vfx_double_tilde");
	}
	return glyphs.getGlyph(tile->tileSprite);
}

std::string GameManager::GetTileChar(Vector2_I tile) {
	return GameManager::GetTileChar(&mainMap.CurrentChunk()->localCoords[tile.x][tile.y]);
}

ImVec4 GameManager::GetTileCharColor(Tile* tile, float intensity = -1.f) {
	if (tile->entity != nullptr) {
		return { 1, 0, 0, 1 };
	}
	if (intensity == -1.f) { intensity = tile->brightness; }
	if (tile->liquid == fire) { return Cosmetic::FireColor(); }
	vec3 color = Items::GetItemColor(tile->itemName);
	color.x /= (darkTime * intensity);
	color.y /= (darkTime * intensity);
	color.z /= (darkTime * intensity);

	return { color.x, color.y, color.z, 1 };

}

std::string GameManager::GetWalkSound(){

	if(mainMap.playerLevel == -1) return rockWalk[Math::RandInt(0, 2)];

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

void GameManager::PlayWalkSound() {
	Audio::Play(mainMap.TileAtPos(mPlayer.coords)->GetWalkSound());
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

ImVec4 GameManager::GetTileColor(Tile* tile, float intensity, bool shadows) {
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
			color = { 1,1,1,1 };
			goto dimming;
			break;
		case ID_COW:
			color = { 0.57,0.3,0,1 };
			goto dimming;
			break;
		}
	}
	if (tile->liquid != nothing) {
		color = Cosmetic::CoveredColor(tile->liquid);
		goto dimming;
	}

	//check if its burnt
	if (tile->burningFor > 0) {
		color = { 0.45, 0.45, 0.45, 1 };
		goto dimming;
	}

	color = { tile->tileColor.x, tile->tileColor.y, tile->tileColor.z, 1 };
dimming:
	//if its night time
	if (isDark()) {
		if ((darkTime >= 10.f && intensity >= 1.f) || (mainMap.playerLevel == -1 && intensity >= 1.f)) {
			if(dormantMoon) color = { 0.1f,0.1f,0.1f,1 };
			else color = { 0.05f,0.05f,0.05f,1 };
		}
		else {
			color.x /= (darkTime * intensity);
			color.y /= (darkTime * intensity);
			color.z /= (darkTime * intensity);
		}
	}
	else if (shadows && mainMap.GetChunkAtCoords(tile->g_coords) != nullptr) {
		if (mainMap.GetChunkAtCoords(tile->g_coords)->shadows.contains(tile->coords) && !tile->double_size) {
			color.x *= 0.5f;
			color.y *= 0.5f;
			color.z *= 0.5f;
		}
	}

	return color;
}


ImVec4 GameManager::GetTileColor(Vector2_I tile, float intensity, bool shadows) {
	return GameManager::GetTileColor(&mainMap.CurrentChunk()->localCoords[tile.x][tile.y], intensity, shadows);
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
	Math::PushFrontLog(&game->consoleLog, input);
	if (tokens[0] == "debug") {
		if (tokens.size() < 2) { return; }
		if (tokens[1] == "pathfind")
		{
			auto path = AStar(game->mainMap.CurrentChunk(), game->mPlayer.coords, {0,0});
			for (auto& tl : path)
			{
				game->mainMap.GetTileFromThisOrNeighbor(tl)->SetLiquid(strange_liquid);
			}
			Math::PushFrontLog(&game->consoleLog, "- Pathfinding to (0,0).");
			return;
		}

		if (tokens[1] == "name")
		{
			Math::PushFrontLog(&game->consoleLog, NameGenerator::generateUniqueName());
			return;
		}
	}

	if (tokens[0] == "list") {
		if (tokens.size() < 2) { return; }

		if (tokens[1] == "factions") {
			for (auto const& factnames : factions.list) {
				Math::PushFrontLog(&game->consoleLog, "- " + factnames.first);
			}
		}

		if (tokens.size() < 3) { return; }

		//Check if faction exists first
		if (!factions.DoesExist(tokens[2])) {
			Math::PushFrontLog(&game->consoleLog, "- Faction '" + tokens[2] + "' does not exist.");
			return;
		}
		
		if (tokens[1] == "enemies") {
			//list all enemies
			if (factions.list[tokens[2]].enemies.size() <= 0) {
				Math::PushFrontLog(&game->consoleLog, "- No enemies");

			}
			for (auto const& enemie : factions.list[tokens[2]].enemies) {
				Math::PushFrontLog(&game->consoleLog, "- " + enemie);
			}
		}
		else if (tokens[1] == "allies") {
			//list all ally
			if (factions.list[tokens[2]].allies.size() <= 0) {
				Math::PushFrontLog(&game->consoleLog, "- No allies");

			}
			for (auto const& ally : factions.list[tokens[2]].allies) {
				Math::PushFrontLog(&game->consoleLog, "- " + ally);
			}
		}
	}

	if (tokens[0] == "spawn") {
		//if not enough or out of bounds
		if (tokens.size() < 4) { return; }
		if (stoi(tokens[2]) > 29 || 
			stoi(tokens[2]) < 0  ||
			stoi(tokens[3]) > 29 || 
			stoi(tokens[3]) < 0  ) { return; }

		//try and get the spawn coords
		Vector2_I spawnCoords;
		try {
			spawnCoords = { stoi(tokens[2]), stoi(tokens[3]) };
		}
		catch (std::exception e) {
			Math::PushFrontLog(&game->consoleLog, "- Invalid spawn coordinates.");
			return;
		}

		if (tokens[1] == "human") {
			game->mainMap.SpawnHumanInCurrent(spawnCoords, Behaviour::Protective, FACTION_HUMAN);
			Math::PushFrontLog(&game->consoleLog, "- Spawned random human.");
		}

	}
	if (tokens[0] == "give") {
		if (tokens.size() < 3) { return; }
		if (Items::list.count(tokens[1]) == 0) { 
			Math::PushFrontLog(&game->consoleLog, "- Item \"" + tokens[1] + "\" cannot be found.");
			return; 
		}
		Math::PushFrontLog(&game->consoleLog, "- Added " + tokens[2] + " " + tokens[1]);
		game->pInv.AddItemByID(tokens[1], stoi(tokens[2]));
	}
	if (tokens[0] == "say") {
		if (tokens.size() < 3) { return; }
		if (game->npcMessages.count(tokens[1]) == 0) {
			Math::PushFrontLog(&game->consoleLog, "- No dialogue list with that name.");
			return;
		}
		Math::PushFrontLog(&game->consoleLog, "- " + game->npcMessages[tokens[1]][stoi(tokens[2])]);
	}
	else if (tokens[0] == "god" || tokens[0] == "buddha") {
		if (tokens.size() < 2) { return; }
		if (tokens[1] == "off") 
		{
			Math::PushFrontLog(&game->consoleLog, "- Buddha/God Mode Off");
			game->mPlayer.damageMode = 0; }

		else if (tokens[0] == "buddha") 
		{
			Math::PushFrontLog(&game->consoleLog, "- Buddha Mode On");
			game->mPlayer.damageMode = 1; }

		else if (tokens[0] == "god") 
		{
			Math::PushFrontLog(&game->consoleLog, "- God Mode On");
			game->mPlayer.damageMode = 2; }
	}
	else if (tokens[0] == "set")
	{
		if (tokens.size() < 3) { return; }
		if (tokens[1] == "health") {
			game->mPlayer.health = stoi(tokens[2]);
		}
		else if (tokens[1] == "thirst") {
			game->mPlayer.thirst = stoi(tokens[2]);
		}
		else if (tokens[1] == "hunger") {
			game->mPlayer.hunger = stoi(tokens[2]);
		}
		else if (tokens[1] == "sickness") {
			game->mPlayer.sicknessLevel = stoi(tokens[2]);
		}
		else if (tokens[1] == "bleeding") {
			game->mPlayer.bleedingLevel = stoi(tokens[2]);
		}
		else if (tokens[1] == "tickrate") {
			game->SetTick(stof(tokens[2]));
		}
		else if (tokens[1] == "weather") {
			if(tokens[2] == "clear") {
				game->mainMap.SetWeather(clear);
			}else if (tokens[2] == "rain") {
				game->mainMap.SetWeather(rainy);
			}else if (tokens[2] == "thunder") {
				game->mainMap.SetWeather(thunder);
			}
			Math::PushFrontLog(&game->consoleLog, "- Weather changed");
		}
		else if (tokens[1] == "time") {
			game->mainMap.worldTimeTicks = stoi(tokens[2]);
			Math::PushFrontLog(&game->consoleLog, "- Time changed");
		}
		else if (tokens[1] == "playerLevel") {
			game->mainMap.playerLevel = stoi(tokens[2]);
			Math::PushFrontLog(&game->consoleLog, "- Player level changed");
		}
	}
	else if (tokens[0] == "help") {
		std::string helpstring =
						"\n ~give {item name} {amount} - gives item\n";
		helpstring += " ~set weather {clear / rain / thunder} - sets the weather\n";
		helpstring += " ~set time {time in ticks} - sets the time to a specific tick\n";
		helpstring += " ~set tickrate {float} - sets time between ticks in seconds (default is 0.5)\n";
		helpstring += " ~set {health / hunger / thirst} {number} - sets attribute\n";
		helpstring += " ~set {bleeding / sickness} {number} - sets attribute (0 - 3)\n";
		helpstring += " ~set playerLevel {-1/0/1} - sets the chunk height (underground, ground, upstairs) \n";
		helpstring += " ~spawn human x y - spawns random human at x,y\n";
		helpstring += " ~buddha {on / off} - toggles buddha mode\n";
		helpstring += " ~god {on / off} - toggles god mode\n";
		helpstring += " ~debug {pathfind} - does debug things\n";
		helpstring += " ~list {factions/enemies/allies} - shows factions/enemies of faction/allies\n";
		helpstring += " ~help - this\n";

		Math::PushFrontLog(&game->consoleLog, helpstring);

		//Console::Log(helpstring, text::white, -1);
	}
	else if (input == "bring him forth") {
		Math::PushFrontLog(&game->actionLog, "He shall arrive.");
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