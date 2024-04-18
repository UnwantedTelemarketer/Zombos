
#include "dat/game.h"
#include "antibox/objects/tokenizer.h"


#include <chrono>

#define UNIFONT "c:\\Users\\Thomas Andrew\\AppData\\Local\\Microsoft\\Windows\\Fonts\\Unifont.ttf"
#define DOSFONT "dat\\fonts\\symbolic_conveyor.ttf"
#define CASCADIA "c:\\Windows\\Fonts\\CascadiaCode.ttf"
using namespace antibox;

enum GameState {playing, menu};

class Caves : public App {
private:
	WindowProperties GetWindowProperties() { 
		WindowProperties props;
		
		props.imguiProps = { true, true, false, DOSFONT };
		props.w = 1280;
		props.h = 720;
		props.vsync = 0;
		props.cc = { 0.5, 0.5, 0.5, 1 };
		return props;
	}

	float tickRateVisual, lastFPS;
	int counter = 0;
	GameState currentState;
	vec3 clothes;
public:
	//UI stuff
	bool statsOpen, debugOpen, interacting, itemMenu, navInv, useBool, craftingMenu, showDialogue, createChar, fancyGraphics;
	Tile* selectedTile = nullptr;
	int currentItemIndex = 0;
	std::string openClose;
	antibox::framerate frame;

	//Game Stuff
	GameManager game;
	Inventory& pInv = game.pInv;
	Player& player = game.mPlayer;
	float& health = game.mPlayer.health;
	Map& map = game.mainMap;

	//Sounds
	std::vector<std::string> walk_sounds = { "", "dat/sounds/walk_1.wav" , "dat/sounds/walk_2.wav" , "dat/sounds/walk_3.wav" , "dat/sounds/walk_4.wav" };

	void Init() override {
		fancyGraphics = true;
		frame.frames.length = 360;
		currentState = menu;
		health = 100.0f;
		statsOpen = true;
		openClose = "Close Stats";
		navInv = false;
		showDialogue = true;

		Engine::Instance().SetVolume(0.1f);
	}

	void Update() {
		bool moved = false;
		if (currentState == menu) { return; }
		if (player.health <= 0) { currentState = menu; player.health = 100; }

		game.UpdateTick();

		if (Input::KeyDown(KEY_SEMICOLON)) {
			Engine::Instance().SetApp(1);
		}

		if (Input::KeyDown(KEY_UP)) {
			if (interacting)
			{
				selectedTile = { map.TileAtPos(Vector2_I{ player.coords.x - 1, player.coords.y }) };
				interacting = false;
				return;
			}
			else if (navInv)
			{
				currentItemIndex--;
				currentItemIndex = currentItemIndex < 0 ? 0 : currentItemIndex;
				return;
			}
			else if (player.aiming) {
				player.crosshair.x -= 1;
				return;
			}
			else {
				moved = true;
				selectedTile = nullptr;
				game.MovePlayer(MAP_UP);
			}
		}
		else if (Input::KeyDown(KEY_DOWN)) {
			if (interacting)
			{
				Tile &selTile = *map.TileAtPos(Vector2_I{ player.coords.x + 1, player.coords.y });
				selectedTile = { &selTile };
				if (selTile.entity != nullptr) {

				}
				interacting = false;
				return;
			}
			else if (navInv)
			{
				currentItemIndex++;
				if(currentItemIndex > pInv.items.size() - 1) currentItemIndex = pInv.items.size() - 1;
				return;
			}
			else if (player.aiming) {
				player.crosshair.x += 1;
				return;
			}
			else {
				moved = true;
				selectedTile = nullptr;
				game.MovePlayer(MAP_DOWN);
			}
		}
		else if (Input::KeyDown(KEY_LEFT)) {
			if (interacting) 
			{
				selectedTile = { map.TileAtPos(Vector2_I{ player.coords.x, player.coords.y - 1}) };
				interacting = false;
				return;
			}
			else if (player.aiming) {
				player.crosshair.y -= 1;
				return;
			}
			else {
				moved = true;
				selectedTile = nullptr;
				game.MovePlayer(MAP_LEFT);
			}
		}
		else if (Input::KeyDown(KEY_RIGHT)) {
			if (interacting)
			{
				selectedTile = { map.TileAtPos(Vector2_I{ player.coords.x, player.coords.y + 1}) };
				interacting = false;
				return;
			}
			else if (player.aiming) {
				player.crosshair.y += 1;
				return;
			}
			else {
				moved = true;
				selectedTile = nullptr;
				game.MovePlayer(MAP_RIGHT);
			}
		}
		else if (Input::KeyDown(KEY_P))
		{
			debugOpen = !debugOpen;
		}

		else if (Input::KeyDown(KEY_I))
		{
			navInv = !navInv;
			itemMenu = navInv;
		}

		else if (Input::KeyDown(KEY_E))
		{
			if (!navInv) 
			{
				Math::PushBackLog(&game.actionLog, "Which direction will you interact with?");
				interacting = true;
			}
		}

		else if (Input::KeyDown(KEY_C))
		{
			craftingMenu = !craftingMenu;
		}

		else if (Input::KeyDown(KEY_A))
		{
			player.aiming = !player.aiming;
		}

		if (moved) {
			const char* soundName = walk_sounds[Math::RandInt(0, 4)].c_str();
			Engine::Instance().StartSound(soundName);
		}
		
		if (player.aiming) { 
			map.ClearLine();
			map.DrawLine(map.GetLine(player.coords, player.crosshair, 25)); 
		}
;	}

