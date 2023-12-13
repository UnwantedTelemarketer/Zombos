#pragma once

#include "../graphics/rendercommands.h"
#include "../graphics/sprite.h"

#include <queue>
#include <stack>


namespace antibox
{
	namespace render {
		class RenderManager
		{
			friend class PushFramebuffer;
			friend class PopFramebuffer;
		public:
			RenderManager() {}
			~RenderManager() {}

			void Init();
			void Shutdown();

			void Clear();
			void Submit(std::unique_ptr<render::RenderCommand> rc);

			//Execute submitted render commands in order of recieval
			void Flush();

			void SetViewport(const glm::ivec4 dimensions);
			void SetClearColor(const glm::ivec4 clearColor);
			void SetWireframeMode(bool enabled);

		private:
			void PushFramebuffer(std::shared_ptr<Framebuffer> framebuffer);
			void PopFramebuffer();

		private:
			std::queue<std::unique_ptr<render::RenderCommand>> mRenderCommands;
			std::stack<std::shared_ptr<Framebuffer>> mFramebuffers;
		};
	}
}