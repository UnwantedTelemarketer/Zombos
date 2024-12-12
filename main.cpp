#include "dat/game.h"
#include "dat/uiscreen.h"
#include <algorithm>
#include <thread>

#include <chrono>

#define DOSFONT "dat/fonts/symbolic/symbolic_crystal_extended.ttf"
#define ITEMFONT "dat/fonts/symbolic/symbolic_item.ttf"
//#define DEV_TOOLS

using namespace antibox;

enum GameState { playing, menu };
static char console_commands[128] = "";

class Caves : public App {

private:
	WindowProperties GetWindowProperties() {
		WindowProperties props;

		props.imguiProps = { true, true, false, {DOSFONT, ITEMFONT}, {"main", "items"}, 16.f };
		props.w = 1280;
		props.h = 720;
		props.vsync = 0;
		props.cc = { 0.5, 0.5, 0.5, 1 };
		props.framebuffer_display = false;
		return props;
	}

	float tickRateVisual, lastFPS;
	int counter = 0;
	GameState currentState;
	vec3 clothes;
	std::string printIcon;
	ImVec4 iconColor;
	std::vector<Tile> itemTiles;
	std::vector<std::string> itemIcons;
	std::vector<ImVec2> itemPositions;
public:
	//UI stuff
	GameUI gameScreen;
	Tile* selectedTile = nullptr;
	int currentItemIndex = 0;
	std::string openClose;
	antibox::framerate frame;
	std::vector<float> frametimes;
	float colChangeTime = 0.f;
	float volume = 1.f;
	bool interacting, flashlightActive;

	//Game Stuff
	Commands cmd;
	GameManager game;
	Inventory& pInv = game.pInv;
	Player& player = game.mPlayer;
	direction playerDir = direction::down;
	float& health = game.mPlayer.health;
	Map& map = game.mainMap;
	std::vector<Vector2_I> item_positions;

	//Crafting Stuff
	int recipeSelected, itemSelected = 0;
	std::string recipeSelectedName, itemSelectedName, invSelectedName = "";

	//Sounds
	std::map<std::string, const char*> sfxs = {
		{"craft", "dat/sounds/craft.wav"},
		{"fail", "dat/sounds/fail.wav"},
		{"collect", "dat/sounds/bckp.wav"},
		{"ui_select", "dat/sounds/ui_confirm.wav"},
		{"splash", "dat/sounds/enter_water.wav"},
		{"click", "dat/sounds/click.wav"},
		{"crunch", "dat/sounds/eat.wav"}
	};


	//Sprites
	Scene main = { "TEST" };
	std::shared_ptr<GameObject> p;

	void Init() override {
		std::thread itemLoading = Items::LoadItemsFromFiles(&game.item_icons);
		std::thread tileLoading = Tiles::LoadTilesFromFiles(&game.tile_colors);

		Engine::Instance().AddScene(&main);
		main.CreateObject("Box", { 0,0 }, { 1,1 }, "res/plank.png");
		p = main.FindObject("Box");

		gameScreen.fancyGraphics = true;
		frame.frames.length = 360;
		currentState = menu;
		health = 100.0f;
		gameScreen.statsOpen = true;
		openClose = "Close Stats";
		gameScreen.containerOpen = false;
		gameScreen.navInv = false;
		gameScreen.showDialogue = true;
		gameScreen.console_showing = false;
		gameScreen.helpMenu = true;

		player.currentWeapon.mod = 5;

		itemLoading.join();
		tileLoading.join();
		Console::Log("Done!", text::green, __LINE__);
	}

