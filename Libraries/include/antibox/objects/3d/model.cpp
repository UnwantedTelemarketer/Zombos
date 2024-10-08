#include "model.h"
#include <antibox/graphics/rendercommands.h>
#include "antibox/core/engine.h"
#include "antibox/managers/factory.h"
#include <iostream>

using namespace antibox;

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

void Model::UpdateModel() {
	Engine::Instance().mainCamera->Matrix(45.f, 0.1f, 100.f, mShader.get(), "camMatrix");
}


void Model::RenderModel() {
	auto rc = std::make_unique<render::RenderMeshTextured>(mMesh, mTexture, mShader);
	Engine::Instance().GetRenderManager().Submit(std::move(rc));
	Engine::Instance().GetRenderManager().Flush();
}