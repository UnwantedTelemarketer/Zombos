#include "engine.h"
#include "app.h"
#include "../graphics/framebuffer.h"
#include "log.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define ANTIBOX_SUBMIT_RC(type, ...) std::move(std::make_unique<antibox::render::type>(__VA_ARGS__));

namespace antibox {

	WindowProperties::WindowProperties() //Default Window Properties if none are written
	{
		title = "Antibox";
		w = 800;
		h = 600;
		vsync = 0; //0 is disabled, 1 is enabled.
		cc = { 0.45f,0.45f,0.45f , 1.0f}; //cc means Clear Color, as in background color when the screen is cleared.
		framebuffer_display = false;
	}

	Window::Window(const unsigned int width, const unsigned int height, const char* windowName)
	{
		glfwInit();
		Window::width = width;
		Window::height = height;
		Window::windowName = windowName;
	}
	void Window::BeginRender() { //This should send the framebuffer that is currently in use to the front
		auto& rm = Engine::Instance().GetRenderManager();
		rm.Clear();
		if (showFramebuffer)
		{
			rm.Submit(std::move(std::make_unique<antibox::render::PushFramebuffer>(mFramebuffer)));
		}
	}

	void Window::EndRender() { //This pops the framebuffer currently in use, flushes all remaining rendercommands and then runs ImGuiRender and swaps buffers
		auto& rm = Engine::Instance().GetRenderManager();
		if (showFramebuffer) { rm.Submit(std::move(std::make_unique<antibox::render::PopFramebuffer>())); }
		rm.Flush();

		mImguiWindow.BeginRender();
		Engine::Instance().GetApp().ImguiRender();

		if (showFramebuffer) {
			ImGui::Begin("Framebuffer");
			ImGui::Image((void*)GetFramebuffer()->GetTextureID(), { ImGui::GetWindowSize().x - 40, ImGui::GetWindowSize().y - 40 });
			ImGui::End();
		}

		mImguiWindow.EndRender(); 


		glfwSwapBuffers(win);
	}

	bool Window::init(const WindowProperties& props) { // Window Properties
		// Create a glfw window object of width by height pixels, naming it whatever the window name is
		Window::win = glfwCreateWindow(props.w, props.h, props.title.c_str(), NULL, NULL);
		// Error check if the window fails to create
		if (win == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			return -1;
		}
		// Introduce the window into the current context
		glfwMakeContextCurrent(Window::win);

		glfwSwapInterval(props.vsync);

		//Load GLAD so it configures OpenGL
		gladLoadGL();
		// Specify the viewport of OpenGL in the Window
		// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
		glViewport(0, 0, width, height);

		Engine::Instance().GetRenderManager().SetClearColor(props.cc);

		mFramebuffer = std::make_shared<Framebuffer>(props.w, props.h);
		mFramebuffer->SetClearColor(props.cc); // props.cc

		mImguiWindow.Create(props.imguiProps);
		showFramebuffer = props.framebuffer_display;
		
		//glEnable(GL_STENCIL_TEST);
	}

	void Window::UpdateCC(glm::vec4 props)
	{
		Engine::Instance().GetRenderManager().SetClearColor(props);
		mFramebuffer->SetClearColor(props); // props.cc
	}
	
	glm::ivec2 Window::GetSize() {
		int w, h;
		glfwGetWindowSize(win, &w, &h);
		return glm::ivec2(w, h);
	}
}