	void ImguiRender() override
	{
		//GameScene();
		switch (currentState) {
		case playing:
			GameScene();
			break;
		case menu:
			MenuScene();
			break;
		default:
			break;
		}


	}

	void MenuScene() {
		ImGui::Begin("Menu");
		if(createChar){ Create_Character(); }
		

		if (ImGui::Button("New Game"))
		{
			game.Setup(10, 10, 0.5f);

			createChar = true;
		}

		if (ImGui::Button("Continue Game"))
		{
			OpenedData data;
			ItemReader::GetDataFromFile("save.eid", "STATS", &data);

			player.health = data.getFloat("health");
			player.thirst = data.getFloat("thirst");
			player.hunger = data.getFloat("hunger");
			map.c_glCoords = { data.getInt("global_x"), data.getInt("global_y") };

			pInv.clothes = { data.getFloat("color_r"), data.getFloat("color_g"), data.getFloat("color_b") };

			for (int i = 0; i < data.getArray("items").size() - 1; i++) //loop through all items
			{
				for (int s = 0; s < stoi(data.getArray("items")[i+1]); s++)
				{
					pInv.AddItemFromFile("items.eid", data.getArray("items")[i]); //give it a certain amount of items
				}
				i++;
			}
			game.Setup(data.getInt("x_pos"), data.getInt("y_pos"), 0.5f, data.getInt("seed"), data.getInt("biomes"));
			currentState = playing;
		}
		ImGui::End();
	}

	void Create_Character() {
		ImGui::Begin("Create your Character");

		ImGui::ColorPicker3("Clothes Color", &clothes.x);

		pInv.clothes = { clothes.x, clothes.y, clothes.z };
		if (ImGui::Button("Start")) {
			pInv.AddItemFromFile("items.eid", "BANDAGE");
			currentState = playing;
		}
		ImGui::End();
	}

