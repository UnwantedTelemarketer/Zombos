#include "antibox/core/antibox.h"
#include "json/json.h"
#include <fstream>
#include <antibox/objects/tokenizer.h>

using json = nlohmann::json;

using namespace antibox;

struct die {
	int sides = 6;
	bool rolling = false;
	float rolled_time = 0;
	int modifier = 0;
	int result = 1;
	float shown_time = 0;
};

#define VGAFONT  "dat/fonts/VGA437.ttf"

class DND : public App {

	WindowProperties GetWindowProperties() {
		WindowProperties props;
		props.imguiProps = { true, true, false, {VGAFONT}, {"ui"}, 24.f };
		props.vsync = 1;
		props.w = 800;
		props.h = 600;
		props.title = "TTRPG Stat Tracker";
		props.cc = { 0.2f, 0.2f, 0.2f, 1.f };
		return props;
	}

	std::vector<die> rolling_dice = {};
	char noteBox[256];
	char saveFileName[64];
	float display_time = 3;
	float minRollTime = 1;
	float maxRollTime = 2;
	int rollModifier = 0;
	bool rollWithMod = false;
	int healthMax = 1;
	int healthCurrent = 1;
	int tempHP = 1;
	int healthModification = 0;
	int notepadWidth = 200;
	int notepadHeight = 200;
	std::string load_message = "";

	//Stat Variables
	std::map<std::string, int> statistics;

	float nat20Color[3] = { 0,1,0 };
	float nat1Color[3] = { 1,0,0 };
	float rainbowHue = 0.f;
	bool rainbow20 = false;
	bool rainbow1 = false;

	Vector3 currentBGColor = { 0.2,0.2,0.2 };
	float arrBGcolor[3] = { 0.2, 0.2, 0.2 };
	Scene main = { "Sprites" };
	std::shared_ptr<GameObject> logo;
	bool loaded = false;
	bool menuOpen = false;
	bool settingsOpen = false;
	bool makeNewMessage = false;
	float autosaveTime = 3600;
	int autosaveMinute = 5;


	void ResetStats() {
		menuOpen = false;

		currentBGColor = { 0.2,0.2,0.2 };
		arrBGcolor[0] = 0.2;
		arrBGcolor[1] = 0.2;
		arrBGcolor[2] = 0.2;
		nat20Color[0] = 0;
		nat20Color[1] = 1;
		nat20Color[2] = 0;
		nat1Color[0] = 1;
		nat1Color[1] = 0;
		nat1Color[2] = 0;

		noteBox[0] = '\0';
		display_time = 3;
		minRollTime = 1;
		maxRollTime = 2;
		rollModifier = 0;
		rollWithMod = false;
		healthMax = 1;
		healthCurrent = 1;
		tempHP = 1;
		healthModification = 0;
		notepadWidth = 200;
		notepadHeight = 200;
		DNDTemplate();
	}

	void DNDTemplate() {
		statistics.insert({ "strength", 1 });
		statistics.insert({ "dexterity", 1 });
		statistics.insert({ "constitution", 1 });
		statistics.insert({ "intelligence", 1 });
		statistics.insert({ "wisdom", 1 });
		statistics.insert({ "charisma", 1 });
	}

	void Init() override {
		Engine::Instance().AddScene(&main);
		main.CreateObject("logo", { -0.25,0.25 }, { 0.5,0.5 }, "res/TTRPG.png");
		logo = main.FindObject("logo");
		/*
		std::ifstream f("res/statsheet.json");
		//json data = json::parse(f);
		//std::string name = data["data"]["name"].template get<std::string>();

		//Console::Log(data["message"].template get<std::string>(), ERROR, __LINE__);
		//Console::Log(name, ERROR, __LINE__);*/
		ResetStats();
	}

