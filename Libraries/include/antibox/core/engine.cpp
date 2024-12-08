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
		mAudio->init();
		prevtime = std::chrono::high_resolution_clock::now();
	}

	void Engine::InitializeApp(App* app) {
		mApp->Init();
	}

	void Engine::LerpFloat(float* val, float endVal, float time) {
		floatsToLerp.push_back({ val, time, *val, {endVal, 0} });
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

		auto crntTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsed = crntTime - prevtime;
		ms = elapsed.count() * 1000.f;
		prevtime = crntTime;
		timePassed += ms;
		counter++;

		if (timePassed >= 1000.f) {
			fps = counter;
			timePassed = 0;
			counter = 0;
		} //report the framerate and ms between frames
		
		//Lerp all currently lerping things
		for (size_t i = 0; i < floatsToLerp.size(); i++)
		{
			lerp_pack* pack = &floatsToLerp[i];
			pack->endVal_Time.y += deltaTime() / 1000.f;
			float normalizedTime = pack->endVal_Time.y / pack->startingTime;
			*pack->val = Math::Lerp(normalizedTime, pack->startingVal, pack->endVal_Time.x);
			if (*pack->val >= pack->endVal_Time.x) 
			{ 
				*pack->val = pack->endVal_Time.x;
				floatsToLerp.erase(floatsToLerp.begin() + i); 
				i--;
				if (i >= floatsToLerp.size()) { break; }
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

	void Engine::StartSound(const char* path, bool loop)
	{
		if (loop) { mAudio->PlayAudioLooping(path);  return; }
		mAudio->PlayAudio(path);
	}

	void Engine::SetVolume(float volume) {
		mAudio->SetVolume(volume);
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
