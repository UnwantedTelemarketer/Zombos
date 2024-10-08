#pragma once

#include "../graphics/sprite.h"
#include "3d/model.h"
#include <memory>

namespace antibox
{
	class GameObject {
	public:
		GameObject(std::string name, glm::vec2 pos, glm::vec2 size, std::string texture_path)
			:mName(name),
			mPos(pos),
			mSize(size)
		{
			CreateSprite(pos, size, texture_path);
		}

		void SetPos(const glm::vec2& pos) { mPos = pos; }
		void SetSize(glm::vec2 size) { mSize = size; }
		void SetColor(const glm::vec4 color) {  }
		void Move(const glm::vec2 distance) {
			mPos += distance;
		}

		const glm::vec2& GetPos() const { return mPos; }
		const glm::vec2& GetSize() const { return mSize; }
		std::string GetName() { return mName; }

		void Update();
		void Render();

	private:
		std::shared_ptr<Sprite> mSprite;
		std::string mName;
		glm::vec2 mPos, mSize;

		void CreateSprite(const glm::vec2 pos, const glm::vec2 size, std::string texture_path);
	};
}
