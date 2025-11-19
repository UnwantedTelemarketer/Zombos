#pragma once
#include "antibox/objects/tokenizer.h"
#include <iostream>
#include <fstream>
#include <set>
#include <unordered_set>

enum ConsumeEffect { none = 0, heal = 1, quench = 2, saturate = 3, pierceDamage = 4, bluntDamage = 5, coverInLiquid = 6, bandage = 7};
enum biome { desert, ocean, forest, swamp, taiga, grassland, urban, jungle };
enum Liquid { nothing = 0, water = 1, blood = 2, fire = 3, guts = 4, mud = 5 , snow = 6};
enum Action { use, consume, combine };
enum Behaviour { Wander, Protective, Protective_Stationary, Stationary, Aggressive, Follow, Tasks };
enum Faction { Human_W, Human_T, Bandit, Dweller, Zombie, Wildlife, Takers, Farmer };
enum equipType { notEquip = 0, weapon = 1, hat = 2, shirt = 3, pants = 4, boots = 5, gloves = 6, neck = 7, back = 8 };

#define ID_PLAYER 0

#define ID_ZOMBIE 1

#define ID_CHICKEN 2

#define ID_HUMAN 3

#define ID_FROG 4

#define ID_CAT 5

#define ID_COW 6

#define ID_FINDER 7

#define ID_TAKER 8



//What the effect is and how much it does.
struct EffectAmount {
	ConsumeEffect effect = none;
	float amount = 0.f;
};

//Consume, BodyUse
struct ActionEffect {
	EffectAmount onConsume;
	EffectAmount onBodyUse;
};

//Name, ID, Stackable, Holds Liquid, Consumable, Count, Consume Text, Use Text
struct Item {
	std::string name, section, description, cooks_into;
	bool stackable, holdsLiquid, consumable, cookable;
	int count = 1;
	std::string consumeTxt, useTxt;

	ActionEffect use = { {none, none}, {none, none} };

	float weight = 0.f;
	float liquidAmount = 0.f;
	Liquid coveredIn = nothing; //liquids
	Liquid heldLiquid = nothing;
	int ticksUntilDry = 0;
	int initialTickTime = 0;
	int ticksUntilCooked = 16;
	equipType eType = notEquip;
	int mod = 0;
	std::string sprite = "#";
	int emissionDist = 0;
	Vector3 spriteColor = {1,1,1};
	float maxDurability = -1.f;
	float durability = 0.f;
	std::map<std::string, int> attributes;
	std::string giveItemOnConsume = "nthng";
	std::string customSound = "nthng";

	bool waterproof = false;
	bool marker = false;
	bool canBurnThings = false;

	void CoverIn(Liquid l, int ticks) {
		coveredIn = l;
		ticksUntilDry = ticks;
		initialTickTime = ticks;
	}