	void Update() override {
		if (loaded) {
			autosaveTime -= 1;
			if (autosaveTime <= 0) {
				SaveUserData(saveFileName);
				Console::Log("Autosaved.", text::white, -1);
				//			   frms  secs		minutes
				autosaveTime = ((60 * 60) * autosaveMinute);
			}
		}

		rainbowHue += 0.05f;
		if (rainbowHue > 1.f) {
			rainbowHue = 0.f;
		}
		if (Input::KeyDown(KEY_ESCAPE)) {
			menuOpen = !menuOpen;
		}
	}

	void ImguiRender() override {
		if (!loaded) {
			//if they type a name that doesnt exist, ask for new file creation
			if (makeNewMessage) {
				ImGui::Begin("New File");
				ImGui::TextWrapped("Looks like there's no file with that name. Would you like to make a new one?");
				if (ImGui::Button("Yes")) {
					SaveData dat;
					ItemReader::SaveDataToFile(saveFileName, dat, true);
					loaded = true;
					ResetStats();
				}
				if (ImGui::Button("No")) {
					makeNewMessage = false;
				}
				ImGui::End();
			}


			ImGui::InputTextWithHint("##filename", "filename.save", &saveFileName[0], sizeof(char) * 64);
			if (ImGui::Button("Load")) {
				OpenedData dat;
				if (ItemReader::GetDataFromFile(saveFileName, "SETTINGS", &dat, false)) {
					std::string noteSaved = dat.getString("notepad");
					for (size_t i = 0; i < noteSaved.size(); i++)
					{
						noteBox[i] = noteSaved[i];
					}
					notepadWidth = dat.getInt("notepadWidth");
					notepadHeight = dat.getInt("notepadHeight");
					healthMax = dat.getInt("maxHealth");
					healthCurrent = dat.getInt("currentHealth");
					tempHP = dat.getInt("tempHealth");
					arrBGcolor[0] = dat.getFloat("bgColorR");
					arrBGcolor[1] = dat.getFloat("bgColorG");
					arrBGcolor[2] = dat.getFloat("bgColorB");
					currentBGColor = { arrBGcolor[0], arrBGcolor[1], arrBGcolor[2] };
					std::vector<std::string> statString = dat.getArray("statistics");
					for (size_t i = 0; i < statString.size(); i++)
					{
						statistics[statString[i]] = stoi(statString[i + 1]);
						i++;
					}
					loaded = true;
				}
				else {
					makeNewMessage = true;
					load_message = "Could not find file named '";
					load_message += saveFileName;
					load_message += "'.";
				}
			}
			ImGui::TextWrapped(load_message.c_str());
			return;
		}
		//bgColor = { arrBGcolor[0], arrBGcolor[1], arrBGcolor[2] };

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ currentBGColor.x, currentBGColor.y, currentBGColor.z, 1 });

		ImGui::PushFont(Engine::Instance().getFont("ui"));




		ImGui::Begin("Stats");
		ImGui::Text("-=:~ Max Health ~:=-");
		ImGui::DragInt("##mHealth", &healthMax);
		ImGui::Text("-=:~ Temp Health ~:=-");
		ImGui::DragInt("##tHP", &tempHP);
		ImGui::Text("-=:~ Current Health ~:=-");
		float healthFloat = (float)healthCurrent / (float)healthMax;

		if (healthFloat >= 0.5f) {
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4{ 0,0.85,0,1 });
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0,0.5,0,1 });
		}
		else if (healthFloat >= 0.25f) {
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4{ 0.85,0.85,0,1 });
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0.5,0.5,0,1 });
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4{ 0.85,0,0,1 });
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0.5,0,0,1 });
		}

		ImGui::ProgressBar(healthFloat, { 200,30 }, std::to_string(healthCurrent).c_str());
		ImGui::PopStyleColor(2);

		if (ImGui::CollapsingHeader("Modify")) {
			ImGui::InputInt("##hMod", &healthModification);
			if (ImGui::Button("Damage")) {
				tempHP -= healthModification;
				//Set BG to red and lerp it back
				currentBGColor = { 0.75f,0,0 };
				Utilities::Lerp("bgColor", &currentBGColor, { arrBGcolor[0], arrBGcolor[1], arrBGcolor[2] }, 0.5f);

				if (tempHP < 0) {
					healthCurrent += tempHP;
				}

				if (tempHP < 0) {
					tempHP = 0;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Heal")) {
				//Set BG to green and lerp it back
				currentBGColor = { 0,0.75f,0 };
				Utilities::Lerp("bgColor", &currentBGColor, { arrBGcolor[0], arrBGcolor[1], arrBGcolor[2] }, 0.5f);

				healthCurrent += healthModification;
			}

			if (ImGui::Button("Reset to Max")) {
				healthCurrent = healthMax;
			}

		}
		ImGui::End();

		ImGui::Begin("Stat Modifiers");
		for (auto& stat : statistics)
		{
			ImGui::InputInt(stat.first.c_str(), &stat.second);
		}
		ImGui::End();

		ImGui::Begin("Notes");
		ImGui::Text("Notepad");
		ImGui::InputTextMultiline("##notes", &noteBox[0], sizeof(char) * 256, { (float)notepadWidth, (float)notepadHeight });
		ImGui::End();

		ImGui::Begin("Dice");

		for (int i = 0; i < rolling_dice.size(); i++)
		{
			if (rolling_dice[i].rolling) {
				rolling_dice[i].rolled_time -= Utilities::deltaTime() / 1000;
				rolling_dice[i].result = Math::RandInt(1, rolling_dice[i].sides);
				if (rolling_dice[i].rolled_time <= 0) {
					rolling_dice[i].rolling = false;
				}
			}
			else {
				rolling_dice[i].shown_time += Utilities::deltaTime() / 1000;
				if (rolling_dice[i].shown_time >= display_time) {
					rolling_dice.erase(rolling_dice.begin() + i);
					i--;
				}
			}
		}

		ImGui::InputInt("Modifier", &rollModifier);
		ImGui::Checkbox("Roll with Modifier?", &rollWithMod);


		//-------Dice Buttons--------
		bool clickedDie = false;
		int sides = 0;

		if (ImGui::Button("D20")) {
			clickedDie = true;
			sides = 20;
		}
		ImGui::SameLine();
		if (ImGui::Button("D12")) {
			clickedDie = true;
			sides = 12;
		}
		ImGui::SameLine();
		if (ImGui::Button("D10")) {
			clickedDie = true;
			sides = 10;
		}
		if (ImGui::Button("D8")) {
			clickedDie = true;
			sides = 8;
		}
		ImGui::SameLine();
		if (ImGui::Button("D6")) {
			clickedDie = true;
			sides = 6;
		}
		ImGui::SameLine();
		if (ImGui::Button("D4")) {
			clickedDie = true;
			sides = 4;
		}
		if (clickedDie) {
			die new_die = { sides, true, Math::RandInt(minRollTime, maxRollTime) };

			//if they are rolling with mod, add mod, otherwise 0 is mod
			if (rollWithMod) {
				new_die.modifier = rollModifier;
			}
			rolling_dice.push_back(new_die);
		}

		//-------Dice Rolled Section--------
		ImGui::Text("---Dice Rolled---");

		for (int i = 0; i < rolling_dice.size(); i++)
		{
			ImGui::Text(("D" + std::to_string(rolling_dice[i].sides) + " --").c_str());
			ImGui::SameLine();

			//Die Color
			ImVec4 dieColor = { 1,1,1,1 };

			//Critical Colors
			if (rolling_dice[i].result == rolling_dice[i].sides && !rolling_dice[i].rolling) {
				if (!rainbow20) {
					dieColor = { nat20Color[0],nat20Color[1] ,nat20Color[2], 1 };
				}
				else {
					dieColor = { rainbowHue, 1, 0.5, 1 };
				}
			}
			if (rolling_dice[i].result == 1 && !rolling_dice[i].rolling) {
				if (!rainbow1) {
					dieColor = { nat1Color[0],nat1Color[1] ,nat1Color[2], 1 };
				}
				else {
					dieColor = { rainbowHue, 1, 0.5, 1 };
				}
			}

			//Print Result in Color
			ImGui::TextColored(dieColor, std::to_string(rolling_dice[i].result).c_str());

			//roll with mod shows added total
			if (rolling_dice[i].modifier != 0) {
				ImGui::SameLine();
				ImGui::Text(("+ " + std::to_string(rolling_dice[i].modifier)).c_str());
				ImGui::SameLine();
				ImGui::Text(("(" + std::to_string(rolling_dice[i].result + rolling_dice[i].modifier) + ")").c_str());
			}

		}

		if (menuOpen) {
			if (settingsOpen) {
				ImGui::Begin("Settings");
				if (ImGui::Button("Close Settings")) {
					settingsOpen = !settingsOpen;
				}
				ImGui::Text("--Disappear Time--");
				ImGui::SliderFloat("##1", &display_time, 0.5, 5);
				ImGui::Text("--Minimum Roll Time--");
				ImGui::SliderFloat("##2", &minRollTime, 0.5, 5);
				ImGui::Text("--Maximum Roll Time--");
				ImGui::SliderFloat("##3", &maxRollTime, 0.5, 5);


				ImGui::Text("--Notepad Width--");
				ImGui::DragInt("##nWidth", &notepadWidth);
				ImGui::Text("--Notepad Height--");
				ImGui::DragInt("##nHeight", &notepadHeight);

				ImGui::Text("--Autosave Time (Minutes)--");
				ImGui::DragInt("##aMin", &autosaveMinute);

				if (ImGui::CollapsingHeader("Color Settings")) {
					ImGui::Text("--Background Color--");
					if (ImGui::ColorPicker3("##bg_color", &arrBGcolor[0])) {
						currentBGColor = { arrBGcolor[0], arrBGcolor[1], arrBGcolor[2] };
					}
					ImGui::Text("--Natural 20 Color--");
					ImGui::ColorPicker3("##20color", &nat20Color[0]);
					ImGui::Text("--Natural 1 Color--");
					ImGui::ColorPicker3("##1color", &nat1Color[0]);
				}
				ImGui::End();
			}
			else {
				ImGui::Begin("Menu");
				if (ImGui::Button("Settings")) {
					settingsOpen = !settingsOpen;
				}
				if (ImGui::Button("Save and Quit")) {
					SaveUserData(saveFileName);
					makeNewMessage = false;
					loaded = false;
					load_message = "Success! Saved to file '";
					load_message += saveFileName;
					load_message += "'.";
				}
				ImGui::End();
			}
		}

		ImGui::PopStyleColor(1);
		ImGui::End();
		ImGui::PopFont();
	}

	void SaveUserData(std::string filename) {
		SaveData dat;

		dat.sections.insert({ "SETTINGS", {} });
		dat.addString("SETTINGS", "notepad", noteBox);
		dat.addInt("SETTINGS", "notepadWidth", notepadWidth);
		dat.addInt("SETTINGS", "notepadHeight", notepadHeight);
		dat.addInt("SETTINGS", "maxHealth", healthMax);
		dat.addInt("SETTINGS", "currentHealth", healthCurrent);
		dat.addInt("SETTINGS", "tempHealth", tempHP);
		dat.addFloat("SETTINGS", "bgColorR", arrBGcolor[0]);
		dat.addFloat("SETTINGS", "bgColorG", arrBGcolor[1]);
		dat.addFloat("SETTINGS", "bgColorB", arrBGcolor[2]);

		std::string statString = "";
		for (auto& stat : statistics)
		{
			statString += stat.first + "," + std::to_string(stat.second) + ",";
		}
		dat.sections["SETTINGS"].lists.insert({ "statistics", {statString} });

		ItemReader::SaveDataToFile(filename, dat, true);
	}

	void Shutdown() override {

		if (loaded) {
			SaveUserData(saveFileName);
		}
	}
};

std::vector<App*> CreateGame() {
	return { new DND };
}

//rainbow text
//shape of dice over roller button