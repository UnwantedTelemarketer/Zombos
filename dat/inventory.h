#pragma once
#include "antibox/core/mathlib.h"
#include <fstream>


class Inventory
{
private:
	std::vector<std::string> itemNames;
	//std::map<std::string, std::string> itemNamesToFile;
public:
	Color clothes;
	std::vector<Item> items;
	std::unordered_map<equipType, Item> equippedItems;

	void AddItem(Item it, int count = 1)
	{
		if (it.stackable) {
			for (int i = 0; i < items.size(); i++)
			{
				if (items[i].section == it.section)
				{
					items[i].count += it.count * count;
					if (items[i].count != 0) itemNames[i] = items[i].name + " x " + std::to_string(items[i].count);
					return;
				}
			}
		}

		items.push_back(it);
		itemNames.push_back(it.name);
		for (int i = 0; i < count - 1; i++)
		{
			AddItem(it);
		}
	}

	void EquipItem(int index) {
		Item tempItem = items[index];
		tempItem.count = 1;
		if (equippedItems.contains(tempItem.eType)) {
			Item oldEquipped = equippedItems[tempItem.eType];
			equippedItems[tempItem.eType] = tempItem;
			RemoveItem(items[index].section);
			AddItem(oldEquipped);
		}
		else {
			equippedItems.insert({ tempItem.eType, tempItem });
			RemoveItem(items[index].section);
		}

		Cleanup();
	}
	
	void Unequip(equipType type) {
		Item tempItem = equippedItems[type];
		AddItem(tempItem);
		equippedItems.erase(type);
		Cleanup();
	}

	bool CurrentEquipMatches(equipType t, std::string itemName) {
		if (!equippedItems.contains(t)) {
			return false;
		}
		else {
			return equippedItems[t].section == itemName;
		}
	}
	bool CurrentEquipExists(equipType t) {
		if (!equippedItems.contains(t)) {
			return false;
		}
		else {
			return true;
		}
	}

	void Cleanup() {
		std::vector<int> itemsToErase;
		for (int i = 0; i < items.size(); i++)
		{
			if (items[i].count <= 0) {
				itemsToErase.push_back(i);
			}
		}

		for (size_t i = 0; i < itemsToErase.size(); i++)
		{
			itemNames.erase(itemNames.begin() + itemsToErase[i] - i);
			items.erase(items.begin() + itemsToErase[i] - i);
		}
	}

	//Returns true if it was fully removed from the inventory, false if only one is removed
	bool RemoveItem(std::string itemID, int amount = 1) {
		for (int i = 0; i < items.size(); i++)
		{
			if (items[i].section == itemID)
			{
				items[i].count -= amount;
				if (items[i].count > 1) itemNames[i] = items[i].name + " x " + std::to_string(items[i].count);
				else if (items[i].count == 1) itemNames[i] = items[i].name;
				else if (items[i].count <= 0) {
					items.erase(items.begin() + i);
					itemNames.erase(itemNames.begin() + i);
					return true;
				}
			}
		}
		return false;
	}

	std::string GetLiquidName(int id) {
		Liquid l;
		if (items[id].heldLiquid != nothing) { l = items[id].heldLiquid; }
		else { l = items[id].coveredIn; }
		switch (l) {
		case water:
			return "Water";
			break;
		case blood:
			return "Blood";
			break;
		case fire:
			return "Fire";
			break;
		default:
			return "Nothing";
			break;
		}
	}

	bool TryGetItem(std::string itemID, bool needEmpty, int* indexOfItem) 
	{
		for (int i = 0; i < items.size(); i++)
		{
			if (items[i].section == itemID)
			{
				if (items[i].coveredIn != nothing && items[i].liquidAmount >= 100.f) { continue; }
				*indexOfItem = i;
				return true;
			}
		}
		return false;
	}