	void CreateFromData(OpenedData item) {
		
		name = item.getString("name");
		section = item.section_name;
		description = item.getString("description");
		weight = item.getFloat("weight");
		//id = item.getString("id");
		stackable = item.getBool("stackable");
		holdsLiquid = item.getBool("holdsLiquid");
		consumable = item.getBool("consumable");
		count = item.getInt("amount");
		consumeTxt = item.getString("consumeTxt");
		useTxt = item.getString("useTxt");
		eType = (equipType)item.getInt("equipType");
		sprite = item.getString("sprite");
		
		//Each of these trys are an attribute that not all items have, and thus shouldnt need to specify
		try {
			maxDurability = item.getFloat("durability");
			durability = maxDurability;
		}
		catch (std::exception e) { maxDurability = -1.f; }

		try {
			 waterproof = item.getBool("waterproof");
		}
		catch (std::exception e) { waterproof = false; }

		try {
			giveItemOnConsume = item.getString("giveItemOnConsume");
		}
		catch (std::exception e) { giveItemOnConsume = "nthng"; }

		try {
			customSound = item.getString("soundOnUse");
		}
		catch (std::exception e) { customSound = "nthng"; }

		try {
			marker = item.getBool("marker");
		}
		catch (std::exception e) { marker = false; }

		try {
			canBurnThings = item.getBool("canBurnThings");
		}
		catch (std::exception e) { canBurnThings = false; }

		try {
			cookable = item.getBool("cookable");
			cooks_into = item.getString("cooksInto");
		}
		catch (std::exception e) { cookable = false; }

		try {
			emissionDist = item.getInt("emissionDistance");
		}
		catch (std::exception e) { emissionDist = 0; }

		try {
			if (item.getArray("spriteColor").size() <= 0) {
				spriteColor = { 1,0,0 };
			}
			else {
				spriteColor.x = stof(item.getArray("spriteColor")[0]);
				spriteColor.y = stof(item.getArray("spriteColor")[1]);
				spriteColor.z = stof(item.getArray("spriteColor")[2]);
			}
		}
		catch (std::exception e) {
			Console::Log("ERROR: Couldn't find spriteColor parameter for item '" + name + "'.", text::red, __LINE__);
			spriteColor = { 1,0,0 };
		}

		std::vector<std::string> effects = item.getArray("effects");

		if(eType != none) mod = item.getInt("equipModifier");
		

		use = { {(ConsumeEffect)stoi(effects[0]), (float)stoi(effects[1])},
				{(ConsumeEffect)stoi(effects[2]), (float)stoi(effects[3])} };
	}
};

struct Container {
	std::vector<Item> items;
	int sizeLimit = 999;

	std::vector<std::string> getItemNames() {
		std::vector<std::string> names;
		for (size_t i = 0; i < items.size(); i++)
		{
			if (items[i].count > 1) { names.push_back(items[i].name + " x " + std::to_string(items[i].count)); }
			else {
				names.push_back(items[i].name);
			}
		}
		return names;
	}

	bool AddItem(Item item, int amount = 1) {

		if (items.size() >= sizeLimit) { return false; }
		
		Item it = item;
		it.count = amount;
		items.push_back(it);
		return true;
	}
};

enum class MemoryType { Trade, Conversation, Observation, Helped, Attacked };

struct Feeling{
	float trust = 0.f;
	float fear = 0.f;
	float happy = 0.f;

	float overall() const {
		return trust + fear + happy;
	}

	Feeling operator*(int mult) const {
		return{
			trust * mult,
			fear * mult,
			happy * mult,
		};
	}
	Feeling operator+(Feeling fl) const {
		return{
			trust + fl.trust,
			fear + fl.fear,
			happy + fl.happy,
		};
	}
	Feeling& operator+=(const Feeling& fl) {
		trust += fl.trust;
		fear += fl.fear;
		happy += fl.happy;
		return *this;
	}

	Feeling operator-() const {
		return{
			-trust,
			-fear,
			-happy,
		};
	}
};

static Feeling FEELING_HAPPY  = { 0.f,0.f,1.f };
static Feeling FEELING_ANGRY  = { 0.f,0.f,-1.f };
static Feeling FEELING_AFRAID = { 0.f,1.f,0.f };
static Feeling FEELING_TRUST  = { 1.f,0.f,0.f };

enum QuestType {collect, kill};

struct Quest {
	QuestType qType;
	std::string what;
	Vector2_I where;
};

static Quest nthng = { kill, "nthng", {-5,-5}};

struct Memory {
	MemoryType type;
	Feeling emotion;
	std::string event;
	int who;
	int ticksPassed;
	bool persistent;
};

enum TaskType {collectItem};

struct Task {
	TaskType task;
	std::vector<Vector2_I> taskArea;
	int taskModifierID;
	std::string taskModifierString;
	int priority;
	bool currentlyFulfilling = false;
	Vector2_I currentObjective;
};

//Health, Name, ID, Behaviour, Aggressive, Faction, View Distance, Damage, Can Talk
struct Entity {
	float health;
	std::string name;
	int entityID;
	Behaviour b;
	bool aggressive;
	Faction faction;
	int viewDistance;
	int damage;
	bool canTalk;

