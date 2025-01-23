#pragma once

#include "gameobject.h"
#include <vector>

namespace antibox {
	class Scene {
	private:
		int sceneID;
		std::string sceneName;
		std::unordered_map<std::string, std::shared_ptr<GameObject>> Hierarchy;
	public:

		Scene(std::string name = "Default");
		~Scene();

		//Update and render objects
		void UpdateObjs();
		void RenderObjs();

		inline int GetSceneID() { return sceneID; }
		inline std::string GetSceneName() { return sceneName; }
		std::vector<std::string> GetObjNames();

		//Create new GameObject in this scene
		void CreateObject(std::string name, glm::vec2 pos, glm::vec2 size, std::string texture_path);

		//Find GameObject in scene by name
		std::shared_ptr<GameObject> FindObject(const std::string name);
	};
}