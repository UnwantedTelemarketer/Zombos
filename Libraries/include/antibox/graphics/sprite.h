#pragma once
#include <memory>

#include "antibox/graphics/shader.h"
#include "antibox/graphics/mesh.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace antibox {
	class Sprite {
	private:
		float xColor;
		float yColor;
		std::shared_ptr<Mesh> mMesh;
		std::shared_ptr<Shader> mShader;
		std::shared_ptr<Texture> mTexture;
		glm::vec2 mRectPos, mRectSize;

		
		bool alreadyMade = false;

	public:
		void UpdateSprite(const glm::vec2 pos, const glm::vec2 size);
		void RenderSprite();

		//void ChangeColor();

		Sprite(const glm::vec2 pos, const glm::vec2 size, std::string texture_path);
	};
}