	Vector2_I coords;
	bool lootAlive = false;
	int index; //in entity list
	Liquid coveredIn = nothing;
	int ticksUntilDry = 0;
	int tempViewDistance;
	Feeling feelingTowardsPlayer = { 0,0,0 };
	std::unordered_map<int, Feeling> feelingTowardsOthers;

	int uID;
	bool targetingPlayer;
	bool talking;
	bool smart = false;
	std::string message = "empty";
	std::string itemWant = "nthng";
	std::string itemGive = "nthng";
	Quest currentQuest = nthng;
	
	std::vector<std::string> idleSounds;
	Entity* target = nullptr;
	std::vector<Item> inv;
	std::vector<Memory> memories;
	std::vector<Task> taskList;
	const float MOOD_STRONG = 2.f;

	bool targeting() { return target != nullptr || targetingPlayer; }

	std::vector<std::string> getItemNames() {
		std::vector<std::string> names;
		for (size_t i = 0; i < inv.size(); i++)
		{
			names.push_back(inv[i].name);
		}
		return names;
	}

	void AddTask(Task t) {
		taskList.push_back(t);
	}

	std::string RandomIdleSound() {
		return idleSounds[Math::RandInt(0, idleSounds.size() - 1)];
	}

	void UpdateMemories() {
		for (auto it = memories.begin(); it != memories.end(); ) {
			// Apply memory’s emotions to feelings
			auto& mem = *it;
			Feeling& towards = (mem.who == ID_PLAYER) ? feelingTowardsPlayer
				: feelingTowardsOthers[mem.who];
			/*reinforce feeling with memory
			towards.trust += mem.emotion.trust * 0.05f;   // small reinforcement
			towards.fear  += mem.emotion.fear  * 0.05f;
			towards.happy += mem.emotion.happy * 0.05f;*/

			// Decay memory strength over time unless persistent
			if (!mem.persistent) {
				mem.emotion.trust *= 0.98f;
				mem.emotion.fear  *= 0.98f;
				mem.emotion.happy *= 0.98f;
			}
			mem.ticksPassed++;

			if (!mem.persistent && mem.emotion.overall() < 0.01f) {
				it = memories.erase(it);
			}
			else {
				++it;
			}
		}
	}

	void AddMemory(MemoryType type, int who, Feeling f, std::string whatHappened, bool persistent = false) {
		Memory m;
		m.type = type;
		m.emotion = f;
		m.who = who;
		m.event = whatHappened;
		m.ticksPassed = 0;
		m.persistent = persistent;

		memories.push_back(m);

		// Immediate effect on feelings
		if (who == ID_PLAYER) {
			feelingTowardsPlayer.trust += f.trust;
			feelingTowardsPlayer.fear  += f.fear;
			feelingTowardsPlayer.happy += f.happy;
		}
		else {
			feelingTowardsOthers[who].trust += f.trust;
			feelingTowardsOthers[who].fear  += f.fear;
			feelingTowardsOthers[who].happy += f.happy;
		}
	}

	void UpdateMood() {
		if (feelingTowardsPlayer.happy < -3.f && feelingTowardsPlayer.fear <= 5.f) {
			b = Aggressive;
		}
		else if (b == Tasks) { return; }

		else if (feelingTowardsPlayer.happy >= -2.5f) {
			b = Protective;
		}
	}

	void GenerateTrades() {
		if (Math::RandInt(0, 6) > 2) {
			OpenedData wants;
			ItemReader::GetDataFromFile("loot_tables/trade.eid", "WANTS", &wants);

			OpenedData gives;
			ItemReader::GetDataFromFile("loot_tables/trade.eid", "REWARDS", &gives);

			itemWant = wants.getArray("items")[Math::RandInt(0, wants.getArray("items").size() - 1)];
			itemGive = gives.getArray("items")[Math::RandInt(0, gives.getArray("items").size() - 1)];
		}
		else {
			itemWant = "nthng";
			itemGive = "nthng";
		}
	}

