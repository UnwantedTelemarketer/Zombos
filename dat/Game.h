#ifndef GAME_H
#define GAME_H
#include "map.h"
#include "cosmetic.h"
#include "items.h"
#include <thread>

using namespace antibox;
class GameManager {
private:
	float tickRate;
	float effectTickRate;
	double tickCount;
	double effectTickCount;
	bool forwardTime = true;
	std::vector<std::string> missMessages = { "blank", "You swing at nothing and almost fall over.", "You miss.", "You don't hit anything." };
	std::vector<std::string> npcMessages;
public:
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


	std::vector<std::string> recipeNames;
	float worldTime = 6.f;
	float darkTime = 1.f;
	bool paused = false;

	double GetTick() { return (tickRate - tickCount); }
	float TickRate() { return tickRate; }
	void SetTick(float secs) { tickRate = secs * 1000; effectTickRate = tickRate / 4; }

	void AddRecipes();
	void Setup(int x, int y, float tick, int seed, int biome);

	void UpdateEntities(Vector2_I chunkCoords);
	void UpdateTick();
	void UpdateEffects();

	void RedrawEntities(Vector2_I chunkCoords);

	void DoBehaviour(Entity* ent);

	void AttemptAttack(Entity* ent);

	bool PlayerNearby(Vector2_I coords);

	void SpawnEntity(Entity* curNPC);

	void MovePlayer(int dir);

	void SetTile(Vector2_I tile, int newTile);
	
	Entity* NearEnt();

	bool EnterCave();

	std::string GetItemChar(Tile tile);
	std::string GetTileChar(Tile tile);

	std::string GetTileChar(Vector2_I tile);

	ImVec4 GetTileColor(Tile tile, float intensity);
	ImVec4 GetTileColor(Vector2_I tile, float intensity);
	ImVec4 GetPlayerColor();
};


static void T_UpdateChunk(GameManager* gm, Vector2_I coords)
{
	if (coords.x >= MAP_WIDTH || coords.y >= MAP_HEIGHT || coords.x < 0 || coords.y < 0) { return; }
	gm->UpdateEntities(coords);
	gm->mainMap.UpdateTiles(coords);
}


void GameManager::Setup(int x, int y, float tick, int seed = -1, int biome = -1) {
	SetTick(tick);
	mPlayer.coords.x = x;
	mPlayer.coords.y = y;
	AddRecipes();
	recipeNames = Crafter.getRecipeNames();
	mainMap.CreateMap(seed, biome);
	//Faction, Enemies
	factionEnemies = {
		{Human, {Zombie}},
		{Zombie, {Human, Wildlife}},
		{Wildlife, {}}
	};
	OpenedData data;
	ItemReader::GetDataFromFile("dialogue.eid", "RANDOM", &data);
	for (size_t i = 1; i < 6; i++)
	{
		npcMessages.push_back(data.getString(std::to_string(i)));
	}

	OpenedData tileData;
	ItemReader::GetDataFromFile("tiles.eid", "TILES", &tileData);

	for (auto const& x : tileData.tokens){
		tile_icons.insert({ stoi(tileData.getArray(x.first)[1]) , tileData.getArray(x.first)[0] });
	}

	OpenedData itemData;
	ItemReader::GetDataFromFile("item_data.eid", "ITEMS", &itemData);

	for (auto const& x : itemData.tokens) {
		item_icons.insert({ itemData.getArray(x.first)[0] , itemData.getArray(x.first)[1] });
	}

	

	mainMap.containers.insert({ {250, 250, 5, 5}, {{250, 250},{5, 5}, {}} });
	mainMap.isUnderground = false;
	Math::PushBackLog(&actionLog, "Welcome to Zombos! Press H to open the help menu.");
	Audio::Play("dat/sounds/ambient12.wav");
}

void GameManager::AddRecipes() {

	Crafter.addRecipe("CAMPFIRE", { {"STICK", 3}, {"ROCK", 2} });
	Crafter.addRecipe("CANTEEN", { {"SCRAP", 3} });
	Crafter.addRecipe("BANDAGE", { {"GRASS", 3}});
	Crafter.addRecipe("ROPE", { {"GRASS", 2} });
	Crafter.addRecipe("HINGE", { {"SCRAP", 3} });
	Crafter.addRecipe("WOOD_PLANK", { {"STICK", 6}, {"ROPE",2} });
	Crafter.addRecipe("KNIFE", { {"STICK", 1}, {"ROPE", 1}, {"SCRAP", 1}});
	Crafter.addRecipe("CHEST", { {"WOOD_PLANK", 3}, {"HINGE", 2} });
}

