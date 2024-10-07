#pragma once
#include <memory>

#include "antibox/graphics/shader.h"
#include "antibox/graphics/mesh.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace antibox {
	class Model {
	private:
		float xColor;
		float yColor;
		std::shared_ptr<Mesh> mMesh;
		std::shared_ptr<Shader> mShader;
		std::shared_ptr<Texture> mTexture;
		glm::vec2 mRectPos, mRectSize;


		bool alreadyMade = false;

	public:
		void UpdateModel(const glm::vec3 pos, float rot, const glm::vec3 size);
		void RenderModel();

		//void ChangeColor();

		Model(const glm::vec3 pos, const glm::vec3 size, std::string texture_path);
	};
}