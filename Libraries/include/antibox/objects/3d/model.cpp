#include "model.h"
#include <antibox/graphics/rendercommands.h>
#include "antibox/core/engine.h"
#include "antibox/managers/factory.h"
#include <iostream>

namespace antibox
{
	Model::Model(const glm::vec3 pos, const glm::vec3 size, std::string texture_path)
	{

		auto mesh = std::make_shared<antibox::Mesh>(&Factory::Vertices3D[0], 5, 3, &Factory::texcoords[0], &Factory::Elements3D[0], 18);
		auto shader = std::make_shared<antibox::Shader>(Factory::Default3DVert, Factory::DefaultFrag3D);
		auto texture = std::make_shared<antibox::Texture>(texture_path);
		mTexture = texture;
		mMesh = mesh;
		mShader = shader;

		mRectPos = pos;
		mRectSize = size;

		alreadyMade = true;
	}

	void Model::UpdateModel(const glm::vec3 pos, float rot, const glm::vec3 size) {
		mShader->SetUniformFloat3("color", 1, 0, 0);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 proj = glm::mat4(1.0f);

		// Assigns different transformations to each matrix
		model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
		view = glm::translate(view, pos);
		proj = glm::perspective(glm::radians(45.0f), (float)(800 / 600), 0.1f, 100.0f);

		mShader->SetUniformMat4("model", model);
		mShader->SetUniformMat4("view", view);
		mShader->SetUniformMat4("proj", proj);
	}

	void Model::RenderModel() {
		auto rc = std::make_unique<render::RenderMeshTextured>(mMesh, mTexture, mShader);
		Engine::Instance().GetRenderManager().Submit(std::move(rc));
		Engine::Instance().GetRenderManager().Flush();
	}
}