	void GameScene() {

		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		//Entity* ent = game.NearEnt();
		//if (ent != nullptr && ent->canTalk) {
		//	DisplayEntity(ent);
		//}

		//------Map-------
		ImGui::Begin("Map");
		if(fancyGraphics) ImGui::PushFont(Engine::Instance().getFont());
		for (int i = 0; i < CHUNK_WIDTH; i++) {
			for (int j = 0; j < CHUNK_HEIGHT; j++) {
				float intensity = map.CurrentChunk()->localCoords[i][j].brightness;

				if (i < CHUNK_HEIGHT - 1) {
					Entity* curEnt = map.CurrentChunk()->localCoords[i + 1][j].entity;

					if (curEnt != nullptr) {
						if (curEnt->targeting() && curEnt->health > 0) {
							ImGui::TextColored(ImVec4{ 1,0,0,1 }, "!");
							ImGui::SameLine();
							continue;
						}
					}
				}

				if (Vector2_I{ player.coords.x,player.coords.y } == Vector2_I{ i,j })
				{
					ImGui::TextColored(ImVec4{ pInv.clothes.x,pInv.clothes.y, pInv.clothes.z, 1 }, ENT_PLAYER);
				}

				else if (map.effectLayer.localCoords[i][j] == 15)
				{
					ImGui::TextColored(ImVec4{ 1,0,1,1 }, "X");
				}
				else {
					ImGui::TextColored(game.GetTileColor(map.CurrentChunk()->localCoords[i][j], intensity),
						game.GetTileChar(map.CurrentChunk()->localCoords[i][j]).c_str());
				}


				ImGui::SameLine();
			}
			ImGui::Text("");
		}
		if (fancyGraphics) ImGui::PopFont();
		ImGui::End();

		TechScreen();

		//------Action Log----
		ImGui::Begin("Action Log");
		//ImGui::PushFont(Engine::Instance().getFont());
		for (int i = 0; i < game.actionLog.size(); i++)
		{
			ImGui::Text(game.actionLog[i].c_str());
		}
		//ImGui::PopFont();
		ImGui::End();

		//------Stats------
		if (statsOpen) {
			ImGui::Begin("Stats");
			ImGui::TextColored(ImVec4{ 0.85, 0.15, 0.15, 1 }, "Health");
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4{ 0.85,0,0,1 });
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0.5,0,0,1 });
			ImGui::ProgressBar(game.mPlayer.health / 100, ImVec2(0.0f, 0.0f));
			ImGui::PopStyleColor(2);


			ImGui::TextColored(ImVec4{ 0.15, 0.65, 1, 1 }, "Thirst");
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4{ 0,0.5,1,1 });
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0,0.25,0.5,1 });
			ImGui::ProgressBar(game.mPlayer.thirst / 100, ImVec2(0.0f, 0.0f));
			ImGui::PopStyleColor(2);


			ImGui::TextColored(ImVec4{ 1, 0.6, 0.15, 1 }, "Hunger");
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4{ 0.8,0.5,0,1 });
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0.4,0.25,0,1 });
			ImGui::ProgressBar(game.mPlayer.hunger / 100, ImVec2(0.0f, 0.0f));
			ImGui::PopStyleColor(2);

			switch (player.coveredIn) {
			case water:
				ImGui::TextColored(ImVec4{ 0, 0.6, 1, 1 }, "Wet!");
				break;
			case blood:
				ImGui::TextColored(ImVec4{ 1, 0.15, 0, 1 }, "Bleeding!");
				break;
			case fire:
				ImGui::TextColored(Cosmetic::FireColor(), "Burning!");
				break;
			}
			ImGui::End();
		}
		//------Inventory------
		ImGui::Begin("Inventory");
		for (int i = 0; i < pInv.items.size(); i++)
		{
			if (pInv.items[i].count <= 0) { continue; }
			if (i == currentItemIndex && navInv) {
				ImGui::Text("> "); ImGui::SameLine();
			}
			int itemLiquid = (int)pInv.items[i].coveredIn;

			ImGui::TextColored(Cosmetic::CoveredColor(itemLiquid), Cosmetic::CoveredName(itemLiquid));
			ImGui::SameLine();

			ImGui::Text(pInv.items[i].name.c_str());
			if (pInv.items[i].count > 1)
			{
				ImGui::SameLine(); ImGui::Text("x");
				ImGui::SameLine(); ImGui::Text(CHAR_ARRAY(pInv.items[i].count));
			}
		}
		ImGui::End();

		//------Crafting-------
		if (craftingMenu) {
			ImGui::Begin("Crafting");

			std::string crafted = "none";
			int amount = 1;
			std::vector<std::string> ids;
			if (ImGui::Button("Craft Rope (3 X Grass)")) {
				pInv.GetItems(&ids, { ITEM_GRASS }, 3);
				crafted = ITEM_ROPE;
			}
			if (ImGui::Button("Craft 3 Scrap Bits (1 X Scrap)")) {
				pInv.GetItems(&ids, { ITEM_SCRAP }, 1);
				crafted = ITEM_MONEY;
				amount = 3;
			}
			if (ImGui::Button("Craft Canteen (3 X Scrap)")) {
				pInv.GetItems(&ids, { ITEM_SCRAP }, 3);
				crafted = ITEM_CONTAINER;
			}
			if (ImGui::Button("Craft Knife (1 X Scrap, 1 X Stick, 1 X Rope)")) {
				pInv.GetItems(&ids, { ITEM_SCRAP, ITEM_STICK, ITEM_GRASS }, 1);
				crafted = ITEM_KNIFE;
			}

			if (crafted != "none") {
				Console::Log(crafted, WARNING, __LINE__);
				Item newItem = game.Crafter.craft(ids, crafted);
				if (newItem.id != "none") {
					pInv.AddItem(newItem, amount);
					for (std::string id : ids) {
						pInv.RemoveItem(id);
					}
				}
			}
			ImGui::End();
		}



		//------Interaction------
		if (selectedTile != nullptr)
		{
			ImGui::Begin("Selected Block");

			ImGui::PushFont(Engine::Instance().getFont());
			ImGui::TextColored(game.GetTileColor(*selectedTile, 1.f), game.GetTileChar(*selectedTile).c_str());
			ImGui::PopFont();

			const char* label = "";
			if (map.isUnderground) {
				label = "Go Up";
			}
			else {
				label = "Go Down";
			}
			if (ImGui::Button(label)) {
				map.isUnderground = !map.isUnderground;
			}


			if (ImGui::Button("Drop Selected Item")) {
				if (itemMenu && !selectedTile->hasItem) {
					selectedTile->hasItem = true;
					std::string upperName = pInv.items[currentItemIndex].name;

					for (auto& c : upperName) c = toupper(c);

					selectedTile->itemName = upperName;
					selectedTile->collectible = true;
					if (pInv.RemoveItem(pInv.items[currentItemIndex].id)) { currentItemIndex = 0; }
				}

			}


			if (selectedTile->collectible || selectedTile->liquid != nothing || selectedTile->hasItem) {
				if (ImGui::Button("Collect")) {
					if (pInv.AttemptCollect(selectedTile)) {
						Math::PushBackLog(&game.actionLog, "You collect an item off the ground.");
					}
					else {
						Math::PushBackLog(&game.actionLog, "You can't collect that.");
					}
					selectedTile = nullptr;
				}

			}
			if (ImGui::Button("Burn")) {
				selectedTile->liquid = fire;
				map.floodFill(selectedTile->coords);
			}

			int _ = 0;
			if (pInv.TryGetItem(ITEM_ROCK, false, &_)) {
				if (ImGui::Button("Place Wall")) {
					*selectedTile = tileByID[ID_STONE];
					pInv.RemoveItem(ITEM_ROCK);
				}
			}

			ImGui::End();
		}
		//------item------
		if (itemMenu)
		{
			ImGui::Begin("Current Item");
			ImGui::Text(pInv.items[currentItemIndex].name.c_str());
			ImGui::Text(pInv.items[currentItemIndex].description.c_str());

			std::string liquid = pInv.GetLiquidName(currentItemIndex);
			ImVec4 color, colorBG;
			if (liquid == "Water") {
				color = { 0, 0.5, 1, 1 };
				colorBG = { 0, 0, 0.45, 1 };
			}
			else if (liquid == "Blood") {
				colorBG = { 0.25, 0, 0, 1 };
				color = { 0.75, 0, 0, 1 };
			}
			else if (liquid == "Fire") {
				colorBG = { 0.35, 0.25, 0, 1 };
				color = Cosmetic::FireColor();
			}
			else {
				colorBG = { 0.2, 0.2, 0.2, 1 };
				color = { 0.6, 0.6, 0.6, 1 };
			}


			if (!pInv.items[currentItemIndex].stackable) { ImGui::TextColored(ImVec4{ 1,0,0,1 }, "[DOES NOT STACK]"); }

			if (pInv.items[currentItemIndex].holdsLiquid) {
				ImGui::Text("Liquid:");
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, colorBG);

				ImGui::ProgressBar(pInv.items[currentItemIndex].liquidAmount / 100, ImVec2(0.0f, 0.0f));
				ImGui::PopStyleColor(2);
				ImGui::Text("Liquid Type:"); ImGui::SameLine();
				ImGui::Text(pInv.GetLiquidName(currentItemIndex).c_str());
			}

			if (pInv.items[currentItemIndex].ticksUntilDry > 0) {
				Item& tempItem = pInv.items[currentItemIndex];
				ImGui::Text("Wetness:");
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, colorBG);

				ImGui::ProgressBar((float)tempItem.ticksUntilDry / (float)tempItem.initialTickTime, ImVec2(0.0f, 0.0f));
				ImGui::PopStyleColor(2);

			}

			if (pInv.items[currentItemIndex].stackable) {
				ImGui::Text("[ Holding x");
				ImGui::SameLine();
				ImGui::Text(std::to_string(pInv.items[currentItemIndex].count).c_str());
				ImGui::SameLine();
				ImGui::Text("]");
			}

			if (ImGui::Button("Use"))
			{
				useBool = !useBool;
			}

			if (useBool)
			{
				ImGui::Text("On what?");
				if (ImGui::Button("Consume"))
				{
					if (pInv.AttemptAction(consume, &pInv.items[currentItemIndex], &player))
					{
						//if its consumable, remove one. 
						if (pInv.items[currentItemIndex].consumable) {
							//If its the last one, move the cursor so we dont get out of vector range
							if (pInv.RemoveItem(pInv.items[currentItemIndex].id)) {
								currentItemIndex--;
							}

						}
						Math::PushBackLog(&game.actionLog, pInv.items[currentItemIndex].consumeTxt);
					}
					else
					{
						Math::PushBackLog(&game.actionLog, "You can't consume " + pInv.items[currentItemIndex].name + ".");
					}
					useBool = !useBool;
				}
				if (ImGui::Button("Body (Self)"))
				{
					if (pInv.AttemptAction(use, &pInv.items[currentItemIndex], &player))
					{
						if (pInv.items[currentItemIndex].consumable) { pInv.items[currentItemIndex].count--; }
						Math::PushBackLog(&game.actionLog, pInv.items[currentItemIndex].useTxt);
					}
					else
					{
						Math::PushBackLog(&game.actionLog, "You can't use " + pInv.items[currentItemIndex].name + ".");
					}
					useBool = !useBool;
				}
			}

			ImGui::End();
		}

		//------DEBUG------
		if (debugOpen) {
			ImGui::Begin("Debug Window");

			if (ImGui::Button("Toggle Graphics")) {
				fancyGraphics = !fancyGraphics;
			}

			ImGui::Text(("Current World Time: " + std::to_string(game.worldTime)).c_str());
			//FPS
			if (counter == 30) {
				lastFPS = Engine::Instance().getFPS();
				counter = 0;
			}
			else { counter++; }
			frame.Update(Engine::Instance().getFPS());
			ImGui::Text(("High: " + std::to_string(frame.highest)).c_str());
			ImGui::Text(("Low: " + std::to_string(frame.lowest)).c_str());
			ImGui::Text(("Current: " + std::to_string(lastFPS)).c_str());
			ImGui::PlotLines("Frame Times", frame.frames.c_arr(), frame.frames.length);
			//ms left until next tick
			ImGui::Text(("Time until next update:"));
			//Display a bar until next tick
			ImGui::ProgressBar(game.GetTick() / game.TickRate(), ImVec2(0.0f, 0.0f));
			//set tickrate
			ImGui::InputFloat("New Tickrate", &tickRateVisual, 0.5f, 10);
			if (ImGui::Button("Change Tickrate")) {
				game.SetTick(tickRateVisual);
			}
			ImGui::Text(("Local X: " + std::to_string(player.coords.x)).c_str());
			ImGui::Text(("Local Y: " + std::to_string(player.coords.y)).c_str());

			ImGui::Text(("Global X: " + std::to_string(map.c_glCoords.x)).c_str());
			ImGui::Text(("Global Y: " + std::to_string(map.c_glCoords.y)).c_str());

			ImGui::Text("Entities: "); ImGui::SameLine();
			ImGui::Text(CHAR_ARRAY(map.CurrentChunk()->entities.size()));
			for (size_t i = 0; i < map.CurrentChunk()->entities.size(); i++)
			{
				ImGui::Text(map.CurrentChunk()->entities[i]->name); ImGui::SameLine();
				ImGui::Text(CHAR_ARRAY(map.CurrentChunk()->entities[i]->coords.x)); ImGui::SameLine();
				ImGui::Text(CHAR_ARRAY(map.CurrentChunk()->entities[i]->coords.y));
			}

			ImGui::End();
		}

