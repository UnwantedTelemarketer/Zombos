
#include "antibox/core/antibox.h"

using namespace antibox;

//-------------------
//ADD VISUAL EDITOR IN ENGINE
//allow use of a function like "loadVisualEditor()"
//-------------------

class Raycaster : public App {
	WindowProperties GetWindowProperties() {
		WindowProperties props;
		props.w = 800;
		props.h = 600;
		props.title = "Raycaster";
		props.cc = { 0.2f, 0.2f, 0.2f, 1.f };
		props.framebuffer_display = false;
		props.vsync = 1;
		return props;
	}

	Scene main = { "TEST" };
	std::shared_ptr<GameObject> player;
	Model* pyramid;
	float rotator = 0.f;
	float angle = 0.f;
	bool fbFlip, wireframe = false;
	

	void Init() override {
		Engine::Instance().AddScene(&main);
		main.CreateObject("Player", { -0.7,-1 }, { 0.25,0.25 }, "res/image.png");
		main.CreateObject("Box", { 0.7,0.75 }, { 0.25,0.25 }, "res/box.png");
		player = main.FindObject("Player");
		pyramid = new Model({1.f,1.f,1.f}, { 1.f,1.f,1.f }, "res/brick.jpg");
		Engine::Instance().GetRenderManager().SetWireframeMode(false);
	}

	void Update() override {
		rotator += Engine::Instance().deltaTime();
		if (rotator >= 1.f) {
			angle += 0.5f;
		}
		pyramid->UpdateModel({ 0.f,-0.5f,-2.f }, angle, { 1.f,-1.f,1.f });
	}

	void Render() override {
		pyramid->RenderModel();
	}
	
	void ImguiRender() override {
		ImGui::Begin("Settings");
			if (ImGui::Button("Toggle Framebuffer")) {
				fbFlip = !fbFlip;
				Engine::Instance().GetWindow()->UseFramebuffer(fbFlip);
			}
			if (ImGui::Button("Toggle Wireframe")) {
				wireframe = !wireframe;
				Engine::Instance().GetRenderManager().SetWireframeMode(wireframe);
			}
		ImGui::End();
	}

	void Shutdown() override {
		Console::Log("Shutting down!", text::red, __LINE__);
	}
};

std::vector<App*> CreateGame() {
	return { new Raycaster };
}