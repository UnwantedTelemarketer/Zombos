#include "dat/Game.h"
#include "dat/uiscreen.h"
#include <algorithm>
#include <thread>

#include <chrono>

#define DOSFONT  "dat/fonts/symbolic/symbolic_chainfence.ttf"
#define ITEMFONT "dat/fonts/symbolic/symbolic_items_extended.ttf"
#define MOBFONT "dat/fonts/symbolic/symbolic_mobs_extended.ttf"
#define VGAFONT  "dat/fonts/VGA437.ttf"
#define SWAP_FONT(newfont) ImGui::PopFont(); ImGui::PushFont(Engine::Instance().getFont(newfont));
//#define DEV_TOOLS

using namespace antibox;

enum GameState { playing, menu, map_gen_test };
static char console_commands[128] = "";
int prevCommandIndex = 0;

class Caves : public App {

private:
	WindowProperties GetWindowProperties() {
		WindowProperties props;

		props.imguiProps = { true, true, false, {DOSFONT, ITEMFONT, VGAFONT, MOBFONT}, {"main", "items", "ui", "mobs"}, 16.f };
		props.w = 1340;
		props.h = 720;
		props.vsync = 0;
		props.cc = { 0.5, 0.5, 0.5, 1 };
		props.framebuffer_display = false;
		return props;
	}

	float tickRateVisual = 0.f, lastFPS = 0.f;
	int counter = 0;
	GameState currentState;
	vec3 clothes = { 1,0,0 };
	std::string printIcon = "";
	ImVec4 iconColor = { 1,0,0,0 };
	std::vector<Tile*> itemTiles;
	std::vector<std::string> itemIcons;
	std::vector<ImVec2> itemPositions;
	std::vector<std::string> mobIcons;
	std::vector<ImVec2> mobPositions;
	std::vector<ImVec4> mobColors;
	vec2_i view_distance = { 15,15 };

	std::shared_ptr<Chunk> customBuilding;
public:
	//UI stuff
	GameUI gameScreen;
	Tile* selectedTile = nullptr;
	int currentItemIndex = 0;
	int ySeparator = 2;
	std::string openClose = "";
	antibox::framerate frame;
	std::vector<float> frametimes;
	float colChangeTime = 0.f;
	float sfxvolume = 1.f;
	float musicvolume = 1.f;
	bool interacting = false, flashlightActive = false;
	int xMin, xMax, yMin, yMax;
	int xViewDist, yViewDist;
	char recipe_search[128] = "";
	int customStructHeight = 1;
	int customStructWidth = 1;
	Vector4 customTabColor;
	bool savedGamesScreen = false;
	bool newGameScreen = false;
	bool wrongDir = false;
	char saveNameSlot[128];

	//Game Stuff
	std::shared_ptr<Chunk> test_chunk;
	Commands cmd;
	GameManager game;
	Inventory& pInv = game.pInv;
	Player& player = game.mPlayer;
	direction playerDir = direction::down;
	float& health = game.mPlayer.health;
	Map& map = game.mainMap;
	std::vector<Vector2_I> item_positions;
	Vector2_I cursorPos = { 15, 15 };
	std::string customTileSelect = "TILE_STONE";

