#pragma once
#ifndef WINDOW_CLASS_H
#define WINDOW_CLASS_H

#include <memory>
#include <glm/glm.hpp>
#include <string>
#include "antibox/core/imguiwindow.h"

// !! [ANDROID] EGL/GLES replace desktop OpenGL + GLFW on Android
#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/native_window.h>
#endif
// !! [ANDROID] end Android GL includes


namespace antibox {

	class Framebuffer;
	//Window Properties is how you edit width, height, title, and background color as well as editing ImGui Properties.
	struct WindowProperties
	{
		ImguiWindowProperties imguiProps; //Titlebar only, Docking, Viewport, vector of Font Paths, vector of Font Names, FontSize (float)
		std::string title; // Title of the window.
		int w, h; //Width and height of the window.
		int vsync; //1 means enabled, 0 means disabled
		glm::vec4 cc; //Background Clear Color.
		bool framebuffer_display;

		WindowProperties();
	};

	class Window
	{
	public:

		Window(const unsigned int width, const unsigned int height, const char* windowName);

		bool init(const WindowProperties& props);
		void UpdateCC(glm::vec4 props);
		void BeginRender();
		void EndRender();
		void GetEvents();
		glm::ivec2 GetSize();

		// !! [ANDROID] GLFWwindow type doesn't exist on Android -- guard desktop-only accessor
#ifndef __ANDROID__
		inline GLFWwindow* glfwin() { return Window::win; }
#endif
		// !! [ANDROID] end glfwin guard
		inline ImguiWindow imwin() { return Window::mImguiWindow; }
		inline Framebuffer* GetFramebuffer() { return mFramebuffer.get(); }
		inline void UseFramebuffer(bool tf) { showFramebuffer = tf; }
		void GetScreenSize(int& w, int& h);
		void ToggleConsoleVisible() { engineConsoleVisible = !engineConsoleVisible; }

	private:
		bool showFramebuffer = false;
		bool engineConsoleVisible = false;
		// !! [ANDROID] GLFWwindow* only exists on desktop; Android uses EGL handles instead
#ifndef __ANDROID__
		GLFWwindow* win;
#else
		// !! [ANDROID] EGL display/surface/context replace the GLFW window
		EGLDisplay eglDisplay = EGL_NO_DISPLAY;
		EGLSurface eglSurface = EGL_NO_SURFACE;
		EGLContext eglContext = EGL_NO_CONTEXT;
#endif
		// !! [ANDROID] end window handle block
		// !! [ANDROID] ANativeWindow* passed in from android_main before init() is called
#ifdef __ANDROID__
	public:
		ANativeWindow* androidNativeWindow = nullptr;
	private:
#endif
		// !! [ANDROID] end ANativeWindow block
		unsigned int width;
		unsigned int height;
		const char* windowName;
		ImguiWindow mImguiWindow;

		std::shared_ptr<Framebuffer> mFramebuffer;
	};
}
#endif // !WINDOW_CLASS_H
