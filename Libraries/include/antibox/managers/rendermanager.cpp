#include "rendermanager.h"
#include "../core/log.h"
#include "../graphics/helpers.h"
#include "../graphics/framebuffer.h"
#include "../core/engine.h"
#include <glm/gtc/type_ptr.hpp>


namespace antibox
{
	namespace render
	{
		void RenderManager::Init()
		{
			/*
			glEnable(GL_DEPTH_TEST); ANTIBOX_CHECK_GL_ERROR
			glEnable(GL_CULL_FACE); ANTIBOX_CHECK_GL_ERROR
			glCullFace(GL_FRONT); ANTIBOX_CHECK_GL_ERROR
			//glFrontFace(GL_CW); ANTIBOX_CHECK_GL_ERROR
			*/
			glEnable(GL_DEPTH_TEST); 
			glDepthFunc(GL_LEQUAL);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); ANTIBOX_CHECK_GL_ERROR // look this up
			SetClearColor({ 0.25f, 0.25f, 0.25f, 1.f });
		}

		void RenderManager::Clear() 
		{
			while (mRenderCommands.size() > 0)
			{
				mRenderCommands.pop();
			}
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); ANTIBOX_CHECK_GL_ERROR
		}

		void RenderManager::SetViewport(const glm::ivec4 dimensions)
		{
			glViewport(dimensions.x, dimensions.y, dimensions.z, dimensions.w); ANTIBOX_CHECK_GL_ERROR
		}

		void RenderManager::Shutdown()
		{
			while (mRenderCommands.size() > 0) 
			{
				mRenderCommands.pop();
			}
		}

		void RenderManager::Submit(std::unique_ptr<render::RenderCommand> rc)
		{
			mRenderCommands.push(std::move(rc));
		}

		void RenderManager::Flush()
		{
			while (mRenderCommands.size() > 0) {
				auto rc = std::move(mRenderCommands.front());
				mRenderCommands.pop();
				rc->Execute();
			}
		}

		void RenderManager::SetClearColor(const glm::ivec4 clearColor)
		{
			glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a); ANTIBOX_CHECK_GL_ERROR
		}

		void RenderManager::SetWireframeMode(bool enabled)
		{
			if (enabled) 
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); ANTIBOX_CHECK_GL_ERROR
			}
			else
			{

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); ANTIBOX_CHECK_GL_ERROR
			}
		}

		void RenderManager::PushFramebuffer(std::shared_ptr<Framebuffer> framebuffer)
		{
			mFramebuffers.push(framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->GetFBO()); ANTIBOX_CHECK_GL_ERROR
				SetViewport({ 0, 0, framebuffer->GetSize().x, framebuffer->GetSize().y });

			glm::vec4 cc = framebuffer->GetClearColor();
			glClearColor(cc.r, cc.g, cc.b,cc.a); //TODO this is why the background color wont change. not sure why the fb cc isnt working
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); ANTIBOX_CHECK_GL_ERROR
		}

		void RenderManager::PopFramebuffer()
		{
			if (mFramebuffers.size() > 0)
			{
				mFramebuffers.pop();
				if (mFramebuffers.size() > 0)
				{
					auto nextfb = mFramebuffers.top();
					glBindFramebuffer(GL_FRAMEBUFFER, nextfb->GetFBO()); ANTIBOX_CHECK_GL_ERROR;
					SetViewport({ 0, 0, nextfb->GetSize().x, nextfb->GetSize().y });
				}
				else
				{
					glBindFramebuffer(GL_FRAMEBUFFER, 0); ANTIBOX_CHECK_GL_ERROR;
					auto& window = *Engine::Instance().GetWindow();
					SetViewport({ 0, 0, window.GetSize().x, window.GetSize().y });
				}
			}
		}

	}
	
}