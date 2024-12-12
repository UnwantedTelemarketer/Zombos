#pragma once
#include "engine.h"
#include "app.h"
#include <iostream>

//PRINTSCREEN creates an auto reference to the current window (screen) and displays the framebuffer that it gets onto an ImGui Image tab.
#define PRINTSCREEN auto& window = *Engine::Instance().GetWindow(); ImGui::Image((void*)window.GetFramebuffer()->GetTextureID(), { ImGui::GetWindowSize().x , ImGui::GetWindowSize().y });

#define TO_CHARARR(number) std::to_string(number).c_str()

//To be implemented in clients app
//client returns pointer to instance of class derived from app
//ownership of returned pointer belongs to antibox, and will be managed as such

//example:
//class ClientApp : public antibox::App {};
//antibox::App* CreateApp() { return new ClientApp(); }

std::vector<antibox::App*> CreateGame();


int main() {
	bool menu = true;
	antibox::Engine& engine = antibox::Engine::Instance();
	engine.SetAppList(CreateGame());

	engine.Run();

	return 0;
}

namespace antibox {

	namespace Utilities {
		float deltaTime() { return antibox::Engine::Instance().deltaTime(); }
		float getFPS() { return antibox::Engine::Instance().getFPS(); }
		void Lerp(float* val, float endVal, float time) { antibox::Engine::Instance().LerpFloat(val, endVal, time); }
		void Lerp(Vector3* val, Vector3 endVal, float time) {
			antibox::Engine::Instance().LerpFloat(&val->x, endVal.x, time);
			antibox::Engine::Instance().LerpFloat(&val->y, endVal.y, time);
			antibox::Engine::Instance().LerpFloat(&val->z, endVal.z, time);
		}
	}

	namespace Audio {
		//Between 0f and 1f.
		void SetVolume(float vol) { antibox::Engine::Instance().SetVolume(vol); }
		//Returns the volume as a float.
		float GetVolume() { return antibox::Engine::Instance().GetVolume(); }
		//Provide path to audio file.
		void Play(std::string path) { antibox::Engine::Instance().StartSound(path.c_str(), "null", false); }
		void PlayLoop(std::string path, std::string name) { antibox::Engine::Instance().StartSound(path.c_str(), name, true); }
		void StopLoop(std::string name) { antibox::Engine::Instance().StopSoundLooping(name); }
		//Does nothing right now
		void Stop(std::string path) { Console::Log("Not implemented :/", text::white, __LINE__); }
	}

	namespace Rendering {
		//Changes 2d and 3d rendering between wireframe or filled.
		void SetWireframeMode(bool wireframe) { Engine::Instance().GetRenderManager().SetWireframeMode(wireframe); }
		//Changes whether to render to the screen directly or to a framebuffer.
		void SetFramebufferMode(bool fb) { Engine::Instance().GetWindow()->UseFramebuffer(fb); }
	}
	
	namespace Editor {
		//Adds a scene full of gameobjects to the editor. (Think of it as a base node)
		void AddScene(Scene* scene) { Engine::Instance().AddScene(scene); }


	}

#ifndef INPUT_FUNCS

#define INPUT_FUNCS 
	namespace Input {

		//Returns true if the key is held, false if not.
		bool KeyHeldDown(int keycode) { return glfwGetKey(antibox::Engine::Instance().GetWindow()->glfwin(), keycode); }


		//Returns true if the mouse button is held, false if not.
		bool MouseButtonHeld(int mouseButton) { return glfwGetMouseButton(antibox::Engine::Instance().GetWindow()->glfwin(), mouseButton); }

		//Returns true if the key is pressed once, false if not.
		bool KeyDown(int keycode) {
			int state = glfwGetKey(antibox::Engine::Instance().GetWindow()->glfwin(), keycode); //glfw getting mouse down
			if (state == GLFW_PRESS && !KD_FLAG) {
				KD_CODE = keycode;
				KD_FLAG = true;
				return true;
			}
			else if (state == GLFW_RELEASE && KD_FLAG) {
				if (KD_CODE != keycode) { return false; }
				KD_FLAG = false;
				return false;
			}
			return false;
		}

		//Returns true if the mouse button is pressed once, false if not.
		bool MouseButtonDown(int mouseButton) {
			int state = glfwGetMouseButton(antibox::Engine::Instance().GetWindow()->glfwin(), mouseButton); //glfw getting mouse down
			if (state == GLFW_PRESS && !MD_FLAG) {
				MD_FLAG = true;
				return true;
			}
			else if (state == GLFW_RELEASE && MD_FLAG) {
				MD_FLAG = false;
				return false;
			}
			return false;
		}

	}

#endif

}


class VisualEditor : public antibox::App {
	antibox::WindowProperties GetWindowProperties() {
		antibox::WindowProperties props;
		props.w = 1280;
		props.h = 720;
		props.title = "Antibox Editor";
		props.cc = { 0.22f, 0.2f, 0.2f, 1.f };
		props.framebuffer_display = true;
		props.vsync = 1;
		props.imguiProps = { true, true, false, {"Libraries\\include\\antibox\\font\\VGA437.ttf"}, {"vga"}, 16.f };
		return props;
	}

	antibox::Scene mainscene = {"main"};
	std::shared_ptr<antibox::GameObject> currentItem;
	int selectedObj = 0;
	std::string selectedObjName;

	void Init() override {
		Console::Log("Editor started!", SUCCESS, __LINE__);
		antibox::Engine::Instance().AddScene(&mainscene);
	}

	void ImguiRender() override {

		ImGui::Begin("Hierarchy");
		if (ImGui::BeginListBox("##"))
		{
			std::vector<std::string> objects = mainscene.GetObjNames();
			for (int n = 0; n < objects.size(); n++)
			{
				const bool is_selected = (selectedObj == n);
				if (ImGui::Selectable(objects[n].c_str(), is_selected)) {
					selectedObjName = objects[n];
					selectedObj = n;
					if (currentItem->GetName() != selectedObjName) { currentItem = mainscene.FindObject(selectedObjName); }
				}
			}
			ImGui::EndListBox();
		}

		if (ImGui::Button("Create GameObject")) {
			mainscene.CreateObject("Player", { -0.7,-1 }, { 0.25,0.25 }, "res/image.png");
		}
		ImGui::End();

		ImGui::Begin("Inspector");
		if (selectedObjName != "") {
			ImGui::Text(selectedObjName.c_str());
		}
		ImGui::End();
	}
};