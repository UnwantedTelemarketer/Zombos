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
	
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
		glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WIN32);
#elif __linux__
		// Default to X11 if not running Wayland
		glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); 
#elif __unix__
		glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#elif __APPLE__
		glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_COCOA);
#endif
		if (!glfwInit()) {
			std::cout << "Error initializing GLFW." << std::endl;
		}
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
			ImGui::Image(GetFramebuffer()->GetTextureID(), { ImGui::GetWindowSize().x - 40, ImGui::GetWindowSize().y - 40 });
			ImGui::End();
		}

		if (engineConsoleVisible) {
			ImGui::Begin("Engine Console");
			for (size_t i = 0; i < Console::allLogs.size(); i++)
			{
				ImGui::Text(Console::allLogs[i].c_str());
			}
			ImGui::End();
		}

		mImguiWindow.EndRender(); 


		glfwSwapBuffers(win);
	}

	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
		// Also update your projection matrix here if needed
	}

	void Window::GetScreenSize(int& w, int& h) {
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		w = mode->width;
		h = mode->height;
	}

	bool Window::init(const WindowProperties& props) { // Window Properties
		// Create a glfw window object of width by height pixels, naming it whatever the window name is
		//
		#ifdef __APPLE__
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
			glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
		#endif

		Window::win = glfwCreateWindow(props.w, props.h, props.title.c_str(), NULL, NULL);
		// Error check if the window fails to create
		if (win == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			return false;
		}

		// Introduce the window into the current context
		glfwMakeContextCurrent(Window::win); 

		glfwSwapInterval(props.vsync); 


		if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ 
			std::cout << "Failed to initialize GLAD" << std::endl;
			return false;
		}

		// Specify the viewport of OpenGL in the Window
		// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
		glViewport(0, 0, width, height); ANTIBOX_CHECK_GL_ERROR;

		Engine::Instance().GetRenderManager().SetClearColor(props.cc);

		mFramebuffer = std::make_shared<Framebuffer>(props.w, props.h);
		mFramebuffer->SetClearColor(props.cc); // props.cc

		mImguiWindow.Create(props.imguiProps);
		showFramebuffer = props.framebuffer_display;

		//set the callback to change the projection of the camera on the screen resizing
		glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);


		return true;
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