	void SelectQuest() {
		Vector2_I QuestCoords = { coords.x + Math::RandInt(-15,15), coords.y + Math::RandInt(-15,15) };
		currentQuest = { (QuestType)Math::RandInt(0,1), "nthng" ,QuestCoords};

		OpenedData c;
		ItemReader::GetDataFromFile("loot_tables/trade.eid", "WANTS", &c);

		switch(currentQuest.qType){
		case kill:
			currentQuest.what = "John Baker";
			break;
		case collect:
			currentQuest.what = c.getArray("items")[Math::RandInt(0, c.getArray("items").size() - 1)];
			break;
		default:
			break;
		}

	}

	void SelectMessage(std::map<std::string,std::vector<std::string>> npcMessages) {

		if (currentQuest.what != nthng.what)
		{
			message = "Hey, would you be able to help me out? I ";

			if (currentQuest.qType == kill) {
				message += "need you to kill ";
				message += currentQuest.what;
				message += ". Would you be able to help me out?";
			}
			if (currentQuest.qType == QuestType::collect) {
				message += "need you to find me a ";
				message += currentQuest.what;
				message += ". Would you be able to help me out?";
			}
			return;
		}
		

		float t = feelingTowardsPlayer.trust;
		float f = feelingTowardsPlayer.fear;
		float h = feelingTowardsPlayer.happy;
		
		float strongest = std::max({ std::abs(t), std::abs(f), std::abs(h) });

		if (strongest < 2.f) {
			message = npcMessages.at("CALM_WANDERER")[Math::RandInt(0, 7)];
		}
		else if (std::abs(h) == strongest) {
			message = (h > 0)
				? npcMessages.at("HAPPY_WANDERER")[Math::RandInt(0, 3)]
				: npcMessages.at("ANGRY_WANDERER")[Math::RandInt(0, 3)];
		}
		else if (std::abs(t) == strongest) {
			message = (t > 0)
				? npcMessages.at("TRUST_WANDERER")[Math::RandInt(0, 3)]
				: npcMessages.at("UNTRUST_WANDERER")[Math::RandInt(0, 3)];
		}
		else if (std::abs(f) == strongest) {
			message = (f > 0)
				? npcMessages.at("AFRAID_WANDERER")[Math::RandInt(0, 3)]
				: npcMessages.at("BRAVE_WANDERER")[Math::RandInt(0, 3)];
		}
	}

	std::vector<std::string> GenerateMessagesForMemories() {
		std::vector<std::string> memorySentences = {};
		std::unordered_set<std::string> repeatMemories;

		for (int i = 0; i < memories.size(); i++) {
			if (repeatMemories.find(memories[i].event) != repeatMemories.end()) {
				continue;
			}
			repeatMemories.insert(memories[i].event);

			std::string msg = "I ";
			switch (memories[i].type) {
			case MemoryType::Observation:
				msg += "saw you ";
				break;
			case MemoryType::Attacked:
				msg += "was attacked by you";
				memorySentences.push_back(msg);
				break;
			case MemoryType::Helped:
				msg += "was helped by you ";
				break;
			case MemoryType::Conversation:
				msg += "talked to you about ";
				break;
			case MemoryType::Trade:
				msg += "traded with you for a ";
				break;
			}

			msg += memories[i].event;
			if (i == memories.size() - 1) {
				msg += ".";
			}
			else {
				msg += ", and ";
			}
			memorySentences.push_back(msg);
		}
		return memorySentences;
	}

	// If theyre status to the player changes, they will be saved on unloading.
	// When the player enters that chunk again, it will choose one of the chunks
	// around that chunk or the original to respawn the entity in.
};

