#include "antibox/core/antibox.h"
#include "json/json.h"
#include <fstream>

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
	float display_time = 3;
	float minRollTime = 1;
	float maxRollTime = 2;
	int rollModifier = 0;
	bool rollWithMod = false;
	int healthMax = 0;
	int healthCurrent = 0;
	int healthModification = 0;

	float nat20Color[3] = { 0,1,0 };
	float nat1Color[3] = { 1,0,0 };

	float bgColor[3] = { 0.2,0.2,0.2 };
	float currentBGColor[3] = { 0.2,0.2,0.2 };
	//Scene main = { "Sprites" };
	//std::shared_ptr<GameObject> stat_page;
	bool loaded = false;



	void Init() override {
		//Engine::Instance().AddScene(&main);
		//main.CreateObject("Stats", { -1,1 }, { 2,-2 }, "res/stats.png");
		//stat_page = main.FindObject("Stats");

		//std::ifstream f("res/statsheet.json");
		//json data = json::parse(f);
		//std::string name = data["data"]["name"].template get<std::string>();

		//Console::Log(data["message"].template get<std::string>(), ERROR, __LINE__);
		//Console::Log(name, ERROR, __LINE__);
	}

	void Update() override {

	}

	void ImguiRender() override {
		if (!loaded) {
			if (ImGui::Button("Load")) {
				Rendering::SetFramebufferMode(true);
				loaded = true;
			}
			return;
		}

		ImGui::PushFont(Engine::Instance().getFont("ui"));
		ImGui::Begin("Settings");
		ImGui::Text("--Disappear Time--");
		ImGui::SliderFloat("##1", &display_time, 0.5, 5);
		ImGui::Text("--Minimum Roll Time--");
		ImGui::SliderFloat("##2", &minRollTime, 0.5, 5);
		ImGui::Text("--Maximum Roll Time--");
		ImGui::SliderFloat("##3", &maxRollTime, 0.5, 5);

		if (ImGui::CollapsingHeader("Color Settings")) {
			ImGui::Text("--Background Color--");
			if (ImGui::Button("Update BG Color")) {
				Rendering::SetBackgroundColor({ bgColor[0], bgColor[1], bgColor[2], 1 });
			}
			ImGui::ColorPicker3("##bg_color", &bgColor[0]);
			ImGui::Text("--Natural 20 Color--");
			ImGui::ColorPicker3("##20color", &nat20Color[0]);
			ImGui::Text("--Natural 1 Color--");
			ImGui::ColorPicker3("##1color", &nat1Color[0]);
		}
		ImGui::End();

		ImGui::Begin("Stats");
		ImGui::Text("Max Health");
		ImGui::DragInt("##mHealth", &healthMax);
		if (ImGui::Button("Reset to Max")) {
			healthCurrent = healthMax;
		}
		ImGui::Text(("Current Health : " + std::to_string(healthCurrent)).c_str());
		ImGui::DragInt("##hMod", &healthModification);
		if (ImGui::Button("Damage")) {
			healthCurrent -= healthModification;
		}
		ImGui::SameLine();
		if (ImGui::Button("Heal")) {
			healthCurrent += healthModification;
		}
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
				dieColor = { nat20Color[0],nat20Color[1] ,nat20Color[2], 1 };
			}
			if (rolling_dice[i].result == 1 && !rolling_dice[i].rolling) {
				dieColor = { nat1Color[0],nat1Color[1] ,nat1Color[2], 1 };
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
		ImGui::End();
		ImGui::PopFont();
	}

	void Shutdown() override {

	}
};

std::vector<App*> CreateGame() {
	return { new DND };
}