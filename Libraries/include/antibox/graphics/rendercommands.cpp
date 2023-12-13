#include "rendercommands.h"
#include "../core/engine.h"
#include "../core/log.h"
#include "helpers.h"
#include "shader.h"
#include "mesh.h"
#include "texture.h"
#include "framebuffer.h" 

namespace antibox 
{
	namespace render 
	{
		void RenderMesh::Execute() 
		{
			std::shared_ptr<Mesh> mesh = mMesh.lock();
			std::shared_ptr<Shader> shader = mShader.lock();
			if (mesh && shader) 
			{
				mesh->Bind();
				shader->Bind();

				if (mesh->GetElementCount() > 0) 
				{
					glDrawElements(GL_TRIANGLES, mesh->GetElementCount(), GL_UNSIGNED_INT, 0); ANTIBOX_CHECK_GL_ERROR
				}
				else 
				{
					glDrawArrays(GL_TRIANGLE_STRIP, 0, mesh->GetVertexCount()); ANTIBOX_CHECK_GL_ERROR
				}


				shader->Unbind();
				mesh->Unbind();
			}
			else
			{
				ANTIBOX_QLOG("Executing RenderMesh with invalid data");
			}
		}

		void PushFramebuffer::Execute() 
		{
			std::shared_ptr<Framebuffer> fb = mFramebuffer.lock();
			if (fb)
			{
				Engine::Instance().GetRenderManager().PushFramebuffer(fb);
			}
			else
			{
				ANTIBOX_QLOG("Tried to push framebuffer with invalid data");
			}
		}
		void PopFramebuffer::Execute()
		{
			Engine::Instance().GetRenderManager().PopFramebuffer();
		}


		void RenderMeshTextured::Execute()
		{
			std::shared_ptr<Mesh> mesh = mMesh.lock();
			std::shared_ptr<Texture> texture = mTexture.lock();
			std::shared_ptr<Shader> shader = mShader.lock();
			if (mesh && texture && shader)
			{

				if (!mesh->IsValid() || !texture->IsValid() || !shader->IsValid())
				{
					// Handle the case where mesh, texture, or shader is not valid
					ANTIBOX_QLOG("Invalid mesh, texture, or shader");
					return;
				}

				mesh->Bind();
				texture->Bind();
				shader->Bind();

				if (mesh->GetElementCount() > 0)
				{
					glDrawElements(GL_TRIANGLES, mesh->GetElementCount(), GL_UNSIGNED_INT, 0); ANTIBOX_CHECK_GL_ERROR
				}
				else
				{
					glDrawArrays(GL_TRIANGLE_STRIP, 0, mesh->GetVertexCount()); ANTIBOX_CHECK_GL_ERROR
				}


				shader->Unbind();
				texture->Unbind();
				mesh->Unbind();
				
			}
			else
			{
				ANTIBOX_QLOG("Executing RenderMesh with invalid data");
			}
		}
	}

}