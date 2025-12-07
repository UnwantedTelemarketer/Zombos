#include <unordered_set>
#include <map>
#include <string>
#include <iostream>

struct Faction {
	std::string name;
	std::string leaderName;
	std::unordered_set<std::string> enemies, allies;

	void AddEnemy(std::string factionName) {
		if (enemies.count(factionName) == 0) {
			enemies.insert(factionName);
		}
	}

	void AddAlly(std::string factionName) {
		if (allies.count(factionName) == 0) {
			allies.insert(factionName);
		}
	}

	bool IsEnemies(std::string factionName) {
		return enemies.count(factionName) != 0;
	}

	bool IsAllied(std::string factionName) {
		return allies.count(factionName) != 0;
	}
};

struct FactionManager {
	std::map<std::string, Faction> list;

	void Create(const std::string& factionName);
	void Destroy(const std::string& factionName);

	bool DoesExist(const std::string& factionName);
	bool AreEnemies(const std::string& factionA, const std::string& factionB);
	bool AreAllied(const std::string& factionA, const std::string& factionB);

	Faction* GetFaction(const std::string& factionName);
};


void FactionManager::Create(const std::string& factionName) {
	Faction newFaction;

	newFaction.name = factionName;
	list.insert({ factionName, newFaction });
}

void FactionManager::Destroy(const std::string& factionName) {
	list.erase(factionName);
}

bool FactionManager::DoesExist(const std::string& factionName) {
	return list.contains(factionName);
}

Faction* FactionManager::GetFaction(const std::string& factionName) {
	if (DoesExist(factionName)) {
		return &list[factionName];
	}
	return nullptr;
}

bool FactionManager::AreEnemies(const std::string& factionA, const std::string& factionB) {
	if (!list.contains(factionA)) {
		std::cout << "Faction '" << factionA << "' does not exist." << std::endl;
		return false;
	}
	if (!list.contains(factionB)) {
		std::cout << "Faction '" << factionB << "' does not exist." << std::endl;
		return false;
	}

	return list[factionA].IsEnemies(factionB) || list[factionB].IsEnemies(factionA);
}

bool FactionManager::AreAllied(const std::string& factionA, const std::string& factionB) {
	if (!list.contains(factionA)) {
		std::cout << "Faction '" << factionA << "' does not exist." << std::endl;
		return false;
	}
	if (!list.contains(factionB)) {
		std::cout << "Faction '" << factionB << "' does not exist." << std::endl;
		return false;
	}

	return list[factionA].IsAllied(factionB) || list[factionB].IsAllied(factionA);
}


static FactionManager factions;