struct Player {
	float health = 100;
	float thirst = 100;
	float hunger = 100;
	int damage = 5;
	int bleedingLevel = 0;
	int sicknessLevel = 0;
	int ticksCovered = 0;
	int liquidLast = 50;
	bool aiming = false;
	std::string name = "Blank";
	Vector2_I coords;
	Vector2_I crosshair;
	Liquid coveredIn = nothing;
	float visualTemp, bodyTemp = 98.5f;
	int bleedTicks = 0;
	int sickTicks = 0;
	int healTick = 0;
	int thirstTick = 0;
	int hungerTick = 0;
	int damageMode = 0;
	bool stunned = false;
	bool indoors = false;

	void Restart() {
		health = 100;
		thirst = 100;
		hunger = 100;
		damage = 5;
		bleedingLevel = 0;
		sicknessLevel = 0;
		ticksCovered = 0;
		liquidLast = 50;

		coords = { 0,0 };
		bodyTemp = 98.5f;
		bleedTicks = 0;
		sickTicks = 0;
		healTick = 0;
		thirstTick = 0;
		hungerTick = 0;
		damageMode = 0;
		coveredIn = nothing;
	}

	void TakeDamage(ConsumeEffect type, int dmg) {
		if (damageMode != 2) { health -= dmg; }
		if (type == pierceDamage) {
			CoverIn(blood, 20);
			bleedingLevel++;
			if (bleedingLevel > 3) {
				bleedingLevel = 3;
			}
		}
		if (damageMode == 1 && health <= 0) { health = 1; }
	}

	void CheckStatus() {
		int prevHealth = health;
		//hunger and thirst
		//get more thirsty if the player is extra hot / sick
		if (bodyTemp >= 99.5f) { thirstTick+=2; }
		else { thirstTick++; }
		hungerTick++;

		//lose health if dehydrated
		if (((thirstTick % 25) == 0) && thirst < 40) {
			health -= 1;
		}
		if (thirstTick >= 50) {
			thirstTick = 0;
			thirst -= 1.f;
		}

		//lose health if starving
		if (((hungerTick % 50) == 0) && hunger < 40) {
			health -= 1;
		}
		if (hungerTick >= 100) {
			hungerTick = 0;
			hunger -= 1.f;

			if (hunger < 40) {
				health -= 1;
			}
		}


		/*if(thirst < 0) { health -= 0.5f; thirst = 0; }
		if (hunger < 0) { health -= 0.5f; hunger = 0; }*/
		if (coveredIn == fire) { health -= 1.f; }

		//Liquid spilling
		if (coveredIn != nothing) { ticksCovered++; }
		if (ticksCovered > liquidLast) { ticksCovered = 0; coveredIn = nothing; }

		if (bleedingLevel > 0) {
			bleedTicks++;
			if (bleedingLevel == 1 && bleedTicks >= 40) {
				bleedTicks = 0;
				health -= 1;
			}
			else if (bleedingLevel == 2 && bleedTicks >= 20) {
				bleedTicks = 0;
				health -= 1;
			}
			else if (bleedingLevel == 3 && bleedTicks >= 2) {
				bleedTicks = 0;
				health -= 1;
			}
		}
		healTick++;

		//cant heal if youre too hungry
		if (hunger >= 70 && bleedingLevel <= 0) {
			if (sicknessLevel < 2) {
				if (healTick >= 100) {
					health += 2;
					healTick = 0;
				}
			}
			else {
				if (healTick >= 150) {
					health += 2;
					healTick = 0;
				}
			}
		}

		if (bleedingLevel > 3) {
			bleedingLevel = 3;
		}
		if (sicknessLevel > 3) {
			sicknessLevel = 3;
		}

		if (damageMode == 2) { health = prevHealth; }
		else if (damageMode == 1 && health <= 0) { health = 1; }
	}

	//Covers them in but does some minor checks to clean you off and stuff
	void CoverIn(Liquid liquid, int turns) {
		liquidLast = turns;
		if (coveredIn != nothing && coveredIn != water && liquid == water) { coveredIn = nothing; return; }
		if (coveredIn == liquid) { ticksCovered -= turns; }
		ticksCovered = ticksCovered < 0 ? 0 : ticksCovered;
		coveredIn = liquid;
	}
};
struct Tile;

