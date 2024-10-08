#include "scene.h"
#include <antibox/core/log.h>

namespace antibox {


	void Scene::CreateObject(std::string name, glm::vec2 pos, glm::vec2 size, std::string texture_path) {
		std::shared_ptr<GameObject> go = std::make_shared<GameObject>(name, pos, size, texture_path);
		Hierarchy.insert({ name, go });
		Console::Log("Succesfully created object " + name, "\033[1;32m", __LINE__);
	}

	std::shared_ptr<GameObject> Scene::FindObject(const std::string name) {
		return Hierarchy[name];
	}

	Scene::Scene(std::string name = "Default") {
		sceneName = name;
		sceneID = 1;
	}
	std::vector<std::string> Scene::GetObjNames() {
		std::vector<std::string> names;
		for (auto& objpair : Hierarchy)
		{
			names.push_back(objpair.first);
		}
		return names;
	}

	Scene::~Scene() {

	}

	void Scene::UpdateObjs() {
		for (auto const& x : Hierarchy)
		{
			x.second->Update();
		}
	}

	void Scene::RenderObjs() {
		for (auto const& x : Hierarchy)
		{
			x.second->Render();
		}
	}
}