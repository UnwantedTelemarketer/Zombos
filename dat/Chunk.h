#include "items.h"
#include <mutex>

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

				Entity* loadedEntity = new Entity{
					tempEnt.getFloat("health"),
					nameList[i].c_str(),
					tempEnt.getInt("entID"),
					(Behaviour)tempEnt.getInt("behaviour"),
					false,				 //aggressive
					(Faction)tempEnt.getInt("faction"),
					10,					 //view distance
					tempEnt.getInt("damage"),
					true,				 //can talk
					Math::RandInt(2,29), //x coord
					Math::RandInt(2,29), //y coord
					true };

				entities.push_back(loadedEntity);
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
				if (entities[i]->feelingTowardsPlayer.overall() != 0 && entities[i]->name != "Human") {
					SaveData specEntDat;
					specEntDat.sections.insert({ entities[i]->name, {} });

					specEntDat.addFloat(entities[i]->name, "health", entities[i]->health);
					specEntDat.addInt(entities[i]->name, "behaviour", entities[i]->b);
					specEntDat.addInt(entities[i]->name, "faction", entities[i]->faction);
					specEntDat.addInt(entities[i]->name, "damage", entities[i]->damage);
					specEntDat.addFloat(entities[i]->name, "happy", entities[i]->feelingTowardsPlayer.happy);
					specEntDat.addFloat(entities[i]->name, "fear", entities[i]->feelingTowardsPlayer.fear);
					specEntDat.addFloat(entities[i]->name, "trust", entities[i]->feelingTowardsPlayer.trust);

					std::vector<std::string> memories;
					for (size_t i = 0; i < entities[i]->memories.size(); i++)
					{
						memories.push_back(std::to_string(entities[i]->memories[i].who));
						memories.push_back(std::to_string((int)(entities[i]->memories[i].type)));
						memories.push_back(entities[i]->memories[i].event);
					}
					specEntDat.sections[entities[i]->name].lists.insert({ "memories", memories });
					
					specEntDat.addInt(entities[i]->name, "entID", entities[i]->entityID);
					std::string fileName = specialEntFilePath + entities[i]->name + ".eid";
					ItemReader::SaveDataToFile(fileName, specEntDat, true);

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