struct Saved_Container {
	int x, y;
	std::vector<std::string> items;
	void Serialize(std::ofstream& stream) {
		// Write the number of strings
		size_t numStrings = items.size();
		stream.write(reinterpret_cast<const char*>(&numStrings), sizeof(numStrings));

		// Write each string
		for (const auto& str : items) {
			size_t length = str.size();
			stream.write(reinterpret_cast<const char*>(&length), sizeof(length)); // Write string length
			stream.write(str.data(), length); // Write string data
		}
		stream.write(reinterpret_cast<const char*>(&x), sizeof(x));
		stream.write(reinterpret_cast<const char*>(&y), sizeof(y));
	}

	void Deserialize(std::ifstream& stream){
		// Read the number of strings
		size_t numStrings;
		stream.read(reinterpret_cast<char*>(&numStrings), sizeof(numStrings));

		// Read each string
		std::vector<std::string> strings;
		for (size_t i = 0; i < numStrings; ++i) {
			size_t length;
			stream.read(reinterpret_cast<char*>(&length), sizeof(length)); // Read string length

			std::string str(length, '\0'); // Allocate memory for the string
			stream.read(&str[0], length); // Read string data
			items.push_back(std::move(str));
		}
		stream.read(reinterpret_cast<char*>(&x), sizeof(x)); 
		stream.read(reinterpret_cast<char*>(&y), sizeof(y)); 

	}
};

struct Saved_Tile {
	int id = -1;
	Liquid liquid = nothing;
	int burningFor = 0;
	int ticksPassed = 0;
	int ticksNeeded = 1;
	int liquidTicks = 0;
	std::string itemName = "NULL";
	short biomeID;
	bool hasContainer = false;
	Saved_Container cont;
	bool colorDiff = false;
	Vector3 tempColor = {0,0,0};

	void Serialize(std::ofstream& stream) {
		stream.write(reinterpret_cast<const char*>(&id), sizeof(id));
		stream.write(reinterpret_cast<const char*>(&liquid), sizeof(liquid));

		stream.write(reinterpret_cast<const char*>(&burningFor), sizeof(burningFor));
		stream.write(reinterpret_cast<const char*>(&ticksPassed), sizeof(ticksPassed));
		stream.write(reinterpret_cast<const char*>(&ticksNeeded), sizeof(ticksNeeded));
		stream.write(reinterpret_cast<const char*>(&liquidTicks), sizeof(liquidTicks));
		stream.write(reinterpret_cast<const char*>(&biomeID), sizeof(biomeID));
		stream.write(reinterpret_cast<const char*>(&hasContainer), sizeof(hasContainer));
		if (hasContainer) {
			cont.Serialize(stream);
		}
		stream.write(reinterpret_cast<const char*>(&colorDiff), sizeof(colorDiff));
		if (colorDiff) {
			stream.write(reinterpret_cast<const char*>(&tempColor.x), sizeof(tempColor.x));
			stream.write(reinterpret_cast<const char*>(&tempColor.y), sizeof(tempColor.y));
			stream.write(reinterpret_cast<const char*>(&tempColor.z), sizeof(tempColor.z));
		}

		size_t size = itemName.size();
		stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
		stream.write(itemName.data(), size);

	}
	void Deserialize(std::ifstream& stream) {
		// Read non-trivial members separately
		stream.read(reinterpret_cast<char*>(&id), sizeof(id));
		stream.read(reinterpret_cast<char*>(&liquid), sizeof(liquid));

		// Read other members
		stream.read(reinterpret_cast<char*>(&burningFor), sizeof(burningFor));
		stream.read(reinterpret_cast<char*>(&ticksPassed), sizeof(ticksPassed));
		stream.read(reinterpret_cast<char*>(&ticksNeeded), sizeof(ticksNeeded));
		stream.read(reinterpret_cast<char*>(&liquidTicks), sizeof(liquidTicks));
		stream.read(reinterpret_cast<char*>(&biomeID), sizeof(biomeID));
		stream.read(reinterpret_cast<char*>(&hasContainer), sizeof(hasContainer));
		if (hasContainer) {
			cont.Deserialize(stream);
		}
		stream.read(reinterpret_cast<char*>(&colorDiff), sizeof(colorDiff));
		if (colorDiff) {
			stream.read(reinterpret_cast<char*>(&tempColor.x), sizeof(tempColor.x));
			stream.read(reinterpret_cast<char*>(&tempColor.y), sizeof(tempColor.y));
			stream.read(reinterpret_cast<char*>(&tempColor.z), sizeof(tempColor.z));
		}

		size_t size = 0;
		stream.read(reinterpret_cast<char*>(&size), sizeof(size));

		// Read the string data
		itemName.resize(size);
		stream.read(&itemName[0], size);
	}
};
enum direction{up, down, left, right, still};

