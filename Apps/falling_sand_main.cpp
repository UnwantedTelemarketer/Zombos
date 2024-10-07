#include "antibox/core/antibox.h"

using namespace antibox;

class Falling_Sand : public App {

	Vector2_I cursor_pos = { 10,10 };

	WindowProperties GetWindowProperties() {
		WindowProperties props;
		//props.imguiProps = { true, true, false, DOSFONT };
		props.w = 800;
		props.h = 600;
		props.title = "Sand";
		props.cc = { 0.2f, 0.2f, 0.2f, 1.f };
		return props;
	}



	void Init() override {
	}

	void Update() override {

	}

	void ImguiRender() override
	{
		ImGui::Begin("Game View");
		for (size_t i = 0; i < 10; i++)
		{
			ImGui::Text("BoB ");
			ImGui::SameLine();
		}
		ImGui::SetCursorPos({ ImGui::GetCursorPosX() / 2, ImGui::GetCursorPosY() });
		ImGui::Text("Chap");
		ImGui::End();
	}
	void Shutdown() override {

	}
};