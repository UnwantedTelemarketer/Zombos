#include "engine.h"
#include "app.h"
#include "../graphics/shader.h"

namespace antibox
{
	Engine& Engine::Instance() 
	{
		if (!mInstance) {
			mInstance = new Engine(); //If we haven't set the engine instance yet, set it.
		}

		return *mInstance;
	}
	//singleton
	Engine* Engine::mInstance = nullptr;

	Engine::Engine() {
		mApp = nullptr;
		window_w = 800;
		window_h = 600;
		window = new Window(window_w, window_h, "name");
		mAudio = new AudioEngine();
	}

	

	void Engine::Initialize() { //Do all the initialization
		srand((unsigned)time(NULL)); //for rng
		WindowProperties props = mApp->GetWindowProperties();
		window->init(props);
		mRenderManager.Init();
		mApp->Init();
		mAudio->init();
	}


	void Engine::Run(App* app) { //This is what loops forever until the window is closed
		if (mApp == nullptr) { mApp = app; }//If we dont have an app, set the private app to the one submitted from wherever run is called.
		else { return; } //if there is no app anywhere, just dont run
		Initialize();

		closeApp = false;
		while (!closeApp) //This is the window loop from GLFW.
		{
			closeApp = glfwWindowShouldClose(window->glfwin());
			Update(); //Run the Update function
			Render(); //Run the Render Function
		}
		End(); //Once the user closes the window, run End
	}

	void Engine::Update() {
		window->BeginRender(); //Start the rendering from window

		crntTime = glfwGetTime();
		timeDiff = crntTime - prevtime;
		counter++; 
		if (timeDiff >= 1.0 / 60.0) {
			fps = (1.0 / timeDiff) * counter;
			ms = (timeDiff / counter) * 1000;
			prevtime = crntTime;
			counter = 0;
		} //report the framerate and ms between frames

		mApp->Update(); //users update function

		//Update all scenes added to the game
		for (int i = 0; i < mScenes.size(); i++)
		{
			mScenes[i]->UpdateObjs();
		}
	}

	void Engine::Render() {
		glfwPollEvents(); //Take in mouse and keyboard inputs
		
		mApp->Render(); //Users render function.
		/*for (int i = 0; i < sprites.size(); i++) //Render each sprite added to screen
		{
			sprites[i]->RenderSprite();
		}*/

		//Render all scenes added to the game
		for (int i = 0; i < mScenes.size(); i++)
		{
			mScenes[i]->RenderObjs();
		}

		window->EndRender(); //Window end render.
	}

	void Engine::End() {
		//client shutdown (?)
		mApp->Shutdown(); //Users shutdown function
		mApp = nullptr;
		mRenderManager.Shutdown();
		glfwDestroyWindow(window->glfwin());
		glfwTerminate();
	}

	bool Engine::AddScene(Scene* sc) {
		try {
			mScenes.push_back(sc);
			Console::Log("Succesfully added scene named '" + sc->GetSceneName() + "' to engine.", "\033[1;32m", __LINE__);
			return true;
		}
		catch (std::exception e) {
			Console::Log("ERROR: Failed to add scene '" + sc->GetSceneName() + "'. Details below.", text::red, __LINE__);
			Console::Log(e.what(), text::red, __LINE__);
			return false;
		}
	}

	void Engine::StartSound(const char* path)
	{
		mAudio->PlayAudio(path);
	}

	Engine::~Engine() {
		delete window;
	}
}
