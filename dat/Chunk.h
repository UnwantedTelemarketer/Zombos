#include "items.h"
#include <mutex>
#include <vector>
#include <queue>
#include <cmath>
#include <limits>

#define CHUNK_WIDTH 30
#define CHUNK_HEIGHT 30

struct Saved_Chunk {
	Saved_Tile tiles[CHUNK_WIDTH][CHUNK_HEIGHT];
};

struct Chunk {
private:
	std::vector<Container*> containers;
public:
	std::mutex mutex;
	bool beenBuilt = false, hadBuilding = false;
	Vector2_I globalChunkCoord;
	Tile localCoords[CHUNK_WIDTH][CHUNK_HEIGHT];
	std::vector<Entity*> entities;
	std::map<int, Entity*> entsByID;
	std::set<Vector2_I> shadows;

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

	void AddEntity(Entity* ent) {
		entities.push_back(ent);
		entsByID.insert({ent->entityID, ent});
	}

	void RemoveEntity(Entity* ent) {
		int indexOfEnt = -1;

		for (int i = 0; i < entities.size(); i++)
		{
			if (entities[i] == ent) {
				indexOfEnt = i;
				break;
			}
		}
		entities.erase(entities.begin() + indexOfEnt);

		entsByID.erase(ent->entityID);
	}

	void RemoveEntity(int entID) {
		RemoveEntity(entsByID[entID]);
	}

