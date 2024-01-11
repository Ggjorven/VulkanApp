#include "vcpch.h"

#define IMGUI_IMPL_VULKAN
#include <backends/imgui_impl_vulkan.cpp>

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <backends/imgui_impl_opengl3.cpp>

// Note(Jorben): GLFW is our windowing library
#include <backends/imgui_impl_glfw.cpp>