void GameManager::DoBehaviour(Entity* ent)
{
	std::vector<Vector2_I> path = mainMap.GetLine(ent->coords, mPlayer.coords, 10);
	std::vector<Vector2_I> entPath;
	Entity* tempTarget = nullptr;

	for (int i = 0; i < mainMap.CurrentChunk()->entities.size(); i++) //loop through every entity
	{
		Entity* curEnt = mainMap.CurrentChunk()->entities[i]; 

		if (curEnt == ent || curEnt->health <= 0) { continue; } //check that we arent looking at the same entity twice

		std::vector<Vector2_I> curPath = mainMap.GetLine(ent->coords, curEnt->coords, 10); //get line to other ent

		if (curPath.size() < entPath.size() || entPath.size() == 0) { //check that its long enough and target them
			entPath = curPath;
			tempTarget = mainMap.CurrentChunk()->entities[i];
		}
	}



	if (entPath.size() < ent->viewDistance) { //if theyre within view distance, target them
		ent->target = tempTarget;
	}

	//mainMap.DrawLine(entPath); //draw the path between entities (for debug)

	int dir = Math::RandInt(1, 10);
	vec2_i oldCoords = ent->coords;

	Tile* tile = mainMap.CurrentChunk()->GetTileAtCoords(ent->coords);

	switch (ent->b) //check the entities behaviour
	{
	case Aggressive:
		//check if player is near
		/*if (path.size() < ent->viewDistance) {
			ent->targetingPlayer = true;
		}
		else {
			ent->targetingPlayer = false;
		}*/
	case Protective:
		//make sure theyre above health and nearby
		if (ent->health > 5 && ent->targeting()) {

			int isEnemies = std::count(factionEnemies[ent->faction].begin(), factionEnemies[ent->faction].end(), ent->target->faction);

			//if the player is targeted
			if (ent->targetingPlayer) {
				ent->coords = path[1];
				break;
			}
			//otherwise, if they are enemies go towards them
			else if (entPath.size() > 2 && isEnemies != 0) {
				ent->coords = entPath[1];
				break;
			}
			//if theyre close enough, attack them
			else if (entPath.size() <= 2) {
				mainMap.AttackEntity(ent->target, ent->damage, &actionLog);
				if (ent->target->health <= 0) {
					ent->target = nullptr;
				}
			}
		}
		//if nothing else, drop the target
		else {
			ent->target = nullptr;
		}

	case Wander:
		//wander around, unless they can talk
		if (PlayerNearby(oldCoords)) {
			ent->talking = true;
			break;
		}
		else {
			ent->talking = false;
			ent->message = npcMessages[Math::RandInt(0, 4)];
		}

		//run out of liquid
		if (tile->liquid == water) {
			ent->coords.x--;
			break;
		}
		else {
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

	if (ent->coords == mPlayer.coords) {
		ent->coords = oldCoords;
		return;
	}
	//cover entities in liquid if they step in it
	if (tile->liquid != nothing) {
		ent->coveredIn = tile->liquid;
		ent->ticksUntilDry = Math::RandInt(10, 30);
	}
	else if(ent->coveredIn != nothing) {
		tile->liquid = ent->coveredIn;
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

void GameManager::SpawnEntity(Entity* ent) {
	ent->health = Math::RandNum(100);
	ent->coords.x = 10;
	ent->coords.y = 15;
	//if(curNPC->name == "") curNPC->name = Math::RandString(possibleNames);
	mainMap.CurrentChunk()->entities.push_back(ent);
	ent->index = mainMap.CurrentChunk()->entities.size() - 1;
}

void GameManager::MovePlayer(int dir) {
	switch (dir) {
	case 1:
		mainMap.MovePlayer(mPlayer.coords.x - 1, mPlayer.coords.y, &mPlayer, &actionLog);
		break;
	case 2:
		mainMap.MovePlayer(mPlayer.coords.x + 1, mPlayer.coords.y, &mPlayer, &actionLog);
		break;
	case 3:
		mainMap.MovePlayer(mPlayer.coords.x, mPlayer.coords.y - 1, &mPlayer, &actionLog);
		break;
	case 4:
		mainMap.MovePlayer(mPlayer.coords.x, mPlayer.coords.y + 1, &mPlayer, &actionLog);
		break;
	default:
		break;
	}
	//if (mainMap.NearNPC(player)) { Math::PushBackLog(&actionLog, "Howdy!"); }
}

void GameManager::UpdateEffects() {
	int tempMap[30][30]{};
	for (size_t i = 0; i < CHUNK_HEIGHT; i++)
	{
		for (size_t j = 0; j < CHUNK_WIDTH; j++)
		{
			if (mainMap.effectLayer.localCoords[i][j] == 1) {
				int newJ = j + Math::RandInt(0, 2);
				int newI = i - Math::RandInt(0, 2);
				if ((newI >= CHUNK_WIDTH || newI < 0) || (newJ >= CHUNK_WIDTH || newJ < 0)) { continue; }
				tempMap[newI][newJ] = 1;
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

	if (tickCount >= tickRate)
	{
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
		if (tileLiquid != nothing) {
			mPlayer.CoverIn(tileLiquid, Math::RandInt(10, 30));
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
		worldTime += 0.1f;
		if (worldTime > 24.f) { worldTime = 0.f; }

		if (!mainMap.isUnderground) {
			if (worldTime >= 20.f || worldTime < 6.f) {
				if (forwardTime) { darkTime = std::min(7.f, darkTime + 0.25f); }
				else { darkTime = std::max(1.f, darkTime - 0.5f); }
				if (worldTime >= 4.f && worldTime <= 5.f) { forwardTime = false; }
			}
			else if (worldTime == 6.f) {
				mainMap.ResetLightValues();
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
		/*std::vector<std::thread> threads;
		for (int i = -1; i < 2; ++i) {
			if (i == 0) { continue; }
			threads.push_back(std::thread(T_UpdateChunk, this, Vector2_I{ mainMap.c_glCoords.x + i,  mainMap.c_glCoords.y }));
		}
		threads.push_back(std::thread(T_UpdateChunk, this, Vector2_I{ mainMap.c_glCoords.x,  mainMap.c_glCoords.y + 1 }));
		threads.push_back(std::thread(T_UpdateChunk, this, Vector2_I{ mainMap.c_glCoords.x,  mainMap.c_glCoords.y - 1}));
		for (auto& th : threads) {
			th.join();
		}*/

		//hardcoded version of updating surrounding chunks
		//T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x + 1,  mainMap.c_glCoords.y });
		//T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x - 1,  mainMap.c_glCoords.y });
		T_UpdateChunk(this, mainMap.c_glCoords);
		//T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x,  mainMap.c_glCoords.y + 1 });
		//T_UpdateChunk(this, Vector2_I{ mainMap.c_glCoords.x,  mainMap.c_glCoords.y - 1 });
	}


}

void GameManager::RedrawEntities(Vector2_I chunkCoords) {
	std::shared_ptr<Chunk> usedChunk = mainMap.GetProperChunk(chunkCoords);
	mainMap.ClearChunkOfEnts(usedChunk);
}

void GameManager::UpdateEntities(Vector2_I chunkCoords) {

	std::shared_ptr<Chunk> usedChunk = mainMap.GetProperChunk(chunkCoords);

	mainMap.ClearLine();
	mainMap.ClearEntities(oldLocations, usedChunk);
	for (int i = 0; i < usedChunk->entities.size(); i++)
	{
		if (i >= usedChunk->entities.size()) { break; }
		if (usedChunk->entities[i]->health <= 0) { continue; }

		if (usedChunk->entities[i]->aggressive && usedChunk->globalChunkCoord == mainMap.CurrentChunk()->globalChunkCoord)
		{
			AttemptAttack(usedChunk->entities[i]);
		}

		if (Math::RandInt(1, 10) >= 4 && !usedChunk->entities[i]->targeting()) { continue; }

		DoBehaviour(usedChunk->entities[i]);
		mainMap.CheckBounds(usedChunk->entities[i], usedChunk);
		
	}

	mainMap.PlaceEntities(usedChunk);

	oldLocations.clear();
	for (int i = 0; i < usedChunk->entities.size(); i++)
	{
		oldLocations.push_back(usedChunk->entities[i]->coords);
	}
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
			Math::PushBackLog(&actionLog, "Zombie bites you for 10 damage!");
			mPlayer.TakeDamage(pierceDamage, 10);
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

std::string GameManager::GetItemChar(Tile tile) {
	return item_icons[tile.itemName];
}
std::string GameManager::GetTileChar(Tile tile) {

	if (tile.entity != nullptr)
	{
		switch (tile.entity->entityID)
		{
		case ID_ZOMBIE:
			return ENT_ZOMBIE;
		case ID_CHICKEN:
			return ENT_CHICKEN;
		case ID_HUMAN:
			return ENT_HUMAN;
		}
	}
	if (tile.liquid != nothing) {
		/*switch (tile.liquid) {
		case fire:
			return TILE_FIRE;
		case water:
			return TILE_LIQUID;
		}*/
		"";
	}
	//if (tile.hasItem) {
		//return item_icons[tile.itemName];
	//}
	if (tile.liquid == water && tile.id == ID_DIRT) {
		return "A";
	}
	return tile_icons[tile.id];
}

std::string GameManager::GetTileChar(Vector2_I tile) {
	return GameManager::GetTileChar(mainMap.CurrentChunk()->localCoords[tile.x][tile.y]);
}

ImVec4 GameManager::GetPlayerColor() {
	Vector3 end_color = { pInv.clothes.x, pInv.clothes.y, pInv.clothes.z};
	int amount_of_colors = 1;
	if (mainMap.CurrentChunk()->localCoords[mPlayer.coords.x + 1][mPlayer.coords.y].double_size)
	{
		end_color += { 0, 0.35, 0 };
		amount_of_colors++;
	}
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

ImVec4 GameManager::GetTileColor(Tile tile, float intensity) {
	ImVec4 color;
	//check for entitites
	if (tile.entity != nullptr)
	{
		if (tile.entity->health <= 0) {
			color = { 1,0,0,1 };
			goto dimming;
		}
		switch (tile.entity->entityID)
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
		}
	}
	//check for liquids
	switch (tile.liquid) {
	case water:
		color = { 0, 0.5, 1, 1 };
		goto dimming;
		break;
	case fire:
		color = Cosmetic::FireColor();
		goto dimming;
		break;
	case blood:
		color = { 1, 0, 0, 1 };
		goto dimming;
		break;
	case nothing:
		break;
	}
	//check if its burnt
	if (tile.burningFor > 0) {
		color = { 0.45, 0.45, 0.45, 1 };
		goto dimming;
	}

	//if (tile.hasItem) {
		//color = { 0.65, 0.45, 0.35, 1 };
		//goto dimming;
	//}

	//regular tile color
	switch (tile.id) {
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
	default:
		color = { 0, 0.65, 0, 1 };
		break;
	}

dimming:
	//if its night time
	if ((worldTime >= 20.f || worldTime < 6.f) || mainMap.isUnderground) {
		color.x /= (darkTime * intensity);
		color.y /= (darkTime * intensity);
		color.z /= (darkTime * intensity);
	}

	return color;
}


ImVec4 GameManager::GetTileColor(Vector2_I tile, float intensity) {
	return GameManager::GetTileColor(mainMap.CurrentChunk()->localCoords[tile.x][tile.y], intensity);
}

class Commands {
public:
	void RunCommand(std::string input, Inventory* inv, Player* p);
};

void Commands::RunCommand(std::string input, Inventory* inv, Player* p) {
	std::vector<std::string> tokens = Tokenizer::getTokens(input);
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "give") {
			if (tokens.size() < 3) { return; }
			if (Items::list.count(tokens[i + 1]) == 0) { 
				LAZY_LOG("Item \"" + tokens[i+1] + "\" cannot be found.")
					return; 
			}
			inv->AddItemFromFile(tokens[i + 1], stoi(tokens[i + 2]));
		}
		else if (tokens[i] == "set")
		{
			if (tokens.size() < 3) { return; }
			if (tokens[i + 1] == "health") {
				p->health = stoi(tokens[i + 2]);
			}
			else if (tokens[i + 1] == "thirst") {
				p->thirst = stoi(tokens[i + 2]);
			}
			else if (tokens[i + 1] == "hunger") {
				p->hunger = stoi(tokens[i + 2]);
			}
		}
		//else if()
	}

}

#endif