	void Update() {
		bool moved = false;
		colChangeTime += Utilities::deltaTime() / 1000;
		if (colChangeTime >= 1.f) {
			colChangeTime = 0;
		}
		//the rest of the update is game logic so we stop here in the menu
		if (currentState == menu) { return; }

		//-=================================================================

		if (player.health <= 0) { currentState = menu; player.health = 100; }

		if (Input::KeyDown(KEY_ENTER)) {
			Console::Log(console_commands, text::green, __LINE__);
			cmd.RunCommand(console_commands, &pInv, &player);
			console_commands[0] = '\0';
		}

		gameScreen.FlipScreens();

		if (gameScreen.console_showing) { return; }


		game.UpdateTick();

		if (Input::KeyDown(KEY_UP) || Input::KeyDown(KEY_W)) {
			if (interacting)
			{
				selectedTile = { map.TileAtPos(Vector2_I{ player.coords.x - 1, player.coords.y }) };
				interacting = false;
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
				playerDir = direction::up;
			}
		}
		else if (Input::KeyDown(KEY_DOWN) || Input::KeyDown(KEY_S)) {
			if (interacting)
			{
				Tile& selTile = *map.TileAtPos(Vector2_I{ player.coords.x + 1, player.coords.y });
				selectedTile = { &selTile };
				if (selTile.entity != nullptr) {

				}
				interacting = false;
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
				playerDir = direction::down;
			}
		}
		else if (Input::KeyDown(KEY_LEFT) || Input::KeyDown(KEY_A)) {
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
				playerDir = direction::left;
			}
		}
		else if (Input::KeyDown(KEY_RIGHT) || Input::KeyDown(KEY_D)) {
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
				playerDir = direction::right;
			}
		}

		

		else if (Input::KeyDown(KEY_E))
		{
			if (!gameScreen.navInv)
			{
				Math::PushBackLog(&game.actionLog, "Which direction will you interact with?");
				interacting = true;
			}
		}

		else if (Input::KeyDown(KEY_F)) {
			flashlightActive = !flashlightActive;
			Audio::Play(sfxs["click"]);
		}


		if (moved) {
			Audio::Play(game.GetWalkSound());
		}

		if (player.aiming) {
			map.ClearLine();
			map.DrawLine(map.GetLine(player.coords, player.crosshair, 25));
		};
	}

	void ImguiRender() override
	{
		//GameScene();
		if (gameScreen.settingsOpen) {
			ImGui::Begin("Settings");

			ImGui::SliderFloat("Volume Level", &volume, 0.f, 1.f);
			if (ImGui::Button("Set Volume")) {
				Audio::SetVolume(volume);
				Audio::Play(sfxs["ui_select"]);
			}

			ImGui::End();
		}
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

		//ImGui::ShowDemoWindow();
		ImGui::Begin("title");
		ImGui::SetFontSize(48.f);
		ImGui::Text("Los Zombos");
		ImGui::SetFontSize(16.f);
		ImGui::End();

		ImGui::Begin("Menu");
		if (gameScreen.createChar) { Create_Character(); }


		ImGui::SliderFloat("Volume Level", &volume, 0.f, 1.f);
		if (ImGui::Button("Set Volume")) {
			Audio::SetVolume(volume); 
			Audio::Play(sfxs["ui_select"]);
		}

		if (ImGui::Button("New Game"))
		{
			game.Setup(10, 10, 0.5f);

			gameScreen.createChar = true;
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
				for (int s = 0; s < stoi(data.getArray("items")[i + 1]); s++)
				{
					pInv.AddItemFromFile(data.getArray("items")[i]); //give it a certain amount of items
				}
				i++;
			}
			game.Setup(data.getInt("x_pos"), data.getInt("y_pos"), 0.5f, data.getInt("seed"), data.getInt("biomes"));
			currentState = playing;
			game.worldTime = data.getFloat("time");
		}
		ImGui::End();
	}

	void Create_Character() {
		ImGui::Begin("Create your Character");

		ImGui::ColorPicker3("Clothes Color", &clothes.x);

		pInv.clothes = { clothes.x, clothes.y, clothes.z };
		if (ImGui::Button("Start")) {
			pInv.AddItemFromFile("BANDAGE");
			pInv.AddItemFromFile("LIGHTER");
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
		if (gameScreen.console_showing) {
			ImGui::Begin("Console");
			ImGui::InputTextWithHint("console", "enter commands", console_commands, IM_ARRAYSIZE(console_commands));
			ImGui::End();
		}
		//------Map rendering-------
		if (game.worldTime >= 20.f || game.worldTime < 6.f) { ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.0,0.0,0.0,1 }); }
		else{ ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ game.backgroundColor.x,game.backgroundColor.y,game.backgroundColor.z,1 }); }
		ImGui::Begin("Map");
		if (gameScreen.fancyGraphics) { ImGui::PushFont(Engine::Instance().getFont("main")); }

		bool item = false;
		ImVec2 playerPos;
		for (int i = 0; i < CHUNK_HEIGHT; i++) {
			for (int j = 0; j < CHUNK_WIDTH; j++) {

				
				float intensity = map.CurrentChunk()->localCoords[i][j].brightness;
				Tile& curTile = map.CurrentChunk()->localCoords[i][j];
				Tile& underTile = map.CurrentChunk()->localCoords[i + 1][j];

				//Flashlight pyramid logic, idk weird math stuff. pack it away please
				{//===================================================================
					float newBrightness = 1.0f;
					if ((i <= player.coords.x && i >= player.coords.x - 10)
						&& (j >= player.coords.y - ((player.coords.x) - i)
						&& j <= player.coords.y + ((player.coords.x) - i))
						&& flashlightActive
						&& playerDir == direction::up) {
						newBrightness = 0.1f *(player.coords.x - i);
					}
					else if ((i >= player.coords.x && i <= player.coords.x + 10)
						&& (j >= player.coords.y - ((i - player.coords.x))
						&& j <= player.coords.y + (i - (player.coords.x)))
						&& flashlightActive
						&& playerDir == direction::down) {
						newBrightness = 0.1f *(i - player.coords.x);
					}
					else if ((j <= player.coords.y && j >= player.coords.y - 10)
						&& (i >= player.coords.x - ((player.coords.y) - j)
						&& i <= player.coords.x + ((player.coords.y) - j))
						&& flashlightActive
						&& playerDir == direction::left) {
						newBrightness = 0.1f *(player.coords.y - j);
					}
					else if ((j >= player.coords.y && j <= player.coords.y + 10)
						&& (i >= player.coords.x - ((j - player.coords.y))
						&& i <= player.coords.x + (j - (player.coords.y)))
						&& flashlightActive
						&& playerDir == direction::right) {
						newBrightness = 0.1f * (j - player.coords.y);
					}
					intensity = newBrightness < intensity ? newBrightness : intensity;
					if (intensity < 0.f) intensity = 0.f;
					if (intensity > 1.f) { intensity = 1.f; }
				}//===================================================================
				
				if (Vector2_I{ player.coords.x,player.coords.y } == Vector2_I{ i,j })
				{
					//screen += ENT_PLAYER;
					//colors.push_back(game.GetPlayerColor());
					printIcon = ENT_PLAYER;
					iconColor = game.GetPlayerColor();
				}

				else if (map.effectLayer.localCoords[i][j] == 1)
				{
					//screen += "?";
					//colors.push_back(ImVec4{ 0.65,0.65,0.65,1 });
					printIcon = "?";
					iconColor = Cosmetic::SmokeColor();
				}

				else {
					printIcon = game.GetTileChar(curTile);
					iconColor = game.GetTileColor(curTile, intensity);
				}

				if (i < CHUNK_HEIGHT - 1) {
					Entity* curEnt = underTile.entity;

					if (curEnt != nullptr) {
						if (curEnt->targeting() && curEnt->health > 0) {
							//screen += "!";
							//colors.push_back(ImVec4{ 1,0,0,1 });
							printIcon = "!";
							iconColor = ImVec4{ 1,0,0,1 };
						}
					}
					if (underTile.id == 11) {
						//screen += "G";
						//colors.push_back(game.GetTileColor(underTile, intensity));
						if(printIcon != ENT_PLAYER) printIcon = "G";
						iconColor = game.GetTileColor(underTile, intensity);
					}
					if (underTile.id == 12) {
						//screen += "J";
						//colors.push_back(game.GetTileColor(underTile, intensity));
						if (printIcon != ENT_PLAYER) printIcon = "J";
						iconColor = game.GetTileColor(underTile, intensity);
					}
				}

				if (curTile.hasItem)
				{
					item = true;
					itemPositions.push_back(ImGui::GetCursorPos());
					itemTiles.push_back(curTile);
					itemIcons.push_back(game.GetItemChar(curTile));
					ImGui::Text(" ");
					ImGui::SameLine();
					continue;
				}


				//screen += game.GetTileChar(curTile);
				//colors.push_back(game.GetTileColor(curTile, intensity));

				ImGui::TextColored(iconColor, printIcon.c_str());
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 0.05);
			}
			ImGui::Text("");
		}


		if (item) {
			ImGui::PopFont();
			ImGui::PushFont(Engine::Instance().getFont("items"));
			for (size_t i = 0; i < itemIcons.size(); i++)
			{
				ImGui::SetCursorPos(itemPositions[i]);

				ImGui::TextColored(game.GetItemColor(itemTiles[i]), itemIcons[i].c_str());
			}
			itemTiles.clear();
			itemIcons.clear();
			itemPositions.clear();
		}

		if (gameScreen.fancyGraphics) ImGui::PopFont();

		ImGui::End();
		ImGui::PopStyleColor(1);

		TechScreen();
		
		if (!game.mainMap.isUnderground) {
			if (ImGui::Button("Go Down")) {
				if (game.EnterCave()) {
					Math::PushBackLog(&game.actionLog, "You enter a dark cave underground.");
				}
			}
		}
		else {
			if (ImGui::Button("Go Up")) {
				game.mainMap.isUnderground = !game.mainMap.isUnderground;
			}
		}

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
		if (gameScreen.statsOpen) {
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

			if (player.coveredIn != nothing) {
				ImVec4 color = Cosmetic::CoveredColor((int)player.coveredIn);
				std::string text = "Covered in: ";
				text += Cosmetic::CoveredName((int)player.coveredIn);
				ImGui::TextColored(color, text.c_str());
				ImGui::ProgressBar((float)(player.liquidLast-player.ticksCovered) / player.liquidLast, ImVec2(0.0f, 0.0f));
			}
			ImGui::Text("-------------");
			ImGui::Text("Currently Equipped Weapon :"); ImGui::SameLine();
			ImGui::Text(player.currentWeapon.name.c_str());
			ImGui::Text("Damage :"); ImGui::SameLine();
			ImGui::Text(std::to_string(player.currentWeapon.mod).c_str());

			ImGui::End();

		}
		//------Inventory------
		ImGui::Begin("Inventory");
		ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.15, 0.15, 0.15, 1});
		if (ImGui::BeginListBox("Inventory"))
		{
			for (int n = 0; n < pInv.items.size(); n++)
			{
				const bool is_selected = (currentItemIndex == n);
				if (pInv.items[n].coveredIn != nothing)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, Cosmetic::CoveredColor(pInv.items[n].coveredIn));
				}
				if (ImGui::Selectable((*pInv.GetItemNames())[n].c_str(), is_selected)) {
					invSelectedName = (*pInv.GetItemNames())[n];
					currentItemIndex = n;
				}
				if (pInv.items[n].coveredIn != nothing)
				{
					ImGui::PopStyleColor(1);
				}
			}
			ImGui::EndListBox();
		}
		ImGui::PopStyleColor(1);
		ImGui::End();


		//------item------
		if (invSelectedName != "")
		{
			ImGui::Begin("Current Item");
			ImGui::Text((pInv.items[currentItemIndex].name + "\n\n").c_str());

			ImGui::PushFont(Engine::Instance().getFont("items"));
			ImGui::SetFontSize(32.f);
			ImGui::Text(game.item_icons[pInv.items[currentItemIndex].section].c_str());
			ImGui::SetFontSize(16.f);
			ImGui::PopFont();

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
				ImGui::Text("Time until dry:");
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
				gameScreen.useBool = !gameScreen.useBool;
			}
			if (pInv.items[currentItemIndex].eType != notEquip) {
				if (ImGui::Button("Equip"))
				{
					player.currentWeapon = pInv.items[currentItemIndex];
				}
			}

			if (gameScreen.useBool)
			{
				ImGui::Text("On what?");
				if (ImGui::Button("Consume"))
				{
					if (pInv.AttemptAction(consume, &pInv.items[currentItemIndex], &player))
					{
						Audio::Play(sfxs["crunch"]);
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
					gameScreen.useBool = !gameScreen.useBool;
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
					gameScreen.useBool = !gameScreen.useBool;
				}
			}

			ImGui::End();
		}

		//------Crafting-------
		if (gameScreen.craftingMenu) {
			ImGui::Begin("Crafting");

			if (ImGui::BeginListBox("Recipes"))
			{
				for (int n = 0; n < game.recipeNames.size(); n++)
				{
					const bool is_selected = (recipeSelected == n);
					if (ImGui::Selectable(game.recipeNames[n].c_str(), is_selected)) {
						recipeSelectedName = game.recipeNames[n];
						recipeSelected = n;
					}
				}
				ImGui::EndListBox();
			}
			if (recipeSelectedName != "") {
				std::vector<std::string> components = game.Crafter.getRecipeComponents(recipeSelectedName);
				ImGui::PushFont(Engine::Instance().getFont("items"));
				ImGui::SetFontSize(32.f);
				ImGui::Text(("\n" + game.item_icons[recipeSelectedName]).c_str());
				ImGui::SetFontSize(16.f);
				ImGui::PopFont();
				ImGui::Text(" - Required Components - ");
				for (size_t i = 0; i < components.size(); i++)
				{
					ImGui::Text(components[i].c_str());
				}
				if (ImGui::Button("Craft Item")) {
					std::string newItem = game.Crafter.AttemptCraft(recipeSelectedName, &pInv.items);

					if (newItem != "none") {
						currentItemIndex = 0;
						pInv.Cleanup();
						pInv.AddItem(Items::GetItem(newItem));
						Audio::Play(sfxs["craft"]);
					}
					else {
						Audio::Play(sfxs["fail"]);
					}
				}
			}
			ImGui::End();
		}

		// Help Menu

		if (gameScreen.helpMenu) {
			ImGui::Begin("Help Menu");
			ImGui::Text("WASD or Arrow keys to move");
			ImGui::Text("F to toggle flashlight on and off");
			ImGui::Text("C to open the crafting menu");
			ImGui::Text("E to begin selecting a block then any of the directional keys to select a block");
			ImGui::Text("P to open/close the Debug menu");
			ImGui::Text("H to open/close the Help menu");
			ImGui::Text("ESC to open/close the Settings menu");
			ImGui::Text("` to open/close the console");
			ImGui::Text("\nTo place an item in a container, click \n'Drop Selected Item' while interacting with the container.");
			ImGui::Text("\nTo attack an entity, just walk into it.");
			ImGui::End();
		}

		//------Interaction------
		if (selectedTile != nullptr)
		{
			ImGui::Begin("Selected Block");

			Container* curCont = game.mainMap.ContainerAtCoord(selectedTile->coords);

			ImGui::PushFont(Engine::Instance().getFont("main"));
			if (curCont != nullptr) {
				ImGui::TextColored(ImVec4{ 0.5, 0.34, 0, 1 }, "U");
			}
			else {
				ImGui::TextColored(game.GetTileColor(*selectedTile, 1.f), game.GetTileChar(*selectedTile).c_str());
			}
			ImGui::PopFont();


			if (curCont != nullptr) {
				std::string text = gameScreen.containerOpen ? "Close Container" : "Open Container";
				if (ImGui::Button(text.c_str())) { gameScreen.containerOpen = !gameScreen.containerOpen; }
		if (gameScreen.containerOpen) {
			std::vector<std::string> names = curCont->getItemNames();
			if (ImGui::BeginListBox("Items"))
			{
				for (int n = 0; n < names.size(); n++)
				{
					const bool is_selected = (recipeSelected == n);
					if (ImGui::Selectable(names[n].c_str(), is_selected)) {
						itemSelectedName = names[n];
						recipeSelected = n;
					}
				}
				ImGui::EndListBox();

		if (itemSelectedName != "") {
			if (ImGui::Button(("Collect " + itemSelectedName).c_str())) {
				pInv.AddItem(curCont->items[recipeSelected]);
				curCont->items.erase(curCont->items.begin() + recipeSelected);
				itemSelectedName = "";
							}
						}
					}
				}
			}

			if (selectedTile->entity != nullptr && selectedTile->entity->health <= 0) {
				std::string text = gameScreen.containerOpen ? "Close Container" : "Open Container";
				if (ImGui::Button(text.c_str())) { gameScreen.containerOpen = !gameScreen.containerOpen; }
				if (gameScreen.containerOpen) {
					std::vector<std::string> names = selectedTile->entity->getItemNames();
					if (ImGui::BeginListBox("Items"))
					{
						for (int n = 0; n < names.size(); n++)
						{
							const bool is_selected = (recipeSelected == n);
							if (ImGui::Selectable(names[n].c_str(), is_selected)) {
								itemSelectedName = names[n];
								itemSelected = n;
							}
						}
						ImGui::EndListBox();

						if (itemSelectedName != "") {
							if (ImGui::Button(("Collect " + itemSelectedName).c_str())) {
								pInv.AddItem(selectedTile->entity->inv[recipeSelected]);
								selectedTile->entity->inv.erase(selectedTile->entity->inv.begin() + recipeSelected);
								itemSelectedName = "";
							}
						}
					}
				}
			}

			if (selectedTile->hasItem) { ImGui::Text(("Item on tile: " + selectedTile->itemName).c_str()); }

			if (ImGui::Button("Drop Selected Item")) {
				//Dropping it into a box
				Container* curCont = game.mainMap.ContainerAtCoord(selectedTile->coords);
				if (curCont != nullptr) {
					//add the item into the box only if theres enough space, and remove it from the inventory
					if (curCont->AddItem(pInv.items[currentItemIndex])) {
						if (pInv.RemoveItem(pInv.items[currentItemIndex].id, pInv.items[currentItemIndex].count)) { currentItemIndex = 0; }
					}
				}
				//Or on the floor
				else {
					if (invSelectedName != "" && !selectedTile->hasItem) {
						selectedTile->hasItem = true;
						std::string upperName = pInv.items[currentItemIndex].name;

						for (auto& c : upperName) c = toupper(c);

						selectedTile->itemName = upperName;
						selectedTile->collectible = true;
						if (selectedTile->itemName == "CAMPFIRE" || selectedTile->itemName == "CHEST") {
							game.mainMap.CreateContainer({ selectedTile->coords });
						}
						if (pInv.RemoveItem(pInv.items[currentItemIndex].id)) { currentItemIndex = 0; }
					}
					else {
						Math::PushBackLog(&game.actionLog, "There is already an item on that space.");
					}
				}
			}

			//grody ahh pyramid
			bool canCollect = true;
			if (selectedTile->collectible || selectedTile->liquid != nothing || selectedTile->hasItem) {
				if (ImGui::Button("Collect")) {
					if (game.mainMap.ContainerAtCoord(selectedTile->coords) != nullptr
						&& game.mainMap.ContainerAtCoord(selectedTile->coords)->items.size() != 0)
					{
						Math::PushBackLog(&game.actionLog, "This container still contains items.");
						canCollect = false;
					}
					else {
						canCollect = true;
					}
					if (canCollect) {
						if (pInv.AttemptCollect(selectedTile))
						{
							if (selectedTile->itemName == "CAMPFIRE" || selectedTile->itemName == "CHEST") {
								game.mainMap.RemoveContainer({ selectedTile->coords });
							}
							Audio::Play(sfxs["collect"]);
							Math::PushBackLog(&game.actionLog, "You collect an item off the ground.");
						}
						else {
							Math::PushBackLog(&game.actionLog, "You can't collect that.");
						}
					}
					selectedTile = nullptr;
				}
			}

			if (ImGui::Button("Burn")) {
				selectedTile->liquid = fire;

				map.floodFill(selectedTile->coords, 5, false);
			}

			int _ = 0;
			if (pInv.TryGetItem("rock", false, &_)) {
				if (ImGui::Button("Place Wall")) {
					*selectedTile = tileByID[ID_STONE];
					pInv.RemoveItem("rock");
				}
			}

			ImGui::End();
		}

		//------DEBUG------
		if (gameScreen.debugOpen) {
			ImGui::Begin("Debug Window");

			ImGui::Text(("Current World Time: " + std::to_string(game.worldTime)).c_str());
			ImGui::Text(("Dark Time: " + std::to_string(game.darkTime)).c_str());
			//FPS

			lastFPS = Utilities::getFPS();
			frame.Update(Utilities::getFPS());
			ImGui::Text(("High: " + std::to_string(frame.highest)).c_str());
			ImGui::Text(("Low: " + std::to_string(frame.lowest)).c_str());
			ImGui::Text(("Average: " + std::to_string(lastFPS)).c_str());
			//ImGui::PlotLines("Frame Times", frame.frames.c_arr(), frame.frames.length);
			//ms left until next tick
			ImGui::Text(("Time until next update:"));
			//Display a bar until next tick
			ImGui::ProgressBar(game.GetTick() / game.TickRate(), ImVec2(0.0f, 0.0f));
			//set tickrate
			ImGui::InputFloat("New Tickrate", &tickRateVisual, 0.1f, 10);
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
		for (int i = 0; i < CHUNK_WIDTH; i++) {
			for (int j = 0; j < CHUNK_HEIGHT; j++) {
				std::string c = std::to_string((int)ceil(map.CurrentChunk()->localCoords[i][j].brightness * 10));
				if (c != "10") { c = "0" + c; }
				switch (stoi(c)) {
				case 1:
					ImGui::TextColored({ 1, 0, 0, 1 }, c.c_str());
					break;
				case 2:
					ImGui::TextColored({ 0.9, 0.1, 0, 1 }, c.c_str());
					break;
				case 3:
					ImGui::TextColored({ 0.8, 0.2, 0, 1 }, c.c_str());
					break;
				case 4:
					ImGui::TextColored({ 0.7, 0.3, 0, 1 }, c.c_str());
					break;
				case 5:
					ImGui::TextColored({ 0.6, 0.4, 0, 1 }, c.c_str());
					break;
				case 6:
					ImGui::TextColored({ 0.5, 0.5, 0, 1 }, c.c_str());
					break;
				case 7:
					ImGui::TextColored({ 0.4, 0.6, 0, 1 }, c.c_str());
					break;
				case 8:
					ImGui::TextColored({ 0.3, 0.7, 0, 1 }, c.c_str());
					break;
				case 9:
					ImGui::TextColored({ 0.2, 0.8, 0, 1 }, c.c_str());
					break;
				case 10:
					ImGui::TextColored({ 0.35, 0.35, 0.35, 1 }, c.c_str());
					break;
				}
				ImGui::SameLine();
			}
			ImGui::Text("");
		}
		ImGui::End();
#endif

		//ImGui::PopFont();
	}

	void TechScreen() {
		ImGui::Begin("Technology");

		ImGui::Text("---Place Conveyor Belt---");

		if (gameScreen.fancyGraphics) ImGui::PushFont(Engine::Instance().getFont("main"));

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
		if (ImGui::Button(" ")) {}
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

		if (gameScreen.fancyGraphics) ImGui::PopFont();
		ImGui::End();
	}

	void DisplayEntity(Entity* ent)
	{
		if (gameScreen.showDialogue) {
			ImGui::Begin("Dialogue");
			ImGui::Text(ent->message.c_str());

			ImGui::Text("\ITEMS:");
			if (ent->inv.size() > 0 && ent->lootAlive || ent->health <= 0) {
				for (int i = 0; i < ent->inv.size(); i++)
				{
					ImGui::Text(ent->inv[i].name.c_str());
					ImGui::Text(ent->inv[i].description.c_str());
					if (ImGui::Button("3", { 20, 20 })) {
						pInv.AddItemFromFile("BAD_PISTOL");
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
			gameScreen.showDialogue = !gameScreen.showDialogue;
		}
	}



	void Shutdown() override {
		if (currentState == playing) {
			float curTime = glfwGetTime();
			LAZY_LOG("Now saving...");
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
			dat.floats.append("time", game.worldTime);

			//Console::Log(map.CurrentChunk()->globalChunkCoord, SUCCESS, __LINE__);

			for (int i = 0; i < pInv.items.size(); i++)
			{
				dat.items.append(pInv.items[i].section, pInv.items[i].count);
			}

			ItemReader::SaveDataToFile("dat/eid/save.eid", "STATS", dat, true);

			for (auto& chunk : game.mainMap.world.chunks) {
				chunk.second->SaveChunk();
			}

			float endTime = glfwGetTime() - curTime;

			Console::Log("Save complete in " + std::to_string(endTime * 1000) + "ms.", text::green, __LINE__);
		}
	}
};

std::vector<App*> CreateGame() {
	return { new Caves };
}