struct Tile {
	int id = -1;
	direction technical_dir = direction::up;
	Liquid liquid = nothing;
	Entity* entity = nullptr;
	Container* tileContainer = nullptr;
	bool collectible = false;
	std::string collectibleName = "NULL";
	std::string collectedReplacement;
	std::string itemName = "NULL";
	int burningFor = 0;
	bool walkable = true;

	bool changesOverTime = false;
	std::string timedReplacement;
	int ticksPassed = 0;
	int ticksNeeded = 1;
	bool hasItem = false;
	float brightness = 1.f;
	int liquidTime = 0;
	bool visited = false;
	bool technical_update = false;
	vec2_i coords;
	vec2_i g_coords = { 0, 0 };
	bool double_size = false;
	bool casts_shadow = false;
	vec3 tileColor;
	vec3 mainTileColor;
	short biomeID;
	std::string tileLerpID = "nthng";
	std::string tileSprite;

	bool CanUpdate() {
		return ticksPassed >= ticksNeeded && changesOverTime;
	}

	void ResetColor() {
		tileColor = mainTileColor;
	}

	void SetLiquid(Liquid l, bool perm = false) {
		liquid = l;
		liquidTime = perm ? -1 : 0;
		switch (l) {
		case water:// check if the water placed is like permanent like a body of water, and choose the color based on biome
			//I HOPE THIS NEVER BREAKS AGAIN BUT IM SURE IT WILL
			//BECAUSE THIS IS SO GROSSLY HARDCODED AND I CANNOT FIGURE OUT ANOTHER WAY SO IM DONE AND IT WORKS
			if (perm) {
				switch (biomeID) {
				case taiga:
					tileColor = mainTileColor;
					tileColor = { 0,0.5,1 };
					break;
				case swamp:
					tileColor = mainTileColor;
					tileColor = { 0.f, 0.5f, 0.4f };
				}
			}
			else {
				tileColor = mainTileColor;
				tileColor += { 0, 0.75, 1.5 };
				tileColor /= 2.5;
			}
			break;
		case guts:
			tileColor = { 0.45, 0, 0 };
			break;
		case blood:
			tileColor = { 1, 0, 0 };
			break;
		case mud:
			tileColor = { 0.5f, 0.3f, 0 };
			break;
		case nothing:
			std::string lerpID = "tileColor" + std::to_string(Math::RandInt(1, 50000));
			tileLerpID = lerpID;
			Utilities::Lerp(lerpID, &tileColor, mainTileColor, 0.5f);
			break;
		}
		 
	}


