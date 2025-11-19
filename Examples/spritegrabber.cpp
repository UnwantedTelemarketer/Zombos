
#include "antibox/core/antibox.h"
#include <iomanip>

using namespace antibox;

class Sprites : public App {
	WindowProperties GetWindowProperties() {
		WindowProperties props;
		props.w = 260;
		props.h = 180;
		props.title = "Conversion";
		props.cc = { 0.2f, 0.2f, 0.2f, 1.f };
		props.framebuffer_display = true;
		props.vsync = 1;
		return props;
	}

	const int WIDTH = 44;
	const int HEIGHT = 12;

	int x = 0;
	int y = 0;


	Scene main = { "TEST" };
	std::stringstream sstream;

	void Init() override {
		Engine::Instance().AddScene(&main);
	}



	void Update() override {
		sstream.str("");
	}

	void Render() override {

	}

	void ImguiRender() override {


		ImGui::Begin("Object Data");

		ImGui::InputInt("Column", &x);
		ImGui::InputInt("Row", &y);

		//Convert it to string
		int coord = 0xE000;
		coord += x + (WIDTH * y);

		sstream << "0x" << std::hex << coord;

		ImGui::Text(sstream.str().c_str());

		ImGui::End();
	}
	void Shutdown() override {

	}
};