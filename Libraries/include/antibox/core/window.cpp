#include "engine.h"
#include "app.h"
#include "../graphics/framebuffer.h"
#include "log.h"
// !! [ANDROID] glad + GLFW don't exist on Android; EGL/GLES3 are used instead
#ifndef __ANDROID__
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#else
// !! [ANDROID] NDK-provided EGL and OpenGL ES 3.0 headers
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#define ANTIBOX_ANDROID_TAG "antibox"
#define ANTIBOX_ALOGI(...) __android_log_print(ANDROID_LOG_INFO,  ANTIBOX_ANDROID_TAG, __VA_ARGS__)
#define ANTIBOX_ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, ANTIBOX_ANDROID_TAG, __VA_ARGS__)
#endif
// !! [ANDROID] end platform GL includes

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
		// !! [ANDROID] GLFW init only runs on desktop -- Android lifecycle handled by NativeActivity
#ifndef __ANDROID__
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
#endif
		// !! [ANDROID] end GLFW init guard
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

		// !! [ANDROID] eglSwapBuffers replaces glfwSwapBuffers on Android
#ifndef __ANDROID__
		glfwSwapBuffers(win);
#else
		if (eglDisplay != EGL_NO_DISPLAY && eglSurface != EGL_NO_SURFACE) {
			eglSwapBuffers(eglDisplay, eglSurface);
		}
#endif
		// !! [ANDROID] end swap buffers
	}

	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
		// Also update your projection matrix here if needed
	}

	void Window::GetScreenSize(int& w, int& h) {
		// !! [ANDROID] EGL surface size replaces GLFW monitor query on Android
#ifndef __ANDROID__
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		w = mode->width;
		h = mode->height;
		ConsoleLog("Monitor Resolution: " + 
			std::to_string(mode->width) 
			+ "x" + 
			std::to_string(mode->height), LOG);
#else
		if (eglDisplay != EGL_NO_DISPLAY && eglSurface != EGL_NO_SURFACE) {
			eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH,  &w);
			eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &h);
		} else {
			w = (int)width;
			h = (int)height;
		}

		ConsoleLog("Screen Resolution: " +
			std::to_string(w)
			+ "x" +
			std::to_string(h), LOG);
#endif
		// !! [ANDROID] end GetScreenSize

		
	}

	bool Window::init(const WindowProperties& props) { // Window Properties
		// !! [ANDROID] desktop GLFW init path -- skipped entirely on Android
#ifndef __ANDROID__
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

#else
		// !! [ANDROID] EGL surface + context creation
		// !!           androidNativeWindow must be set before calling init()
		if (androidNativeWindow == nullptr) {
			ANTIBOX_ALOGE("Window::init -- androidNativeWindow is null! Set it before calling init().");
			return false;
		}
		eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if (eglDisplay == EGL_NO_DISPLAY) { ANTIBOX_ALOGE("eglGetDisplay failed"); return false; }

		EGLint major, minor;
		if (!eglInitialize(eglDisplay, &major, &minor)) { ANTIBOX_ALOGE("eglInitialize failed"); return false; }

		// !! [ANDROID] request OpenGL ES 3.0 config
		const EGLint attribs[] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
			EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
			EGL_BLUE_SIZE,  8,
			EGL_GREEN_SIZE, 8,
			EGL_RED_SIZE,   8,
			EGL_DEPTH_SIZE, 16,
			EGL_NONE
		};
		EGLConfig config;
		EGLint numConfigs;
		if (!eglChooseConfig(eglDisplay, attribs, &config, 1, &numConfigs) || numConfigs == 0) {
			ANTIBOX_ALOGE("eglChooseConfig failed"); return false;
		}
		eglSurface = eglCreateWindowSurface(eglDisplay, config, androidNativeWindow, nullptr);
		if (eglSurface == EGL_NO_SURFACE) { ANTIBOX_ALOGE("eglCreateWindowSurface failed"); return false; }

		const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
		eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, ctxAttribs);
		if (eglContext == EGL_NO_CONTEXT) { ANTIBOX_ALOGE("eglCreateContext failed"); return false; }

		if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
			ANTIBOX_ALOGE("eglMakeCurrent failed"); return false;
		}
		EGLint surfW = 0, surfH = 0;
		eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH,  &surfW);
		eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &surfH);
		width  = (unsigned int)surfW;
		height = (unsigned int)surfH;
		glViewport(0, 0, width, height);
		Engine::Instance().GetRenderManager().SetClearColor(props.cc);
		mFramebuffer = std::make_shared<Framebuffer>(props.w, props.h);
		mFramebuffer->SetClearColor(props.cc);
		mImguiWindow.Create(props.imguiProps);
		showFramebuffer = props.framebuffer_display;
		ANTIBOX_ALOGI("EGL ready: %dx%d ES %d.%d", surfW, surfH, major, minor);
		return true;
#endif
		// !! [ANDROID] end init() platform split
	}

	void Window::UpdateCC(glm::vec4 props)
	{
		Engine::Instance().GetRenderManager().SetClearColor(props);
		mFramebuffer->SetClearColor(props); // props.cc
	}

	glm::ivec2 Window::GetSize() {
		// !! [ANDROID] EGL surface dimensions replace GLFW window size query
#ifndef __ANDROID__
		int w, h;
		glfwGetWindowSize(win, &w, &h);
		return glm::ivec2(w, h);
#else
		EGLint w = 0, h = 0;
		if (eglDisplay != EGL_NO_DISPLAY && eglSurface != EGL_NO_SURFACE) {
			eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH,  &w);
			eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &h);
		}
		return glm::ivec2(w, h);
#endif
		// !! [ANDROID] end GetSize
	}
}
