#include "sprite.h"
#include <antibox/graphics/rendercommands.h>
#include "antibox/core/engine.h"
#include "antibox/managers/factory.h"
#include <iostream>

namespace antibox 
{
	Sprite::Sprite(const glm::vec2 pos, const glm::vec2 size, std::string texture_path)
	{

		auto mesh = std::make_shared<antibox::Mesh>(&Factory::Vertices2D[0], 4, 3, &Factory::texcoords[0], &Factory::Elements2D[0], 6);
		auto shader = std::make_shared<antibox::Shader>(Factory::Default2DVert, Factory::DefaultFrag2D);
		auto texture = std::make_shared<antibox::Texture>(texture_path);
		mTexture = texture;
		mMesh = mesh;
		mShader = shader;

		mRectPos = pos;
		mRectSize = size;

		alreadyMade = true;
	}

	void Sprite::UpdateSprite(const glm::vec2 pos, const glm::vec2 size) {
		mShader->SetUniformFloat3("color", 1, 0, 0);
		
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, { pos.x, pos.y, 0.f });
		model = glm::scale(model, { size.x, size.y, 0.f });
		mShader->SetUniformMat4("model", model);
	}

	void Sprite::RenderSprite() {
		auto rc = std::make_unique<render::RenderMeshTextured>(mMesh, mTexture, mShader);
		Engine::Instance().GetRenderManager().Submit(std::move(rc));
		Engine::Instance().GetRenderManager().Flush();
	}
}