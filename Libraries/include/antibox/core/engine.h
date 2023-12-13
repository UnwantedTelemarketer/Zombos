#pragma once
#include "audio.h"
#include "window.h"
#include "../managers/rendermanager.h"
#include "antibox/graphics/framebuffer.h"
#include "antibox/graphics/helpers.h"
#include "antibox/core/mathlib.h"
#include <antibox/objects/scene.h>


//=======================================================
#define MD_FLAG antibox::Engine::Instance().mouseDownFlag
#define KD_FLAG antibox::Engine::Instance().keyDownFlag
#define KD_CODE antibox::Engine::Instance().keyDownCode
//=======================================================


namespace antibox {
	class App;
	class Engine {
	public:
		unsigned int window_w, window_h; //Window height and width
		Window* window; //Reference to the window

		bool mouseDownFlag; //Flag used to see if a mouse button is held down, used in the antibox.h input section
		bool keyDownFlag; //Flag used to see if a key is held down
		int keyDownCode; //Variable holding the last pressed key

		//Singleton for engine
		static Engine& Instance(); 

		//Play sound once at file path
		void StartSound(const char* path); 
		~Engine(); //Destructor

		
		void End(); //Called when window is closed
		void CloseApp() { closeApp = true; } //close the loop

		void Run(App* app); //The constant loop every frame
		inline App& GetApp() { return *mApp; } //Returns App
		Window* GetWindow() { return window; } //Returns Window
		inline render::RenderManager& GetRenderManager() { return mRenderManager; } //Returns RenderManager
		//Returns the framerate.
		double getFPS() { return fps; } 
		
		//Returns the milliseconds between frames.
		double deltaTime() { return ms; }

		//Returns the custom font uploaded via WindowProperties.
		ImFont* getFont() { return window->imwin().mainFont; }

		bool AddScene(Scene* sc);
		//Scene* GetScene(std::string name);

	private:

		Engine();
		App* mApp; //Pointer to the current app. There should only ever be one app.
		AudioEngine* mAudio; //Audio manager
		static Engine* mInstance; //Local reference to the engine singleton

		render::RenderManager mRenderManager; //RenderManager takes in Render Commands for rendering

		double prevtime = 0.0; //delta time stuff
		double crntTime = 0.0; 
		double timeDiff; 
		double fps;
		double ms;
		unsigned int counter = 0;
		bool closeApp;

		void Update(); //Self Explanatory
		void Render();
		void Initialize();

		std::vector<Scene*> mScenes;
	};
}