	char customMapBuffer[512] = "";

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
		{"crunch", "dat/sounds/eat.wav"},
		{"drink", "dat/sounds/drink.wav"},
		{"crunchy_click", "dat/sounds/crunchy_click.mp3"}
	};


	//Sprites
	Scene main = { "TEST" };
	std::shared_ptr<GameObject> p;

	//Character Creation
	int bgSelected = -1;
	std::vector<classes> backgrounds = 
	{	{"Fighter",		{"MACHETE", "LEATHER_JACKET", "ROCK"}, {1, 1, 10}},
		{"Survivalist", {"MATCH", "RAINCOAT", "BANDAGE"}, {5, 1, 2}},
		{"Hunter",		{"BEAR_TRAP", "LEATHER_BOOTS", "CAMPFIRE"}, {2, 1, 1}},
		{"Explorer",	{"RATION", "LEATHER_BOOTS", "BITS"}, {5, 1, 10}},
		{"Vagrant",		{"ROCK", "STICK", "ROPE"}, {5, 5, 3}},
		{"Amnesiac",	{"MUD"}, {1}},
	};

	//Yelling Text above NPCs

	void ResetMenu() {
		gameScreen.MainMenu();
		bgSelected = -1;

	}

	void SaveCurrentGame() {
		float curTime = glfwGetTime();
		LAZY_LOG("Now saving...");
		SaveData dat;

		//save settings and basic player data
		dat.sections.insert({ "STATS", {} });
		dat.sections.insert({ "SETTINGS", {} });
		dat.addFloat("STATS", "color_r", pInv.clothes.x);
		dat.addFloat("STATS", "color_g", pInv.clothes.y);
		dat.addFloat("STATS", "color_b", pInv.clothes.z);
		dat.addFloat("STATS", "health", player.health);
		dat.addFloat("STATS", "thirst", player.thirst);
		dat.addFloat("STATS", "hunger", player.hunger);
		dat.addInt("STATS", "x_pos", player.coords.x);
		dat.addInt("STATS", "y_pos", player.coords.y);
		dat.addInt("STATS", "biomes", map.biomeSeed);
		dat.addInt("STATS", "seed", map.landSeed);
		dat.addInt("STATS", "global_x", map.CurrentChunk()->globalChunkCoord.x);
		dat.addInt("STATS", "global_y", map.CurrentChunk()->globalChunkCoord.y);
		dat.addFloat("STATS", "time", game.worldTime);
		dat.addInt("STATS", "weather", map.currentWeather);
		dat.addInt("SETTINGS", "ySep", ySeparator);
		dat.addInt("SETTINGS", "xDist", xViewDist);
		dat.addInt("SETTINGS", "yDist", yViewDist);
		dat.addFloat("SETTINGS", "sfxSound", sfxvolume);
		dat.addFloat("SETTINGS", "musicSound", musicvolume);

		std::vector<std::string> listString;
		std::vector<std::string> itemList;

		//save inventory items
		for (int i = 0; i < pInv.items.size(); i++)
		{
			std::string itemName = pInv.items[i].section;
			itemList.push_back(itemName);
			dat.sections.insert({ itemName, {} });
			dat.addInt(itemName, "count", pInv.items[i].count);
			dat.addInt(itemName, "coveredIn", pInv.items[i].coveredIn);
			dat.addInt(itemName, "ticksUntilDry", pInv.items[i].ticksUntilDry);
			dat.addInt(itemName, "initialTickTime", pInv.items[i].initialTickTime);
			dat.addInt(itemName, "durability", pInv.items[i].durability);
			dat.addInt(itemName, "heldLiquid", pInv.items[i].heldLiquid);
			if (pInv.items[i].heldLiquid != nothing) {
				dat.addInt(itemName, "liquidAmount", pInv.items[i].liquidAmount);
			}
		}

		//save equipped items
		for (auto const& eType : Items::EquipmentTypes)
		{
			if (pInv.CurrentEquipExists(eType)) {
				std::string eTypeName = Cosmetic::EquipTypeName(eType);
				dat.sections.insert({ eTypeName, {} });
				dat.addInt(eTypeName, "durability", pInv.equippedItems[eType].durability);
				dat.addString(eTypeName, "name", pInv.equippedItems[eType].section);
			}
		}

		//save saved recipes
		for (auto const& rec : game.Crafter.savedRecipes)
		{
			listString.push_back(rec);
		}

		dat.sections["STATS"].lists.insert({ "savedRecipes", listString });
		dat.sections["STATS"].lists.insert({ "items", itemList });
		std::string savePath = "dat/saves/" + map.currentSaveName + "/save.eid";
		ItemReader::SaveDataToFile(savePath, dat, true);

		//save loaded chunks
		for (auto& chunk : game.mainMap.world.chunks) {
			chunk.second->SaveChunk(map.currentSaveName);
		}

		float endTime = glfwGetTime() - curTime;

		Console::Log("Save complete in " + std::to_string(endTime * 1000) + "ms.", text::green, __LINE__);
	}

	void SettingsScreen() {
		if (gameScreen.settingsOpen) {
			ImGui::Begin("Settings");

			if (ImGui::Button("Save Game")) {
				SaveCurrentGame();
			}
			if (ImGui::Button("Exit to Menu")) {
				ResetMenu();
			}

			ImGui::Text("\n--UI Settings--");
			ImGui::SliderInt("Y Separator Value", &ySeparator, 0, 20);
			ImGui::SliderInt("View Distance (Width)", &yViewDist, 1, 40);
			ImGui::SliderInt("View Distance (Height)", &xViewDist, 1, 40);


			ImGui::Text("\n--Sound Settings--");
			if (ImGui::SliderFloat("SFX Volume Level", &sfxvolume, 0.f, 1.f)) {
				Audio::SetVolume(sfxvolume);
				Audio::SetVolumeLoop(sfxvolume / 2, "rain");
			}
			if (ImGui::SliderFloat("Music Volume Level", &musicvolume, 0.f, 1.f)) {
				Audio::SetVolumeLoop(musicvolume, "ambient_day");
				Audio::SetVolumeLoop(musicvolume, "ambient_cave");
			}

			ImGui::Text("\n--Additional Settings--");
			if (!map.isUnderground) {
				if (ImGui::RadioButton("Free View Enabled (Experimental)", game.freeView)) {
					game.freeView = !game.freeView;
				}
			}

			//ImGui::ColorPicker3("Tab Color", &customTabColor.x);

			ImGui::End();
		}
	}

	void MenuScene() {
		ImGui::PushFont(Engine::Instance().getFont("ui"));
		//ImGui::ShowDemoWindow();
		ImGui::Begin("title");
		ImGui::SetFontSize(48.f);
		ImGui::Text("By The Fire");
		ImGui::SetFontSize(16.f);
		ImGui::End();

		

		if (newGameScreen) {
			ImGui::Begin("New Game Menu");
			ImGui::InputText("Save Name", &saveNameSlot[0], sizeof(char) * 128);
			if (ImGui::Button("Create New Save")) {
				Audio::Play(sfxs["crunchy_click"]);
				map.currentSaveName = std::string(saveNameSlot);
				if (CreateNewDirectory("dat/saves/" + map.currentSaveName)) {
					CreateNewDirectory("dat/saves/" + map.currentSaveName + "/map");
					Console::Log("New save created successfully!", SUCCESS, __LINE__);

					game.Setup(10, 10, 0.5f);
					gameScreen.createChar = true;
				}
				else {
					Console::Log("Failed to create save or save exists!", ERROR, __LINE__);
				}
			}
			ImGui::End();
		}

		if (savedGamesScreen) {
			ImGui::Begin("Load Game Menu");
			ImGui::InputText("Save Name", &saveNameSlot[0], sizeof(char) * 128);
			if (ImGui::Button("Load Save")) {
				Audio::Play(sfxs["crunchy_click"]);
				map.currentSaveName = std::string(saveNameSlot);

				if (DoesDirectoryExist(map.GetCurrentSavePath())) {

					OpenedData data;
					ItemReader::GetDataFromFile(map.GetCurrentSavePath() + "save.eid", "STATS", &data, false);
					OpenedData settings;
					ItemReader::GetDataFromFile(map.GetCurrentSavePath() + "save.eid", "SETTINGS", &settings, false);

					player.health = data.getFloat("health");
					player.thirst = data.getFloat("thirst");
					player.hunger = data.getFloat("hunger");
					map.c_glCoords = { data.getInt("global_x"), data.getInt("global_y") };

					pInv.clothes = { data.getFloat("color_r"), data.getFloat("color_g"), data.getFloat("color_b") };

					pInv.LoadItemsFromData(&data, map.GetCurrentSavePath() + "save.eid");

					for (int i = 0; i < data.getArray("savedRecipes").size(); i++)
					{
						game.Crafter.SaveRecipe(data.getArray("savedRecipes")[i]); //retrieve saved recipes
					}
					game.Setup(data.getInt("x_pos"), data.getInt("y_pos"), 0.5f, data.getInt("seed"), data.getInt("biomes"));
					currentState = playing;
					game.worldTime = data.getFloat("time");

					if (game.worldTime > 20.f || game.worldTime < 6.f) {
						Audio::PlayLoop("dat/sounds/music/night_zombos.wav", "night_music");
					}
					else {
						Audio::PlayLoop("dat/sounds/music/ambient12.wav", "ambient_day");
					}

					game.lerpingTo = urban;
					map.SetWeather((weather)data.getInt("weather"));


					ySeparator = settings.getInt("ySep");
					xViewDist = settings.getInt("xDist");
					yViewDist = settings.getInt("yDist");
					sfxvolume = settings.getFloat("sfxSound");
					Audio::SetVolume(sfxvolume);
					musicvolume = settings.getFloat("musicSound");
					Audio::StopLoop("menu");
					Audio::SetVolumeLoop(musicvolume, "ambient_day");

					for (auto const& eType : Items::EquipmentTypes)
					{
						std::string eTypeName = Cosmetic::EquipTypeName(eType);
						OpenedData data;
						if (ItemReader::GetDataFromFile("save.eid", eTypeName, &data)) {
							Item equippedItem = Items::GetItem(data.getString("name"));
							equippedItem.durability = data.getInt("durability");

							pInv.EquipItem(equippedItem);
						}
					}
				}
				else {
					gameScreen.CreatePopup("Error", "No save found.");
					saveNameSlot[0] = '\0';
				}
			}
			ImGui::End();
		}



		ImGui::Begin("Menu");
		if (gameScreen.createChar) { Create_Character(); }


		if (ImGui::Button("New Game"))
		{
			newGameScreen = true;
			Audio::Play(sfxs["crunchy_click"]);
		}

		if (ImGui::Button("Continue Game"))
		{
			savedGamesScreen = true;
			Audio::Play(sfxs["crunchy_click"]);
		}

		ImGui::End();

		ImGui::Begin("Debug");
		if (ImGui::Button("Open Custom Structure Editor")) {
			currentState = map_gen_test;
			map.CreateMap(map.landSeed, map.biomeSeed);
		}
		ImGui::End();

		ImGui::PopFont();
	}

	void MapTool() {

		ImGui::Begin("Custom Building Tool");

		ImGui::PushFont(Engine::Instance().getFont("main"));
		for (int i = 0; i < 30; i++) {
			for (int j = 0; j < 30; j++) {
				Vector2_I curPos = { i, j };
				Tile* curTile = customBuilding->GetTileAtCoords(curPos);

				if (cursorPos == curPos) {
					printIcon = 'X';
					iconColor = { 1,0,0,1 };
				}
				else {

					printIcon = game.GetTileChar(curTile);
					iconColor = game.GetTileColor(curTile, 0.f);
				}

				ImGui::TextColored(iconColor, printIcon.c_str());
				ImGui::SameLine();
			}
			ImGui::Text("");
		}
		ImGui::PopFont();

		ImGui::InputInt("Width", &customStructWidth);
		ImGui::InputInt("Height", &customStructHeight);
		if (ImGui::Button("Export Custom Structure")) {
			std::string mapExport = "";
			for (int i = 0; i < 30; i++) {
				for (int j = 0; j < 30; j++) {
					Vector2_I curPos = { i, j };
					switch (customBuilding->GetTileAtCoords(curPos)->id) {
					case ID_GRASS:
						mapExport += "?";
						break;
					case ID_SAND:
						break;
					case ID_STONE:
						mapExport += "w";
						break;
					case 13:
						mapExport += "f";
						break;
					case 14:
						mapExport += "+";
						break;
					case 16:
						mapExport += "-";
						break;
					case 20:
						mapExport += "c";
						break;
					}
				}
			}
			SaveData structure;
			structure.sections.insert({ "MONKEY_BAR", {} });
			structure.addString("MONKEY_BAR", "tiles", mapExport);
			structure.addInt("MONKEY_BAR", "width", customStructWidth);
			structure.addInt("MONKEY_BAR", "height", customStructHeight);

			ItemReader::SaveDataToFile("newStructure", structure, true);
		}
		ImGui::End();

		ImGui::Begin("Settings");
		ImGui::InputInt("Biome Seed", &map.biomeSeed);
		ImGui::InputInt("Land Seed", &map.landSeed);
		ImGui::InputFloat("Temperature Minimum", &map.tempMin, 0.1f);
		ImGui::InputFloat("Moisture Minimum", &map.moistureMin, 0.1f);
		if (ImGui::Button("Generate")) {
			map.CreateMap(map.landSeed, map.biomeSeed);
		}
		ImGui::End();

		ImGui::Begin("Map Generator");
		bool item = false;
		//ImGui::PushFont(Engine::Instance().getFont("main"));
		/*for (int i = -CHUNK_WIDTH; i < CHUNK_WIDTH * 2; i++) {
			for (int j = -CHUNK_HEIGHT; j < CHUNK_HEIGHT * 2; j++) {

				Tile* curTile = map.GetTileFromThisOrNeighbor({ i, j });
				Tile* underTile = map.GetTileFromThisOrNeighbor({ i + 1, j });

				if (curTile->hasItem)
				{
					item = true;
					itemPositions.push_back(ImGui::GetCursorPos());
					itemTiles.push_back(curTile);
					itemIcons.push_back(game.GetItemChar(curTile));
					ImGui::Text(" ");
					ImGui::SameLine();
					continue;
				}

				printIcon = game.GetTileChar(curTile);
				iconColor = game.GetTileColor(curTile, 0.f);

				if (underTile != nullptr) {
					if (underTile->id == 11) {
						//screen += "G";
						//colors.push_back(game.GetTileColor(underTile, intensity));
						printIcon = "G";
						iconColor = game.GetTileColor(underTile, 0.f);
					}
					else if (underTile->id == 12) {
						//screen += "J";
						//colors.push_back(game.GetTileColor(underTile, intensity));
						printIcon = "J";
						iconColor = game.GetTileColor(underTile, 0.f);
					}
					else if (underTile->id == 18) {
						//screen += "J";
						//colors.push_back(game.GetTileColor(underTile, intensity));
						printIcon = "Y";
						iconColor = game.GetTileColor(underTile, 0.f);
					}
				}

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
		*/
		//ImGui::PopFont();

		ImGui::End();
	}

	void Create_Character() {
		ImGui::Begin("Create your Character");

		//ImGui::ColorPicker3("Clothes Color", &clothes.x);
		//ImGui::RadioButton("")
		if (ImGui::TreeNode("-:Choose your Background:-"))
		{
			for (int n = 0; n < backgrounds.size(); n++)
			{
				if (ImGui::Selectable(&backgrounds[n].name[0], bgSelected == n)) {
					Audio::Play(sfxs["crunchy_click"]);
					bgSelected = n;
				}
			}
			ImGui::TreePop();

			if (bgSelected != -1) {
				ImGui::Text("~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~");
				switch (bgSelected) {
				case 0:
					ImGui::TextWrapped("You've always been one to get into tuffles, so when you went off on your own, you took what you needed.");
					ImGui::Text("--Starting Inventory--");
					ImGui::Text("1x Machete\n1x Leather Jacket\n10x Rock");
					break;
				case 1:
					ImGui::TextWrapped("You've always been prepared for everything, so when you went off on your own, you took what you knew would keep you alive.");
					ImGui::Text("--Starting Inventory--");
					ImGui::Text("5x Matches\n1x Raincoat\n2x Sterile Bandages");
					break;
				case 2:
					ImGui::TextWrapped("You were taught how to hunt growing up, so when you went off on your own, you took your hunting supplies to survive in the wild.");
					ImGui::Text("--Starting Inventory--");
					ImGui::Text("2x Bear Trap\n1x Leather Boots\n1x Campfire");
					break;
				case 3:
					ImGui::TextWrapped("You've always been curious about exploring every part of the world, so when you went off on your own, you took whatever could help you journey as far as possible.");
					ImGui::Text("--Starting Inventory--");
					ImGui::Text("5x Rations\n1x Leather Boots\n10x Scrap Bits (Money)");
					break;
				case 4:
					ImGui::TextWrapped("You've never really been one for staying around. You don't keep many things with you, and so you didn't bring much.");
					ImGui::Text("5x Rocks\n5x Sticks\n3x Ropes");
					break;
				case 5:
					ImGui::TextWrapped("You don't remember who you are after waking up in the middle of the forest. What is this stuff in your pockets?");
					ImGui::Text("--Starting Inventory--");
					ImGui::Text("???");
					break;
				}
				ImGui::Text("~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~");
			}
		}

		if (bgSelected != -1) {
			pInv.clothes = { 1.f, 1.f, 1.f };
			if (ImGui::Button("Start")) {
				for (size_t i = 0; i < backgrounds[bgSelected].items.size(); i++)
				{
					pInv.AddItemByID(backgrounds[bgSelected].items[i], backgrounds[bgSelected].itemCounts[i]);
				}
				Audio::StopLoop("menu");
				Audio::PlayLoop("dat/sounds/music/ambient12.wav", "ambient_day");
				currentState = playing;
			}
		}
		ImGui::End();
	}

	void GameScene() {

		//ImGui::PushStyleColor(ImGuiCol_TitleBg, { customTabColor.x - 0.4f,customTabColor.y - 0.4f,customTabColor.z - 0.4f,1 });

		ImGui::PushFont(Engine::Instance().getFont("ui"));
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);

		if (gameScreen.console_showing) {
			ImGui::Begin("Console");
			ImGui::InputTextWithHint("console", "enter commands", console_commands, IM_ARRAYSIZE(console_commands));
			ImGui::End();
		}
		//-------Map rendering-------

		if (game.worldTime >= 20.f || game.worldTime < 6.f) { ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.0,0.0,0.0,1 }); }
		else { ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ game.bgColor.x,game.bgColor.y,game.bgColor.z,1 }); }

		//map drawing
		{
			ImGui::Begin("Map");
			if (gameScreen.fancyGraphics) {
				SWAP_FONT("main");
			}

			bool item = false;
			ImVec2 playerPos;
			ImVec4 batchColor;
			std::string batchedString = "";
			char lastTile = ' ';

			for (int i = xMin; i < xMax; i++) {
				for (int j = yMin; j < yMax; j++) {
					vec2_i curCoords = { i, j };
					Tile* curTile = nullptr;
					Tile* underTile = nullptr;

					if (!game.freeView) {
						curTile = map.CurrentChunk()->GetTileAtCoords(curCoords);
						underTile = map.CurrentChunk()->GetTileAtCoords({ curCoords.x + 1, curCoords.y });
					}
					else {
						curTile = map.GetTileFromThisOrNeighbor(curCoords);
						underTile = map.GetTileFromThisOrNeighbor({ curCoords.x + 1, curCoords.y });
					}
					float intensity = 0.f;
					bool effectShowing = false;

					if (curTile == nullptr) {
						printIcon = "!";
						iconColor = { 1,0,0,1 };
						goto printing;
					}


					intensity = curTile->brightness;


					//Flashlight pyramid logic, idk weird math stuff. pack it away please
					{//===================================================================
						float newBrightness = 1.0f;
						if ((i <= player.coords.x && i >= player.coords.x - 10)
							&& (j >= player.coords.y - ((player.coords.x) - i)
								&& j <= player.coords.y + ((player.coords.x) - i))
							&& flashlightActive
							&& playerDir == direction::up) {
							newBrightness = 0.1f * (player.coords.x - i);
						}
						else if ((i >= player.coords.x && i <= player.coords.x + 10)
							&& (j >= player.coords.y - ((i - player.coords.x))
								&& j <= player.coords.y + (i - (player.coords.x)))
							&& flashlightActive
							&& playerDir == direction::down) {
							newBrightness = 0.1f * (i - player.coords.x);
						}
						else if ((j <= player.coords.y && j >= player.coords.y - 10)
							&& (i >= player.coords.x - ((player.coords.y) - j)
								&& i <= player.coords.x + ((player.coords.y) - j))
							&& flashlightActive
							&& playerDir == direction::left) {
							newBrightness = 0.1f * (player.coords.y - j);
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
						mobPositions.push_back(ImGui::GetCursorPos());
						mobIcons.push_back(ENT_PLAYER);
						mobColors.push_back(game.GetPlayerColor());
						//batchedString.append("#");
						//continue;
					}

					else if (map.GetEffectFromThisOrNeighbor(curCoords) == 1)
					{
						effectShowing = true;
						printIcon = "A";
						iconColor = Cosmetic::SmokeColor();
					}
					else if (map.GetEffectFromThisOrNeighbor(curCoords) == 2)
					{
						effectShowing = true;
						printIcon = "a";
						iconColor = Cosmetic::CoveredColor(water);
					}
					else if (map.GetEffectFromThisOrNeighbor(curCoords) == 15)
					{
						effectShowing = true;
						printIcon = "#";
						iconColor = { 1,0,0,1 };
					}
					else if (curTile->entity != nullptr) {
						mobPositions.push_back(ImGui::GetCursorPos());
						mobIcons.push_back(game.GetTileChar(curTile));
						mobColors.push_back(game.GetTileColor(curTile, 0.f));
						//batchedString.append(" ");
						ImGui::Text(" ");
						ImGui::SameLine();
						continue;

					}
					else {
						printIcon = game.GetTileChar(curTile);
						iconColor = game.GetTileColor(curTile, intensity);
					}

					if (i < CHUNK_HEIGHT - 1 && !effectShowing && underTile != nullptr) {
						Entity* curEnt = underTile->entity;

						if (curEnt != nullptr) {
							if (curEnt->targeting() && curEnt->health > 0) {
								//screen += "!";
								//colors.push_back(ImVec4{ 1,0,0,1 });
								printIcon = "!";
								iconColor = ImVec4{ 1,0,0,1 };
							}
						}
						if (underTile->id == 11) {
							//screen += "G";
							//colors.push_back(game.GetTileColor(underTile, intensity));
							if (printIcon != ENT_PLAYER) printIcon = "G";
							iconColor = game.GetTileColor(underTile, intensity);
						}
						else if (underTile->id == 12) {
							//screen += "J";
							//colors.push_back(game.GetTileColor(underTile, intensity));
							if (printIcon != ENT_PLAYER) printIcon = "J";
							iconColor = game.GetTileColor(underTile, intensity);
						}
						else if (underTile->id == 18) {
							//screen += "J";
							//colors.push_back(game.GetTileColor(underTile, intensity));
							if (printIcon != ENT_PLAYER) printIcon = "Y";
							iconColor = game.GetTileColor(underTile, intensity);
						}
					}

					if (curTile->hasItem)
					{
						item = true;
						itemPositions.push_back(ImGui::GetCursorPos());
						itemTiles.push_back(curTile);
						itemIcons.push_back(game.GetItemChar(curTile));
						ImGui::Text(" ");
						//batchedString.append(" ");
						ImGui::SameLine();
						continue;
					}
				printing:

					//screen += game.GetTileChar(curTile);
					//colors.push_back(game.GetTileColor(curTile, intensity));
					//if (lastTile != printIcon[0]) {
						//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 0.05);
						//ImGui::TextColored(batchColor, batchedString.c_str());
						//batchedString.clear();
					//}
					//if it IS the same as the last tile, group them
					//else {
						//batchedString.append(printIcon);
						//batchColor = iconColor;
					//}
					//lastTile = printIcon[0];
					ImGui::TextColored(iconColor, printIcon.c_str());
					ImGui::SameLine();
				}
				//ImGui::TextColored(batchColor, batchedString.c_str());
				//batchedString.clear();
				ImGui::Text("");
			}

			//print items with separate font
			if (item) {
				SWAP_FONT("items");
				for (size_t i = 0; i < itemIcons.size(); i++)
				{
					ImGui::SetCursorPos(itemPositions[i]);
					ImGui::TextColored(game.GetItemColor(itemTiles[i]), itemIcons[i].c_str());
				}
				itemTiles.clear();
				itemIcons.clear();
				itemPositions.clear();
			}

			//print mobs with separate font
			if (mobPositions.size() > 0) {
				SWAP_FONT("mobs");
				for (size_t i = 0; i < mobPositions.size(); i++)
				{
					ImGui::SetCursorPos(mobPositions[i]);

					ImGui::TextColored(mobColors[i], mobIcons[i].c_str());
				}
				mobIcons.clear();
				mobPositions.clear();
				mobColors.clear();
			}
			ImGui::End();
		}

		if (gameScreen.fancyGraphics) {
			SWAP_FONT("ui");
		}


		ImGui::PopStyleColor(1);

		//TechScreen();

		if (game.mainMap.isUnderground) {
			if (ImGui::Button("Leave Cave")) {
				Audio::StopLoop("ambient_cave");
				Audio::PlayLoop("dat/sounds/music/ambient12.wav", "ambient_day");
				Audio::SetVolumeLoop(musicvolume, "ambient_day");
				game.mainMap.isUnderground = !game.mainMap.isUnderground;
				game.freeView = true;
			}
		}

		//------Action Log----
		ImGui::Begin("Action Log");
		//ImGui::PushFont(Engine::Instance().getFont());
		for (int i = 0; i < game.actionLog.size(); i++)
		{
			ImGui::TextWrapped(game.actionLog[i].c_str());
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
				ImGui::ProgressBar((float)(player.liquidLast - player.ticksCovered) / player.liquidLast, ImVec2(0.0f, 0.0f));
			}

			ImGui::Text("Body Temperature :"); ImGui::SameLine();
			ImGui::Text(std::to_string(round(player.bodyTemp * 10.0f) / 10.0f).c_str());

			ImGui::End();

		}

		//------Equipment Screen------
		if (gameScreen.equipmentScreenOpen) {

			ImGui::Begin("Equipment");
			if (pInv.CurrentEquipExists(hat)) {
				ImGui::Text("Hat :"); ImGui::SameLine();
				ImGui::Text(pInv.equippedItems[hat].name.c_str());
				if (pInv.equippedItems[hat].maxDurability != -1)
					ImGui::ProgressBar(pInv.equippedItems[hat].durability / pInv.equippedItems[hat].maxDurability, ImVec2(0.0f, 0.0f));
				if (ImGui::Button("Unequip Hat")) {
					pInv.Unequip(hat);
				}
			}

			if (pInv.CurrentEquipExists(shirt)) {
				ImGui::Text("Shirt :"); ImGui::SameLine();
				ImGui::Text(pInv.equippedItems[shirt].name.c_str());
				if (pInv.equippedItems[shirt].maxDurability != -1)
					ImGui::ProgressBar(pInv.equippedItems[shirt].durability / pInv.equippedItems[shirt].maxDurability, ImVec2(0.0f, 0.0f));
				if (ImGui::Button("Unequip Shirt")) {
					pInv.Unequip(shirt);
				}
			}

			if (pInv.CurrentEquipExists(gloves)) {
				ImGui::Text("Gloves :"); ImGui::SameLine();
				ImGui::Text(pInv.equippedItems[gloves].name.c_str());
				if (pInv.equippedItems[gloves].maxDurability != -1)
					ImGui::ProgressBar(pInv.equippedItems[gloves].durability / pInv.equippedItems[gloves].maxDurability, ImVec2(0.0f, 0.0f));
				if (ImGui::Button("Unequip Gloves")) {
					pInv.Unequip(gloves);
				}
			}

			if (pInv.CurrentEquipExists(pants)) {
				ImGui::Text("Pants :"); ImGui::SameLine();
				ImGui::Text(pInv.equippedItems[pants].name.c_str());
				if (pInv.equippedItems[pants].maxDurability != -1)
					ImGui::ProgressBar(pInv.equippedItems[pants].durability / pInv.equippedItems[pants].maxDurability, ImVec2(0.0f, 0.0f));
				if (ImGui::Button("Unequip Pants")) {
					pInv.Unequip(pants);
				}
			}

			if (pInv.CurrentEquipExists(boots)) {
				ImGui::Text("Boots :"); ImGui::SameLine();
				ImGui::Text(pInv.equippedItems[boots].name.c_str());
				if (pInv.equippedItems[boots].maxDurability != -1)
					ImGui::ProgressBar(pInv.equippedItems[boots].durability / pInv.equippedItems[boots].maxDurability, ImVec2(0.0f, 0.0f));
				if (ImGui::Button("Unequip Boots")) {
					pInv.Unequip(boots);
				}
			}

			if (pInv.CurrentEquipExists(weapon)) {
				ImGui::Text("Weapon :"); ImGui::SameLine();
				ImGui::Text(pInv.equippedItems[weapon].name.c_str());
				ImGui::Text("Damage :"); ImGui::SameLine();
				ImGui::Text(std::to_string(pInv.equippedItems[weapon].mod).c_str());
				if (pInv.equippedItems[weapon].maxDurability != -1)
					ImGui::ProgressBar(pInv.equippedItems[weapon].durability / pInv.equippedItems[weapon].maxDurability, ImVec2(0.0f, 0.0f));
				if (ImGui::Button("Unequip Weapon")) {
					pInv.Unequip(weapon);
				}
			}
			ImGui::End();
		}

		//------Inventory------
		ImGui::Begin("Inventory");
		ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.15, 0.15, 0.15, 1 });
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
			Item& curItem = pInv.items[currentItemIndex];

			ImGui::Begin("Current Item");
			ImGui::Text((curItem.name + "\n\n").c_str());

			SWAP_FONT("items");
			ImGui::SetFontSize(32.f);
			ImGui::Text(game.item_icons[curItem.section].c_str());
			ImGui::SetFontSize(16.f);
			SWAP_FONT("ui");

			if (curItem.maxDurability != -1.f) {
				ImGui::Text("-- Durability --");
				ImGui::ProgressBar(curItem.durability / curItem.maxDurability, ImVec2(0.0f, 0.0f));
			}

			ImGui::TextWrapped(curItem.description.c_str());

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


			if (!curItem.stackable) { ImGui::TextColored(ImVec4{ 1,0,0,1 }, "[DOES NOT STACK]"); }

			if (curItem.holdsLiquid) {
				ImGui::Text("Liquid:");
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, colorBG);

				ImGui::ProgressBar(curItem.liquidAmount / 100, ImVec2(0.0f, 0.0f));
				ImGui::PopStyleColor(2);
				ImGui::Text("Liquid Type:"); ImGui::SameLine();
				ImGui::Text(pInv.GetLiquidName(currentItemIndex).c_str());
			}

			if (curItem.ticksUntilDry > 0) {
				ImGui::Text("Time until dry:");
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, colorBG);

				ImGui::ProgressBar((float)curItem.ticksUntilDry / (float)curItem.initialTickTime, ImVec2(0.0f, 0.0f));
				ImGui::PopStyleColor(2);

			}

			if (curItem.stackable) {
				ImGui::Text("[ Holding x");
				ImGui::SameLine();
				ImGui::Text(std::to_string(curItem.count).c_str());
				ImGui::SameLine();
				ImGui::Text("]");
			}

			ImGui::Text("Crafting ID ::"); ImGui::SameLine();
			ImGui::Text(curItem.section.c_str());

			if (ImGui::Button("Use"))
			{
				gameScreen.useBool = !gameScreen.useBool;
			}
			if (curItem.eType != notEquip) {
				if (ImGui::Button("Equip"))
				{
					pInv.EquipItem(currentItemIndex);
					currentItemIndex = 0;
				}
			}

			if (gameScreen.useBool)
			{
				ImGui::Text("On what?");
				if (ImGui::Button("Consume"))
				{
					if (pInv.AttemptAction(consume, &pInv.items[currentItemIndex], &player))
					{
						if (pInv.items[currentItemIndex].use.onConsume.effect == saturate) Audio::Play(sfxs["crunch"]);
						else if (pInv.items[currentItemIndex].use.onConsume.effect == quench) Audio::Play(sfxs["drink"]);

						//if its consumable, remove one. 
						if (curItem.consumable) {
							//If its the last one, move the cursor so we dont get out of vector range
							if (pInv.RemoveItem(curItem.section)) {
								currentItemIndex--;
							}

						}
						Math::PushBackLog(&game.actionLog, curItem.consumeTxt);
					}
					else
					{
						Math::PushBackLog(&game.actionLog, "You can't consume " + curItem.name + ".");
					}
					gameScreen.useBool = !gameScreen.useBool;
				}
				if (ImGui::Button("Body (Self)"))
				{
					if (pInv.AttemptAction(use, &curItem, &player))
					{
						if (curItem.consumable) { curItem.count--; }
						Math::PushBackLog(&game.actionLog, curItem.useTxt);
					}
					else
					{
						Math::PushBackLog(&game.actionLog, "You can't use " + curItem.name + ".");
					}
					gameScreen.useBool = !gameScreen.useBool;
				}
			}

			ImGui::End();
		}

		//------Crafting-------
		if (gameScreen.craftingMenu) {
			ImGui::Begin("Crafting");

			//Saved Recipe section
			if (ImGui::CollapsingHeader("Saved Recipes")) {

				if (ImGui::BeginListBox("Saved")) {
					int n = 0;
					for (auto const& rec : game.Crafter.savedRecipes)
					{
						const bool is_selected = (recipeSelected == n);
						if (ImGui::Selectable(rec.c_str(), is_selected)) {
							recipeSelectedName = rec;
							recipeSelected = n;
						}
						n++;
					}
					ImGui::EndListBox();
				}
			}
			if (ImGui::CollapsingHeader("Search for Recipes")) {
				//search section
				if (ImGui::BeginListBox("Recipes"))
				{
					ImGui::InputText("Material", recipe_search, IM_ARRAYSIZE(recipe_search));
					std::vector<std::string> recipesList = game.Crafter.getRecipesByItem(recipe_search);
					for (int n = 0; n < recipesList.size(); n++)
					{
						const bool is_selected = (recipeSelected == n);
						if (ImGui::Selectable(recipesList[n].c_str(), is_selected)) {
							recipeSelectedName = recipesList[n];
							recipeSelected = n;
						}
					}
					ImGui::EndListBox();
				}
			}

			//Selected recipe section
			if (recipeSelectedName != "") {
				std::vector<std::string> components = game.Crafter.getRecipeComponents(recipeSelectedName);
				SWAP_FONT("items");
				ImGui::SetFontSize(32.f);
				ImGui::Text(("\n" + game.item_icons[recipeSelectedName]).c_str());
				ImGui::SetFontSize(16.f);
				SWAP_FONT("ui");
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

				std::string saveButton = "Save ";
				if (game.Crafter.savedRecipes.count(recipeSelectedName) != 0) {
					saveButton = "Unsave ";
				}
				saveButton += recipeSelectedName;
				saveButton += " Recipe";
				if (ImGui::Button(saveButton.c_str())) {
					Audio::Play(sfxs["ui_select"]);
					if (game.Crafter.savedRecipes.count(recipeSelectedName) != 0) {
						game.Crafter.UnsaveRecipe(recipeSelectedName);
					}
					else {
						game.Crafter.SaveRecipe(recipeSelectedName);
					}
				}
			}


			ImGui::PushStyleColor(ImGuiCol_Button, { 0.5,0,0,1 });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.8,0,0,1 });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 1,0.7,0.7,1 });
			if (ImGui::Button("Close Crafting Window")) {
				Audio::Play(sfxs["fail"]);
				gameScreen.craftingMenu = false;
			}
			ImGui::PopStyleColor(3);
			ImGui::End();
		}


		// Help Menu

		if (gameScreen.helpMenu) {
			ImGui::Begin("Help Menu");
			ImGui::Text("WASD or Arrow keys to move");
			ImGui::Text("F to toggle Flashlight on and off");
			ImGui::Text("C to open the Crafting menu");
			ImGui::Text("I to open/close the Equipment menu");
			ImGui::Text("E to begin selecting a block then any of the directional keys to select a block");
			ImGui::Text("P to open/close the Debug menu");
			ImGui::Text("H to open/close the Help menu");
			ImGui::Text("ESC to open/close the Settings menu");
			ImGui::Text("` to open/close the console");
			ImGui::Text("\nTo attack an entity, walk into it with something equipped.");
			ImGui::End();
		}

		//------Interaction------
		if (selectedTile != nullptr)
		{
			ImGui::Begin("Selected Block");

			Container* curCont = selectedTile->tileContainer;

			//Both Lists for the entity and for the chest
			if (selectedTile->entity != nullptr) {
				if (selectedTile->entity->health <= 0) {
					if (ImGui::CollapsingHeader("Corpse")) {
						ImGui::BeginListBox("Items");
						std::vector<std::string> containerList = selectedTile->entity->getItemNames();
						for (int n = 0; n < containerList.size(); n++)
						{
							const bool is_selected = (itemSelected == n);
							if (ImGui::Selectable(containerList[n].c_str(), is_selected)) {
								itemSelectedName = containerList[n];
								itemSelected = n;
							}
						}
						ImGui::EndListBox();

						if (containerList.size() > 0) {
							if (ImGui::Button("Take Item")) {
								pInv.AddItem(selectedTile->entity->inv[itemSelected]);
								selectedTile->entity->inv.erase(selectedTile->entity->inv.begin() + itemSelected);
							}
						}
					}
				}
				else {
				//--------TRADING---------
				//If they have something for trade
					if (selectedTile->entity->itemWant != "nothing") {
						//generate message
						std::string tradeMessage = "Hey, if you have a ";
						tradeMessage += Items::GetItem_NoCopy(selectedTile->entity->itemWant)->name;
						tradeMessage += " then I'll trade you a ";
						tradeMessage += Items::GetItem_NoCopy(selectedTile->entity->itemGive)->name;
						tradeMessage += ". Deal?";

						//if player has item, then trade them
						ImGui::TextWrapped(tradeMessage.c_str());
						if (ImGui::Button("Trade?")) {
							int _ = -1;
							if (pInv.TryGetItem(selectedTile->entity->itemWant, false, &_)) {
								//success
								selectedTile->entity->ChangePlayerStatus(0.5f);
								pInv.RemoveItem(selectedTile->entity->itemWant);
								pInv.AddItemByID(selectedTile->entity->itemGive);
								selectedTile->entity->GenerateTrades();
							}
						}
					}
					//------------------------
					//ImGui::Text(std::to_string(selectedTile->entity->feelingTowardsPlayer).c_str());

					//Ask the human if they want to follow or unfollow, if they're favor is high enough
					if (selectedTile->entity->feelingTowardsPlayer >= 1.f) {
						if (selectedTile->entity->b != Follow) {
							if (ImGui::Button("Follow me.")) {
								gameScreen.CreatePopup("Response", "Alright, I'll follow you.");
								selectedTile->entity->b = Follow;
							}
						}
						else {
							if (ImGui::Button("Stop following me.")) {
								gameScreen.CreatePopup("Response", "Alright, we'll split up.");
								selectedTile->entity->b = Protective;
							}
						}
					}
					if (selectedTile->entity->canTalk) {
						ImGui::TextWrapped(("\"" + selectedTile->entity->message + "\"").c_str());
						//ImGui::TextColored({ 1,0,0,1 }, "RAAAHHH!!");
					}
				}
			}

			else if (curCont != nullptr) {
				if (ImGui::CollapsingHeader("Container")) {
					ImGui::BeginListBox("Items");
					std::vector<std::string> containerList = curCont->getItemNames();
					for (int n = 0; n < containerList.size(); n++)
					{
						const bool is_selected = (itemSelected == n);
						if (ImGui::Selectable(containerList[n].c_str(), is_selected)) {
							itemSelectedName = containerList[n];
							itemSelected = n;
						}
					}
					ImGui::EndListBox();

					if (ImGui::Button("Put selected item in Container")) {
						curCont->AddItem(pInv.items[currentItemIndex]);
						if (pInv.RemoveItem(pInv.items[currentItemIndex].section)) { currentItemIndex = 0; }
					}
					if (containerList.size() > 0) {
						if (ImGui::Button("Take Item")) {
							pInv.AddItem(curCont->items[itemSelected]);
							curCont->items.erase(curCont->items.begin() + itemSelected);
						}
					}
				}
			}


			SWAP_FONT("main");
			/*if (curCont != nullptr) {
				ImGui::TextColored(ImVec4{ 0.5, 0.34, 0, 1 }, "U");
			}
			else {*/
			ImGui::TextColored(game.GetTileColor(selectedTile, 1.f), game.GetTileChar(selectedTile).c_str());
			//}
			SWAP_FONT("ui");

			if (selectedTile->hasItem) { ImGui::Text(("Item on tile: " + selectedTile->itemName).c_str()); }

			if (ImGui::Button("Drop Selected Item")) {
				if (invSelectedName != "" && !selectedTile->hasItem) {
					selectedTile->hasItem = true;
					std::string upperName = pInv.items[currentItemIndex].section;

					selectedTile->itemName = upperName;
					selectedTile->collectible = true;
					if (selectedTile->itemName == "CAMPFIRE" || selectedTile->itemName == "CHEST") {
						//add a container to that tile
						selectedTile->tileContainer = new Container;
					}
					if (pInv.RemoveItem(pInv.items[currentItemIndex].section)) { currentItemIndex = 0; }
				}

				else {
					Math::PushBackLog(&game.actionLog, "There is already an item on that space.");
				}
			}

			//grody ahh pyramid
			bool canCollect = true;
			if (selectedTile->collectible || selectedTile->liquid != nothing || selectedTile->hasItem) {
				if (ImGui::Button("Collect")) {
					if (selectedTile->tileContainer != nullptr && selectedTile->tileContainer->items.size() != 0)
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
							if (selectedTile->tileContainer != nullptr) {
								//Phew!!
								delete selectedTile->tileContainer;
								selectedTile->tileContainer = nullptr;
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

			//only light fires if we are holding a lighter or match
			if (pInv.CurrentEquipMatches(weapon, "LIGHTER") || pInv.CurrentEquipMatches(weapon, "MATCH")) {
				if (ImGui::Button("Burn")) {
					selectedTile->liquid = fire;
					map.floodFill(selectedTile->coords, 5, false);

					if (pInv.CurrentEquipMatches(weapon, "MATCH")) {
						pInv.Unequip(weapon);
						pInv.RemoveItem("MATCH");
					}
				}
			}

			int _ = 0;
			if (pInv.TryGetItem("ROCK", false, &_)) {
				if (ImGui::CollapsingHeader("Building Options")) {
					if (ImGui::Button("Place Wall ( 1 x Rock )")) {
						*selectedTile = Tiles::GetTile("TILE_STONE");
						pInv.RemoveItem("ROCK");
					}
					if (ImGui::Button("Place Floor ( 1 x Rock )")) {
						*selectedTile = Tiles::GetTile("TILE_STONE_FLOOR");
						pInv.RemoveItem("ROCK");
					}
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

			ImGui::Text(("Tested Function Runtime: " + std::to_string(game.testTime)).c_str());
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

			ImGui::Text("Weather : "); ImGui::SameLine();
			ImGui::Text(Cosmetic::WeatherName(game.mainMap.currentWeather));

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

		ImGui::PopFont();
		//ImGui::PopStyleColor(3);
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
						pInv.AddItemByID("BAD_PISTOL");
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

	//---------------------------------------------BASE FUNCTIONS---------------------------------------------

	void Init() override {

		test_chunk = std::make_shared<Chunk>();
		customBuilding = std::make_shared<Chunk>();
		Items::LoadItems(&game.item_icons);
		Tiles::LoadTiles(&game.tile_colors);

		map.EmptyChunk(customBuilding);
		//std::thread itemLoading = Items::LoadItemsFromFiles(&game.item_icons);
		//std::thread tileLoading = Tiles::LoadTilesFromFiles(&game.tile_colors);

		Engine::Instance().AddScene(&main);
		main.CreateObject("BG", { 0,0 }, { 3,3 }, "res/bg.png");
		main.CreateObject("Box", { 0,0 }, { 1,1 }, "res/plank.png");
		p = main.FindObject("Box");

		gameScreen.fancyGraphics = true;
		frame.frames.length = 360;
		currentState = menu;
		health = 100.0f;
		openClose = "Close Stats";
		gameScreen.statsOpen = true;
		gameScreen.showDialogue = true;
		gameScreen.helpMenu = true;
		game.freeView = true;
		game.LoadData();
		xViewDist = 15;
		yViewDist = 15;

		sfxvolume = Audio::GetVolume();


		if (!DoesDirectoryExist("dat/saves")) {
			Console::Log("Save folder does not exist. Creating new...", text::white, __LINE__);
			CreateNewDirectory("dat/saves/");
		}

		//itemLoading.join();
		//tileLoading.join();
		Console::Log("Done!", text::green, __LINE__);
		Audio::PlayLoop("dat/sounds/music/night_zombos.wav", "menu");
	}

	void Update() override
	{
		bool moved = false;
		colChangeTime += Utilities::deltaTime() / 1000;
		if (colChangeTime >= 1.f) {
			colChangeTime = 0;
		}

		//the rest of the update is game logic so we stop here in the menu
		if (currentState == map_gen_test) {
			if (Input::KeyDown(KEY_UP)) {
				cursorPos.x -= 1;
			}
			if (Input::KeyDown(KEY_DOWN)) {
				cursorPos.x += 1;
			}
			if (Input::KeyDown(KEY_LEFT)) {
				cursorPos.y -= 1;
			}
			if (Input::KeyDown(KEY_RIGHT)) {
				cursorPos.y += 1;
			}
			if (Input::KeyHeldDown(KEY_SPACE)) {
				if (Input::KeyHeldDown(KEY_LEFT_SHIFT)) {
					*customBuilding->GetTileAtCoords(cursorPos) = Tiles::GetTile("TILE_SAND");
				}
				else if (Input::KeyHeldDown(KEY_F)) {
					*customBuilding->GetTileAtCoords(cursorPos) = Tiles::GetTile("TILE_STONE_FLOOR");
				}
				else if (Input::KeyHeldDown(KEY_A)) {
					*customBuilding->GetTileAtCoords(cursorPos) = Tiles::GetTile("TILE_GRASS");
				}
				else if (Input::KeyHeldDown(KEY_S)) {
					*customBuilding->GetTileAtCoords(cursorPos) = Tiles::GetTile("TILE_CRYSTAL");
				}
				else if (Input::KeyHeldDown(KEY_C)) {
					*customBuilding->GetTileAtCoords(cursorPos) = Tiles::GetTile("TILE_CHAIN_FENCE");
				}
				else if (Input::KeyHeldDown(KEY_D)) {
					*customBuilding->GetTileAtCoords(cursorPos) = Tiles::GetTile("TILE_MUD");
				}
				else {
					*customBuilding->GetTileAtCoords(cursorPos) = Tiles::GetTile(customTileSelect);
				}
			}
			return;
		}
		else if (currentState != playing) { return; }

		//-=================================================================

		if (player.health <= 0) { currentState = menu; player.health = 100; }

		if (gameScreen.console_showing) {
			if (Input::KeyDown(KEY_ENTER)) {
				Console::Log(console_commands, text::green, __LINE__);
				cmd.RunCommand(console_commands, &game);
				console_commands[0] = '\0';
				prevCommandIndex = -1;
			}
			if (Input::KeyDown(KEY_UP)) {
				if (cmd.prevCommandSize() > 0) {
					std::string prevCom = cmd.GetOldCommand(&prevCommandIndex);
					//LAZY_LOG(prevCommandIndex);
					memset(console_commands, '\0', sizeof(console_commands));

					for (int i = 0; i < prevCom.size(); i++) {
						console_commands[i] = prevCom[i];
					}
				}
			}
		}

		gameScreen.FlipScreens();

		if (gameScreen.console_showing || gameScreen.craftingMenu) { return; }

		game.UpdateTick();

		if (Input::KeyDown(KEY_UP) || Input::KeyDown(KEY_W)) {
			if (interacting)
			{
				selectedTile = game.SelectTile({ player.coords.x - 1, player.coords.y });
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
				selectedTile = game.SelectTile({ player.coords.x + 1, player.coords.y });
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
				selectedTile = { game.SelectTile({ player.coords.x, player.coords.y - 1}) };
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
				selectedTile = { game.SelectTile({ player.coords.x, player.coords.y + 1}) };
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

		if (game.freeView) {
			xMin = player.coords.x - xViewDist;
			xMax = player.coords.x + xViewDist;
			yMin = player.coords.y - yViewDist;
			yMax = player.coords.y + yViewDist;
		}
		else {
			xMin = 0;
			xMax = CHUNK_HEIGHT;
			yMin = 0;
			yMax = CHUNK_WIDTH;
		}
	}

	void ImguiRender() override
	{
		//ImGui::ShowDemoWindow();
		//GameScene();

		gameScreen.ShowPopups();

		SettingsScreen();

		switch (currentState) {
		case playing:
			GameScene();
			break;
		case menu:
			MenuScene();
			break;
		case map_gen_test:
			MapTool();
			break;
		default:
			break;
		}


	}

	void Shutdown() override {
		if (currentState == playing) {
			SaveCurrentGame();
		}
	}
};

std::vector<App*> CreateGame() {
	return { new Caves };
}