	bool AttemptCollect(Tile* tile) {
		int itemIndex = 0;
		bool covered = false;
		if (tile->hasItem) {
			Item item = GetItemFromFile("items.eid", tile->itemName);
			if (covered) { item.CoverIn(tile->liquid, Math::RandInt(30, 60)); }

			tile->itemName = "";
			tile->hasItem = false;
			tile->ticksNeeded = Math::RandInt(1, 1000) + 500;
			AddItem(item);
			return true;
		}
		else if (tile->liquid != nothing)
		{
			if (TryGetItem("CANTEEN", false, &itemIndex)) {
				Item* item = &items[itemIndex];
				if (item->liquidAmount < 100.f && item->heldLiquid == tile->liquid || item->heldLiquid == nothing)
				{
					item->heldLiquid = tile->liquid;
					item->liquidAmount += 25.f;
					tile->SetLiquid(nothing);
					return true;
				}
			}
			else if (tile->collectible || tile->hasItem) {
				covered = true;
			}
		}
		if (tile->collectible)
		{
			Item item = GetItemFromFile("items.eid", tile->collectibleName);
			if (covered) { item.coveredIn = tile->liquid; }
			
			*tile = Tiles::GetTile(tile->collectedReplacement);
			tile->ticksNeeded = Math::RandInt(1, 1000) + 500;
			AddItem(item);
			return true;
		}
		return false;
	}

	bool AttemptAction(Action act, Item* item, Player* p)
	{
		if (item->count <= 0) { return false; }
		float amount;
		switch (act) {										//Check if we are consuming or using
		case use:
			amount = item->use.onBodyUse.amount;
			switch (item->use.onBodyUse.effect)				//If we are using, check what effect this item has on use
			{
			case heal:
				p->health += amount;
				if (p->coveredIn != nothing) { p->coveredIn = nothing; }
				break;
			case quench:
				if (item->holdsLiquid && item->liquidAmount > 0.f)
				{
					p->CoverIn(item->coveredIn, 10);
					item->liquidAmount -= 25.f;
				}
				else
				{
					return false;
				}
				p->thirst += amount;
				break;
			case saturate:
				p->hunger += amount;
				break;
			case bluntDamage:
			case pierceDamage:
				p->TakeDamage(item->use.onBodyUse.effect, amount);
				break;
			case coverInLiquid:
				p->coveredIn = (Liquid)item->use.onBodyUse.amount;
				p->liquidLast = 30;
				break;
			default:
				return false;
			}    
			return true;
			break;
		case consume:
			amount = item->use.onConsume.amount;
			switch (item->use.onConsume.effect)				//If we are consuming, check what effect this item has on consume
			{
			case heal:
				p->health += amount;
				break;
			case quench:
				if (item->holdsLiquid && item->liquidAmount > 0.f)
				{
					item->liquidAmount -= 25.f;
					p->thirst += amount;
					p->thirst = std::min(100.f, p->thirst);
					if (item->liquidAmount <= 0.f) {
						item->coveredIn = nothing;
					}
				}
				else 
				{
					return false;
				}
				break;
			case saturate:
				p->hunger += amount;
				break;
			case bluntDamage:
			case pierceDamage:
				p->TakeDamage(item->use.onConsume.effect, amount);
				break;
			default:
				return false;
			}
			return true;
			break;
		}
		return false;
	}
	


	void AddItemFromFile(std::string header, int amount = 1) {
		for (size_t i = 0; i < amount; i++)
		{
			AddItem(GetItemFromFile("file", header));
		}
	}

	Item GetItemFromFile(std::string file, std::string header) {
		/*OpenedData data;
		ItemReader::GetDataFromFile("items.eid", header, &data);
		Item item;
		item.CreateFromData(data);*/
		return Items::list[header];
	}

	std::vector<std::string>* GetItemNames() {
		return &itemNames;
	}

	void GetItems(std::vector<std::string>* idList, std::vector<std::string> validIds, int max) {
		std::vector<std::string> ids;
		for (size_t i = 0; i < items.size(); i++)
		{
			for (int b = 0; b < validIds.size(); b++)
			{
				if (items[i].section == validIds[b]) {
					for (int c = 0; c < std::min(items[i].count, max); c++)
					{
						ids.push_back(items[i].section);
					}
				}
			}
		}
		*idList = ids;
	}
};
