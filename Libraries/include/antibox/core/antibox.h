#pragma once
#include "engine.h"
#include "app.h"
#include <iostream>

//PRINTSCREEN creates an auto reference to the current window (screen) and displays the framebuffer that it gets onto an ImGui Image tab.
#define PRINTSCREEN auto& window = *Engine::Instance().GetWindow(); ImGui::Image((void*)window.GetFramebuffer()->GetTextureID(), { ImGui::GetWindowSize().x , ImGui::GetWindowSize().y });

#define TO_CHARARR(number) std::to_string(number).c_str()

#define MISSING_TEXTURE "Libraries/include/antibox/core/res/missing.png"


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
