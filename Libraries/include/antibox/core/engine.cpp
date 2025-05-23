#include "engine.h"
#include "app.h"
#include "../graphics/shader.h"
#include <iostream>
#include <fstream>  
#include <chrono>

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
		mainCamera = new Camera(window_w, window_h, glm::vec3(0.f, 0.f, 2.f));
	}

	void Engine::SetAppList(std::vector<App*> apps) {
		mAppList = apps;
	}


	void Engine::Initialize() { //Do all the initialization
		srand((unsigned)time(NULL)); //for rng
		WindowProperties props = mApp->GetWindowProperties();
		window->init(props);
		mRenderManager.Init();
		mApp->Init();
		Console::Log(mAudio->init(), text::red, __LINE__);
		prevtime = glfwGetTime();
	}

	void Engine::InitializeApp(App* app) {
		mApp->Init();
	}

	void Engine::LerpFloat(std::string lerpID, float* val, float endVal, float time) {
		if (floatsToLerp.count(lerpID) == 0) {
			//Console::Log(lerpID + " already on lerp stack. Overwritting...", WARNING, __LINE__);
			floatsToLerp.erase(lerpID);
		}
		floatsToLerp.insert({ lerpID, { val, time, *val, endVal, 0 } });
	}


	void Engine::Run() { //This is what loops forever until the window is closed
		if (mApp == nullptr) { mApp = mAppList[0]; }//If we dont have an app, set the private app to the one submitted from wherever run is called.
		else { return; } //if there is no app anywhere, just dont run
		Initialize();

		closeScene = false;
		while (!closeScene) //This is the window loop from GLFW.
		{
			if (appToChangeTo != -1) {
				ChangeApp(appToChangeTo);
				appToChangeTo = -1;
			}
			Update(); //Run the Update function
			Render(); //Run the Render Function
			closeScene = glfwWindowShouldClose(window->glfwin());
		}
		End(); //Once the user closes the window, run End
		std::ofstream myfile("output_log.txt");
		for (size_t i = 0; i < Console::allLogs.size(); i++)
		{
			myfile << Console::allLogs[i];
		}
		myfile.close();
	}

	void Engine::ChangeApp(int appNum) {
		mApp->Shutdown();
		mApp = mAppList[appNum];
		InitializeApp(mApp);
	}

	void Engine::RotateCam() {
		if (movingCam)
		{
			// Hides mouse cursor
			glfwSetInputMode(window->glfwin(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			// Prevents camera from jumping on the first click
			if (firstClick)
			{
				glfwSetCursorPos(window->glfwin(), (window_w / 2), (window_h / 2));
				firstClick = false;
			}

			// Stores the coordinates of the cursor
			double mouseX;
			double mouseY;
			// Fetches the coordinates of the cursor
			glfwGetCursorPos(window->glfwin(), &mouseX, &mouseY);

			// Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
			// and then "transforms" them into degrees 
			float rotX = mainCamera->sensitivity * (float)(mouseY - (window_h / 2)) / window_h;
			float rotY = mainCamera->sensitivity * (float)(mouseX - (window_w / 2)) / window_w;

			// Calculates upcoming vertical change in the Orientation
			glm::vec3 newOrientation = glm::rotate(mainCamera->orientation, glm::radians(-rotX), glm::normalize(glm::cross(mainCamera->orientation, mainCamera->up)));

			// Decides whether or not the next vertical Orientation is legal or not
			if (abs(glm::angle(newOrientation, mainCamera->up) - glm::radians(90.0f)) <= glm::radians(85.0f))
			{
				mainCamera->orientation = newOrientation;
			}

			// Rotates the Orientation left and right
			mainCamera->orientation = glm::rotate(mainCamera->orientation, glm::radians(-rotY), mainCamera->up);

			// Sets mouse cursor to the middle of the screen so that it doesn't end up roaming around
			glfwSetCursorPos(window->glfwin(), (window_w / 2), (window_h / 2));
		}
		else
		{
			// Unhides cursor since camera is not looking around anymore
			glfwSetInputMode(window->glfwin(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			// Makes sure the next time the camera looks around it doesn't jump
			firstClick = true;
		}
	}

	void Engine::Update() {
		window->BeginRender(); //Start the rendering from window

		auto crntTime = glfwGetTime();
		double elapsed = crntTime - prevtime;
		ms = elapsed * 1000.f;
		prevtime = crntTime;
		timePassed += ms;
		counter++;

		if (timePassed >= 1000.f) {
			fps = counter;
			timePassed = 0;
			counter = 0;
		} //report the framerate and ms between frames
		

		//Lerp all currently lerping things
		if (!floatsToLerp.empty()) {
			for (auto currentPack = floatsToLerp.begin(); currentPack != floatsToLerp.end();) {
				lerp_pack* pack = &currentPack->second;
				pack->elapsedTime += deltaTime() / 1000.f;

				float normalizedTime = std::min(pack->elapsedTime / pack->endTime, 1.0f);

				if(pack->valToChange){
					*pack->valToChange = Math::Lerp(normalizedTime, pack->startingVal, pack->endVal);
				}
				else{
					Console::Log(currentPack->first + " is nullptr. Removing from lerp stack...", ERROR, __LINE__);
					currentPack = floatsToLerp.erase(currentPack);
					++currentPack;
					continue;
				}

				if (normalizedTime >= 1.0f) {
					*pack->valToChange = pack->endVal;
					currentPack = floatsToLerp.erase(currentPack); // Erase and get the next valid iterator
				}
				else {
					++currentPack; // Move to the next element
				}
			}
		}


		if(movingCam) RotateCam();

		mApp->Update(); //users update function

		//Update all scenes added to the game
		/*for (int i = 0; i < mScenes.size(); i++)
		{
			mScenes[i]->UpdateObjs();
		}*/
		if (mScenes.size() != 0) {
			mScenes[currentSceneID]->UpdateObjs();
		}
	}

	void Engine::Render() {
		glfwPollEvents(); //Take in mouse and keyboard inputs

		mApp->Render(); //Users render function.

		//Render all scenes added to the game
		/*for (int i = 0; i < mScenes.size(); i++)
		{
			mScenes[i]->RenderObjs();
		}*/
		if (mScenes.size() != 0) {
			mScenes[currentSceneID]->RenderObjs();
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

	void Engine::StartSound(const char* path, std::string name, bool loop)
	{
		if (loop) { mAudio->PlayAudioLooping(path, name);  return; }
		mAudio->PlayAudio(path);
	}

	void Engine::StopSoundLooping(std::string name) {
		mAudio->StopAudioLooping(name);
	}

	void Engine::SetVolume(float volume) {
		mAudio->SetVolume(volume);
	}

	void Engine::SetVolumeLoop(float volume, std::string name) {
		mAudio->SetVolumeLoop(volume, name);
	}


	float Engine::GetVolume()
	{
		return mAudio->GetVolume();
	}

	Engine::~Engine() {
		delete window;
		delete mainCamera;
	}
}
