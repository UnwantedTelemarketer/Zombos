
#include "antibox/core/antibox.h"

using namespace antibox;

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
	//std::shared_ptr<GameObject> player;
	Model* pyramid;
	float rotator = 0.f;
	float angle = 0.f;
	bool fbFlip, wireframe = false;
	float lighting = 1.f;
	float color[3] = {1,1,1};

	void Init() override {
		Editor::AddScene(&main);

		pyramid = new Model({1.f,1.f,1.f}, { 1.f,1.f,1.f }, "res/brick.jpg");
	}

	void Update() override {

		pyramid->UpdateIntensity(lighting);
		pyramid->UpdateColor({ color[0], color[1], color[2] });
		pyramid->UpdateModel();

		if (Input::KeyDown(KEY_C)) {
			Engine::Instance().movingCam = !Engine::Instance().movingCam;
		}
		if (Input::KeyHeldDown(KEY_W)) {
			Engine::Instance().mainCamera->MoveCamera(0);
		}
		if (Input::KeyHeldDown(KEY_A)) {
			Engine::Instance().mainCamera->MoveCamera(1);
		}
		if (Input::KeyHeldDown(KEY_S)) {
			Engine::Instance().mainCamera->MoveCamera(2);
		}
		if (Input::KeyHeldDown(KEY_D)) {
			Engine::Instance().mainCamera->MoveCamera(3);
		}
		if (Input::KeyHeldDown(KEY_SPACE)) {
			Engine::Instance().mainCamera->MoveCamera(4);
		}
		if (Input::KeyHeldDown(KEY_LEFT_CONTROL)) {
			Engine::Instance().mainCamera->MoveCamera(5);
		}
	}

	void Render() override {
		pyramid->RenderModel();
	}
	
	void ImguiRender() override {
		ImGui::Begin("Settings");
			if (ImGui::Button("Toggle Framebuffer")) {
				fbFlip = !fbFlip;
				Rendering::SetFramebufferMode(fbFlip);
			}
			if (ImGui::Button("Toggle Wireframe")) {
				wireframe = !wireframe;
				Rendering::SetWireframeMode(wireframe);
			}
			ImGui::ColorPicker3("Light Color", color);
			ImGui::SliderFloat("Intensity", &lighting, 0.0f, 1.0f);
		ImGui::End();
	}

	void Shutdown() override {
		Console::Log("Shutting down!", text::red, __LINE__);
	}
};

std::vector<App*> CreateGame() {
	return { new Raycaster };
}