	void CreateFromData(OpenedData data) {
		id = data.getInt("id");
		technical_dir = (direction)data.getInt("direction");
		liquid = (Liquid)data.getInt("liquid");
		collectible = data.getBool("collectible");
		collectedReplacement = data.getString("replacement");
		itemName = data.getString("itemName");
		hasItem = data.getBool("hasItem");
		walkable = data.getBool("walkable");
		changesOverTime = data.getBool("changesOverTime");
		timedReplacement = data.getString("timedReplacement");
		double_size = data.getBool("double_size");
		tileSprite = data.getString("sprite");

		if (data.getArray("color").size() <= 0) {
			tileColor = { 1,0,0 };
		}
		else {
			tileColor.x = stof(data.getArray("color")[0]);
			tileColor.y = stof(data.getArray("color")[1]);
			tileColor.z = stof(data.getArray("color")[2]);
		}
		try { //not everything is collectible, dont require it
			collectibleName = data.getString("collectibleName");
		} catch (std::exception e) { }
		try { //not everything is shadow, dont require it
			casts_shadow = data.getBool("castsShadow");
		}
		catch (std::exception e) {}

		mainTileColor = tileColor;
	}
};

static void CreateSavedTile(Saved_Tile* sTile, Tile tile) {
	sTile->id = tile.id;
	sTile->liquid = tile.liquid;
	sTile->burningFor = tile.burningFor;
	sTile->ticksPassed = tile.ticksPassed;
	sTile->ticksNeeded = tile.ticksNeeded;
	sTile->liquidTicks = tile.liquidTime;
	sTile->itemName = tile.itemName == "NULL" ? "_" : tile.itemName;
	sTile->biomeID = tile.biomeID;

	if (tile.tileContainer != nullptr) {
		sTile->hasContainer = true;
		sTile->cont.x = tile.coords.x;
		sTile->cont.y = tile.coords.y;
		for (auto const& item : tile.tileContainer->items) {
			sTile->cont.items.push_back(item.section);
		}
	}
	if (tile.tileColor != tile.mainTileColor && tile.liquid == Liquid::nothing) {
		sTile->tempColor = tile.tileColor;
		sTile->colorDiff = true;
	}
}

class Inventory;





#ifdef REGULAR_FONT
#define TILE_LIQUID "~"

#define TILE_NULL "?"
#define ID_NULL -1

#define TILE_GRASS ";"
#define ID_GRASS 1

#define TILE_DIRT "."
#define ID_DIRT 2

#define TILE_TREE "^"
#define ID_TREE 999

#define TILE_FLOWER "#"
#define ID_FLOWER 3

#define TILE_FIRE "&"
#define ID_FIRE 4

#define TILE_SCRAP "%%"
#define ID_SCRAP 5

#define TILE_STONE "8"
#define ID_STONE 6

#define TILE_STICK "/"
#define ID_STICK 10
#endif

#ifdef ICON_FONT
#define TILE_LIQUID "A"

#define TILE_NULL "?"
#define ID_NULL -1

#define TILE_GRASS "B"
#define ID_GRASS 1

#define TILE_DIRT "C"
#define ID_DIRT 2

#define TILE_TREE "Z"
#define ID_TREE 999

#define TILE_FLOWER "H"
#define ID_FLOWER 3

#define TILE_FIRE "G"
#define ID_FIRE 4

#define TILE_SCRAP "D"
#define ID_SCRAP 5

#define TILE_STONE "J"
#define ID_STONE 6

#define TILE_SAND "C"
#define ID_SAND 7

#define TILE_CONTAINER "J"
#define ID_CONTAINER 8

#define TILE_CONTAINER_OPEN "J"
#define ID_CONTAINER_OPEN 9

#define TILE_STICK "I"
#define ID_STICK 10


#define TILE_CONVEYOR_U "M"
#define ID_CONVEYOR_U 100

#define TILE_CONVEYOR_D "N"
#define ID_CONVEYOR_D 101

#define TILE_CONVEYOR_L "T"
#define ID_CONVEYOR_L 102

#define TILE_CONVEYOR_R "S"
#define ID_CONVEYOR_R 103

#define TILE_CONVEYOR_UL "O"
#define ID_CONVEYOR_UL 104

#define TILE_CONVEYOR_UR "P"
#define ID_CONVEYOR_UR 105

#define TILE_CONVEYOR_DL "Q"
#define ID_CONVEYOR_DL 106

#define TILE_CONVEYOR_DR "R"
#define ID_CONVEYOR_DR 107
#endif
