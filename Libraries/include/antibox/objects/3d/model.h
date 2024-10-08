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
		void UpdateModel();
		void RenderModel();
		void UpdateIntensity(float intense){ mShader->SetUniformFloat("intensity", intense); }
		void UpdateColor(glm::vec3 color) { mShader->SetUniformFloat4("color", color[0], color[1], color[2], 1); }

		//void ChangeColor();

		Model(const glm::vec3 pos, const glm::vec3 size, std::string texture_path);
	};
}