#ifdef DEV_TOOLS
		ImGui::Begin("Brightness Map");
		int counter = 0;
		for (const auto& pair : game.mainMap.world.chunks) {
			ImGui::Text(("(" + std::to_string(pair.first.x)).c_str()); ImGui::SameLine();
			ImGui::Text((std::to_string(pair.first.y) + ")").c_str()); if(counter != 2 && counter != 5) ImGui::SameLine();
			counter++;
		}

		ImGui::End();
#endif

		//ImGui::PopFont();
	}

	void TechScreen() {
		ImGui::Begin("Technology");

		ImGui::Text("---Place Conveyor Belt---");

		if (fancyGraphics) ImGui::PushFont(Engine::Instance().getFont());

		//TOP ROW
		if (ImGui::Button("O")) {
			*game.mainMap.TileAtPos(player.coords) = tileByID[ID_CONVEYOR_UL];
		}
		ImGui::SameLine();
		if (ImGui::Button("S")) {
			*game.mainMap.TileAtPos(player.coords) = tileByID[ID_CONVEYOR_R];
		}
		ImGui::SameLine();
		if (ImGui::Button("P")) {
			*game.mainMap.TileAtPos(player.coords) = tileByID[ID_CONVEYOR_UR];
		}

		//MIDDLE ROW
		if (ImGui::Button("M")) {
			*game.mainMap.TileAtPos(player.coords) = tileByID[ID_CONVEYOR_U];
		}
		ImGui::SameLine();
		if (ImGui::Button(" ")) { }
		ImGui::SameLine();
		if (ImGui::Button("N")) {
			*game.mainMap.TileAtPos(player.coords) = tileByID[ID_CONVEYOR_D];
		}

		//BOTTOM ROW
		if (ImGui::Button("Q")) {
			*game.mainMap.TileAtPos(player.coords) = tileByID[ID_CONVEYOR_DL];
		}
		ImGui::SameLine();
		if (ImGui::Button("T")) {
			*game.mainMap.TileAtPos(player.coords) = tileByID[ID_CONVEYOR_L];
		}
		ImGui::SameLine();
		if (ImGui::Button("R")) {
			*game.mainMap.TileAtPos(player.coords) = tileByID[ID_CONVEYOR_DR];
		}

		if (fancyGraphics) ImGui::PopFont();
		ImGui::End();
	}

	void DisplayEntity(Entity* ent) 
	{
		if (showDialogue) {
			ImGui::Begin("Dialogue");
			ImGui::Text(ent->message.c_str());

			ImGui::Text("\ITEMS:");
			if (ent->inv.size() > 0 && ent->lootAlive || ent->health <= 0) {
				for (int i = 0; i < ent->inv.size(); i++)
				{
					ImGui::Text(ent->inv[i].name.c_str());
					ImGui::Text(ent->inv[i].description.c_str());
					if (ImGui::Button("3", { 20, 20 })) {
						pInv.AddItemFromFile("items.eid", "BAD_PISTOL");
					}
					ImGui::Text("--------------------");
				}
			}
			else {
				ImGui::Text("Empty");
			}
			ImGui::End();
		}

		if (Input::KeyDown(KEY_D)) {
			showDialogue = !showDialogue;
		}
	}



	void Shutdown() override{
		if (currentState == playing) {
			SaveData dat;
			dat.floats.append("color_r", pInv.clothes.x);
			dat.floats.append("color_g", pInv.clothes.y);
			dat.floats.append("color_b", pInv.clothes.z);
			dat.floats.append("health", player.health);
			dat.floats.append("thirst", player.thirst);
			dat.floats.append("hunger", player.hunger);
			dat.ints.append("x_pos", player.coords.x);
			dat.ints.append("y_pos", player.coords.y);
			dat.ints.append("seed", map.landSeed);
			dat.ints.append("biomes", map.biomeSeed);
			dat.ints.append("global_x", map.CurrentChunk()->globalChunkCoord.x);
			dat.ints.append("global_y", map.CurrentChunk()->globalChunkCoord.y);

			Console::Log(map.CurrentChunk()->globalChunkCoord, SUCCESS, __LINE__);

			for (int i = 0; i < pInv.items.size(); i++)
			{
				dat.items.append(pInv.items[i].section, pInv.items[i].count);
			}

			ItemReader::SaveDataToFile("dat/eid/save.eid", "STATS", dat, true);

			LAZY_LOG("Now saving...");


			for (auto& chunk : game.mainMap.world.chunks) {
				chunk.second->SaveChunk();
			}

			LAZY_LOG("Save complete.");
		}
	}
};