	void LoadChunk(Vector2_I coords, std::string currentSaveName) {

		Saved_Chunk sChunk;
		std::string filePath = ("dat/saves/"
			+ currentSaveName
			+ "/map/"
			+ std::to_string(globalChunkCoord.x)
			+ std::to_string(globalChunkCoord.y)
			+ ".chunk");

		std::string entFilePath = ("dat/saves/"
			+ currentSaveName
			+ "/entities/"
			+ std::to_string(globalChunkCoord.x)
			+ std::to_string(globalChunkCoord.y)
			+ ".eid");

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
		for (int i = 0; i < CHUNK_WIDTH; i++)
		{
			for (int j = 0; j < CHUNK_HEIGHT; j++)
			{
				Tiles::LoadTile(&localCoords[i][j], sChunk.tiles[i][j]);
				localCoords[i][j].coords = { i, j };
				//Correctly load the correct color of tile if its grass
				if (localCoords[i][j].id == 1) 
				{
					switch (localCoords[i][j].biomeID) {
					case (short)swamp:
						localCoords[i][j].mainTileColor = { 0.5f, 0.55f, 0.35f };
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

				if (sChunk.tiles[i][j].colorDiff) {
					localCoords[i][j].tileColor = sChunk.tiles[i][j].tempColor;
				}
			}
		}

		//Check if the entity file exists
		std::ifstream f(entFilePath);
		if (!f.good()) { return; }

		//Lets load the entities back in from the files
		OpenedData entNames;
		ItemReader::GetDataFromFile(entFilePath, "NAMES", &entNames, false);
		
		std::vector<std::string> nameList = entNames.getArray("names");
		//read in the entities
		if (nameList.size() > 0) {
			for (int i = 0; i < nameList.size() - 1; i++) {
				OpenedData tempEnt;
				ItemReader::GetDataFromFile(entFilePath, nameList[i], &tempEnt, false);

				Entity* loadedEntity = new Entity(
					tempEnt.getFloat("health"),
					tempEnt.getString("entDisplayName"),
					0,
					(Behaviour)tempEnt.getInt("behaviour"),
					false,				 //aggressive
					10,					 //view distance
					tempEnt.getInt("damage"),
					{Math::RandInt(2,29), //x coord
					Math::RandInt(2,29)}, //y coord
					factions.GetFaction("Human_W"));

				loadedEntity->uID = stoi(nameList[i]);

				if (!factions.DoesExist(tempEnt.getString("faction"))) {
					factions.Create(tempEnt.getString("faction"));
				}

				loadedEntity->faction = &factions.list[tempEnt.getString("faction")];

				entities.push_back(loadedEntity);
			}
		}
	}

	void SetTileCoords() {
		for (int i = 0; i < CHUNK_WIDTH; i++)
		{
			for (int j = 0; j < CHUNK_HEIGHT; j++)
			{
				localCoords[i][j].g_coords = globalChunkCoord;
			}
		}
	}
	
	void SaveEntities(std::string currentSaveName) {
		std::string specialEntFilePath = (
			"dat/saves/"
			+ currentSaveName
			+ "/entities/");

		OpenedData specialEnts;
		ItemReader::GetDataFromFile(specialEntFilePath + "names.eid", "NAMES", &specialEnts, false);

		std::vector<std::string> entNames = specialEnts.getArray("names");

		//save entities
		if (entities.size() != 0) {
			for (int i = 0; i < entities.size(); i++) {
				//if they have been interacted with, save them special to the side
				if ((entities[i]->feelingTowardsPlayer.overall() != 0 && entities[i]->name != "Human") || entities[i]->factionLeader) {
					entities[i]->SaveToFile(specialEntFilePath);

					//if the name is already in the list, dont add it
					if (std::find(entNames.begin(), entNames.end(), entities[i]->name) == entNames.end())
					{
						entNames.push_back(entities[i]->name);
					}
				}
			}
		}
		SaveData specialSave;
		specialSave.sections.insert({ "NAMES", {} });
		specialSave.sections["NAMES"].lists.insert({ "names", entNames });
		
		ItemReader::SaveDataToFile(specialEntFilePath + "names.eid", specialSave, false);
	}

	void SaveChunk(std::string currentSaveName) {
		std::string filePath = (
			"dat/saves/"
			+ currentSaveName
			+ "/map/"
			+ std::to_string(globalChunkCoord.x)
			+ std::to_string(globalChunkCoord.y)
			+ ".chunk");

		SaveEntities(currentSaveName);

		std::ofstream outFile(filePath, std::ios::binary);

		Saved_Chunk *sChunk = new Saved_Chunk();
		for (size_t i = 0; i < CHUNK_WIDTH; i++)
		{
			for (size_t j = 0; j < CHUNK_HEIGHT; j++)
			{
				if (localCoords[i][j].tileLerpID != "nthng") {
					Utilities::RemoveLerp(localCoords[i][j].tileLerpID);
				}

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
		outFile.close();
	}

	~Chunk() {

		if (containers.size() != 0) {
			for (int i = 0; i < containers.size() - 1; i++) {
				delete containers[i];
				
			}
		}
		if (entities.size() != 0) {
			for (int i = 0; i < entities.size() - 1; i++) {
				delete entities[i];
			}
		}
		
		containers.clear();
		entities.clear();
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
			sChunk->tiles[i][j].itemName = ch.localCoords[i][j].itemName;
			sChunk->tiles[i][j].biomeID = ch.localCoords[i][j].biomeID;
		}
	}
}

struct T_Chunk //we dont need the current chunk copy to have an entity list
{
	int localCoords[CHUNK_WIDTH][CHUNK_HEIGHT];
};



//============ A* Pathfinding ============

std::vector<Vector2_I> AStar(
	std::shared_ptr<Chunk> grid, // true = walkable, false = blocked
	Vector2_I start,
	Vector2_I goal
) {
	const int w = 30;
	const int h = 30;

	auto inBounds = [&](int x, int y) {
		return x >= 0 && y >= 0 && x < w && y < h;
		};

	auto heuristic = [&](Vector2_I a, Vector2_I b) {
		int dx = std::abs(a.x - b.x);
		int dy = std::abs(a.y - b.y);
		return 10 * std::max(dx, dy) + 4 * std::min(dx, dy);
		};

	std::vector<std::vector<int>> gScore(h, std::vector<int>(w, INT_MAX));
	gScore[start.y][start.x] = 0;

	std::vector<std::vector<Vector2_I>> cameFrom(h, std::vector<Vector2_I>(w, { -1,-1 }));

	struct Node {
		Vector2_I p;
		int f;
		bool operator<(const Node& other) const { return f > other.f; }
	};

	std::priority_queue<Node> openSet;
	openSet.push({ start, heuristic(start, goal) });

	const Vector2_I dirs[8] = {
	{ 1,  0},
	{-1,  0},
	{ 0,  1},
	{ 0, -1},
	{ 1,  1},
	{ 1, -1},
	{-1,  1},
	{-1, -1}
	};


	while (!openSet.empty()) {
		Vector2_I current = openSet.top().p;
		openSet.pop();

		if (current == goal) {
			// Reconstruct path
			std::vector<Vector2_I> path;
			Vector2_I p = goal;
			while (!(p == start)) {
				path.push_back(p);
				p = cameFrom[p.y][p.x];
			}
			path.push_back(start);
			std::reverse(path.begin(), path.end());
			return path;
		}

		for (auto d : dirs) {
			int nx = current.x + d.x;
			int ny = current.y + d.y;

			if (!inBounds(nx, ny) || !grid->localCoords[nx][ny].walkable)
				continue;

			// Prevent diagonal corner-cutting
			if (d.x != 0 && d.y != 0) {
				if (!grid->localCoords[current.x][nx].walkable) continue;
				if (!grid->localCoords[nx][current.y].walkable) continue;
			}

			int moveCost = (d.x == 0 || d.y == 0) ? 10 : 14;

			int newCost = gScore[current.y][current.x] +
				((d.x == 0 || d.y == 0) ? 10 : 14);


			if (newCost < gScore[ny][nx]) {
				gScore[ny][nx] = newCost;
				cameFrom[ny][nx] = current;

				int f = newCost + heuristic({ nx, ny }, goal);
				openSet.push({ {nx, ny}, f });
			}
		}

	}

	std::vector<Vector2_I> emptyvec;
	return emptyvec; // No path found
}
