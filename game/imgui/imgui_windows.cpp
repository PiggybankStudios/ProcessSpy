/*
File:   game_imgui_windows.cpp
Author: Taylor Robbins
Date:   03\05\2024
Description: 
	** None 
*/

// +==============================+
// |    ImguiDemoWindowRender     |
// +==============================+
PIGGEN_RegisterFunc(ImguiDemoWindowRender)
// void ImguiDemoWindowRender(struct PigRegisteredImguiWindow_t* window)
IMGUI_WINDOW_RENDER_FUNC_DEF(ImguiDemoWindowRender)
{
	ImGui::ShowDemoWindow(&window->isOpen);
}

// +==============================+
// |     MemArenaWindowRender     |
// +==============================+
PIGGEN_RegisterFunc(MemArenaWindowRender)
// void MemArenaWindowRender(struct PigRegisteredImguiWindow_t* window)
IMGUI_WINDOW_RENDER_FUNC_DEF(MemArenaWindowRender)
{
	// ImGui::SetNextWindowBgAlpha(pig->memGraph.imguiOverlayMode ? 0.0f : 0.75f);
	if (ImGui::Begin("Memory Arenas", &window->isOpen, (pig->memGraph.imguiOverlayMode ? (ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground) : 0)))
	{
		RenderPigMemGraph_Imgui(&pig->memGraph);
	}
	ImGui::End();
}

#include "imgui/imgui_window_test.cpp"
#include "imgui/imgui_window_resources.cpp"
#include "imgui/imgui_window_bindings.cpp"
#include "imgui/imgui_window_input_debugger.cpp"
#include "imgui/imgui_window_memory_debugger.cpp"
#include "imgui/imgui_window_opengl_extensions.cpp"

void GameRegisterImguiWindows()
{
	PigRegisterImguiWindow("Imgui Demo", MakeFunc(ImguiDemoWindowRender));
	PigRegisterImguiWindowWithStruct("Test Window", MakeFunc(TestImguiWindowRender), MakeFunc(TestImguiWindowFree), TestImguiWindowState_t);
	PigRegisterImguiWindow("Memory Arenas", MakeFunc(MemArenaWindowRender));
	PigRegisterImguiWindow("Resources", MakeFunc(ResourcesWindowRender));
	PigRegisterImguiWindowWithStruct("Bindings", MakeFunc(BindingsWindowRender), Func_Nullptr, BindingsWindowState_t);
	PigRegisterImguiWindowWithStruct("Input Debugger", MakeFunc(InputDebuggerWindowRender), Func_Nullptr, InputDebuggerWindowState_t);
	PigRegisterImguiWindow("Memory Debugger", MakeFunc(MemoryDebuggerWindowRender));
	PigRegisterImguiWindowWithStruct("OpenGL Extensions", MakeFunc(OpenGlExtensionsWindowRender), Func_Nullptr, OpenGlExtensionsWindowState_t);
}
