
#include "antibox/core/antibox.h"

using namespace antibox;

class Sprites : public App {
	WindowProperties GetWindowProperties() {
		WindowProperties props;
		props.w = 800;
		props.h = 600;
		props.title = "Sprites";
		props.cc = { 0.2f, 0.2f, 0.2f, 1.f };
		props.framebuffer_display = true;
		props.vsync = 1;
		return props;
	}

	float p_vel = 0.f;
	bool grounded = false;
	int sceneid = 1;

	Scene main = { "TEST" };
	std::shared_ptr<GameObject> player;

	void Init() override {
		Engine::Instance().AddScene(&main);
		main.CreateObject("Player", { -0.7,-1 }, { 0.25,0.25 }, "res/image.png");
		main.CreateObject("Box", { 0.7,0.75 }, { 0.25,0.25 }, "res/box.png");
		player = main.FindObject("Player");
	}



	void Update() override {

		if (player->GetPos().y < 0.725) {
			p_vel += 0.002f;
			player->Move({ 0.f, p_vel });
			grounded = false;
		}
		else {
			player->SetPos({ player->GetPos().x, 0.725f });
			p_vel = 0;
			grounded = true;
		}

		if (Input::KeyDown(KEY_W) && grounded) {
			p_vel = -0.035f;
			player->Move({ 0.f, p_vel });
		}
		if (Input::KeyHeldDown(KEY_A)) {
			player->Move({ -0.025f, 0.f });
			player->SetSize({ -0.25, 0.25 });
		}
		if (Input::KeyHeldDown(KEY_D)) {
			player->Move({ 0.025f, 0.f });
			player->SetSize({ 0.25, 0.25 });
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