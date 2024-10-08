#pragma once
#include <memory>

namespace antibox
{
	class Mesh;
	class Shader;
	class Texture;
	class Framebuffer;
	namespace render 
	{
		class RenderCommand {
		public:
			virtual void Execute() = 0;
			virtual ~RenderCommand() {}
		};



		class RenderMesh : public RenderCommand
		{
		public:
			RenderMesh(std::weak_ptr<Mesh> mesh, std::weak_ptr<Shader> shader) 
			: mMesh(mesh)
			, mShader(shader)
			{}

			virtual void Execute();
		private:
			std::weak_ptr<Mesh> mMesh;
			std::weak_ptr<Shader> mShader;
		}; 


		class RenderMeshTextured3D : public RenderCommand
		{
		public:
			RenderMeshTextured3D(std::weak_ptr<Mesh> mesh, std::weak_ptr<Texture> texture, std::weak_ptr<Shader> shader)
				: mMesh(mesh)
				, mShader(shader)
				, mTexture(texture)
			{}

			virtual void Execute();
		private:
			std::weak_ptr<Mesh> mMesh;
			std::weak_ptr<Texture> mTexture;
			std::weak_ptr<Shader> mShader;
		};



		class RenderMeshTextured : public RenderCommand
		{
		public:
			RenderMeshTextured(std::weak_ptr<Mesh> mesh, std::weak_ptr<Texture> texture, std::weak_ptr<Shader> shader)
				: mMesh(mesh)
				, mShader(shader)
				, mTexture(texture)
			{}

			virtual void Execute();
		private:
			std::weak_ptr<Mesh> mMesh;
			std::weak_ptr<Texture> mTexture;
			std::weak_ptr<Shader> mShader;
		};
		


		class PushFramebuffer : public RenderCommand
		{
		public:
			PushFramebuffer(std::weak_ptr<Framebuffer> framebuffer) : mFramebuffer(framebuffer) {}
			virtual void Execute() override;
		private:
			std::weak_ptr<Framebuffer> mFramebuffer;
		};



		class PopFramebuffer : public RenderCommand
		{
		public:
			PopFramebuffer() {}
			virtual void Execute() override;

		};
	}
}