class Falling_Sand : public App {

	Vector2_I cursor_pos = {10,10};

	WindowProperties GetWindowProperties() {
		WindowProperties props;
		props.imguiProps = { true, true, false, DOSFONT };
		props.w = 800;
		props.h = 600;
		props.title = "Sand";
		props.cc = { 0.2f, 0.2f, 0.2f, 1.f };
		return props;
	}

	std::vector<std::vector<int>> map, changed_map;
	int updates = 0;


	void Init() override {
		for (size_t i = 0; i < 30; i++)
		{
			map.push_back({});
			for (size_t j = 0; j < 30; j++)
			{
				map[i].push_back(0);
			}
		}
		changed_map = map;
		map[15][15] = 1;
		map[17][15] = 1;
		map[19][15] = 1;
	}

	void Update() override {
		if (Input::KeyDown(KEY_SPACE)) {
			changed_map[cursor_pos.x][cursor_pos.y] = 1;
		}
		if (Input::KeyDown(KEY_UP)) {
			cursor_pos.x -= 1;
		}
		if (Input::KeyDown(KEY_DOWN)) {
			cursor_pos.x += 1;
		}
		if (Input::KeyDown(KEY_LEFT)) {
			cursor_pos.y -= 1;
		}
		if (Input::KeyDown(KEY_RIGHT)) {
			cursor_pos.y += 1;
		}

		updates++;
		if (updates < 30) { return; }
		updates = 0;
		for (size_t i = 0; i < map.size(); i++)
		{
			for (size_t j = 0; j < map[i].size(); j++)
			{
				if (map[i][j] == 1 && i - 1 >= 0 && i + 1 < 30) {
					if (map[i + 1][j] != 1) {
						changed_map[i + 1][j] = 1;
						changed_map[i][j] = 0;
					}
					else if (map[i + 1][j + 1] != 1) {
						changed_map[i + 1][j + 1] = 1;
						changed_map[i][j] = 0;
					}
					else if (map[i + 1][j - 1] != 1) {
						changed_map[i + 1][j - 1] = 1;
						changed_map[i][j] = 0;
					}
					else {
						continue;
					}
				}
			}
		}
		map = changed_map;
	}

