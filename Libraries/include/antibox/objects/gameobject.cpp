#include "gameobject.h"
#include "../core/engine.h"


void antibox::GameObject::Update() {
	mSprite->UpdateSprite(mPos, mSize);
}

void antibox::GameObject::Render() {
	mSprite->RenderSprite();
}

void antibox::GameObject::CreateSprite(const glm::vec2 pos, const glm::vec2 size, std::string texture_path) {

	auto ret = std::make_shared<antibox::Sprite>(pos, size, texture_path);
	mSprite = ret;
}