	void ImguiRender() override {

		ImGui::PushFont(Engine::Instance().getFont());
		ImGui::Begin("Game View");
			for (int i = 0; i < map.size(); i++)
			{ 
				for (int j = 0; j < map[i].size(); j++)
				{
					if (Vector2_I{ i, j } == cursor_pos) {
						ImGui::Text("X"); ImGui::SameLine();
					}
					else if (map[i][j] == 0) {
						ImGui::TextColored({ 0.2f, 0.2f, 0.2f, 1.0f }, "J"); ImGui::SameLine();
					}
					else {
						ImGui::TextColored({ 1.0f, 1.0f, 0.f, 1.0f }, "J"); ImGui::SameLine();
					}
				}
				ImGui::NewLine();
			}
		ImGui::End();
		ImGui::PopFont();
	}
	void Shutdown() override {

	}
};

class Sprites : public App {
	WindowProperties GetWindowProperties() {
		WindowProperties props;
		props.w = 800;
		props.h = 600;
		props.title = "Sprites";
		props.cc = { 0.2f, 0.2f, 0.2f, 1.f };
		props.framebuffer_display = true;
		return props;
	}

	Scene main = { "Level 1" };

	void Init() override {
		Engine::Instance().AddScene(&main);
		main.CreateObject("Player", { 0,0 }, { 0.25,0.25 }, "res/image.png");
	}
	void Update() override {


		if (Input::KeyDown(KEY_SEMICOLON)) {
			Engine::Instance().SetApp(0);
		}

		if (Input::KeyDown(KEY_W)) {
			main.FindObject("Player")->Move({0.5f, 0.0f});
		}
		if (Input::KeyDown(KEY_S)) {

		}
		if (Input::KeyDown(KEY_A)) {

		}
		if (Input::KeyDown(KEY_D)) {

		}
	}
	void Render() override {

	}

	void ImguiRender() override {
		ImGui::Begin("Object Data");

		ImGui::Text("Player Coords:");
		ImGui::Text(std::to_string(main.FindObject("Player")->GetPos().x).c_str());
		ImGui::Text(std::to_string(main.FindObject("Player")->GetPos().y).c_str());

		ImGui::End();
	}
	void Shutdown() override {

	}
};



std::vector<App*> CreateGame() {
	return {new Caves, new Sprites};
}