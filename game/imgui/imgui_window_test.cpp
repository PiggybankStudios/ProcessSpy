/*
File:   imgui_window_test.cpp
Author: Taylor Robbins
Date:   03\09\2024
Description: 
	** A simple ImGui window that's a dedicated spot for placing temporary test code
	** For example, any tools that needed to be created to debug a particular problem
	** can be made here, used for their purpose, and then deleted.
	** It's also a great place to add graphs of information
*/

struct TestImguiWindowState_t
{
	bool initialized;
};

// +==============================+
// |     TestImguiWindowFree      |
// +==============================+
PIGGEN_RegisterFunc(TestImguiWindowFree)
// void TestImguiWindowFree(struct PigRegisteredImguiWindow_t* window)
IMGUI_WINDOW_FREE_FUNC_DEF(TestImguiWindowFree)
{
	UNUSED(window);
	//TODO: Add any freeing code you need here
}

// +==============================+
// |    TestImguiWindowRender     |
// +==============================+
PIGGEN_RegisterFunc(TestImguiWindowRender)
// void TestImguiWindowRender(struct PigRegisteredImguiWindow_t* window)
IMGUI_WINDOW_RENDER_FUNC_DEF(TestImguiWindowRender)
{
	NotNull(window->contextPntr);
	Assert(window->contextAllocSize == sizeof(TestImguiWindowState_t));
	TestImguiWindowState_t* state = (TestImguiWindowState_t*)window->contextPntr;
	
	if (ImGui::Begin("Test Window", &window->isOpen))
	{
		if (!state->initialized)
		{
			//TODO: Add any initialization code you need here
			
			state->initialized = true;
		}
		
		//TODO: Add any ImGui code you want here
		ImGui::Text("Nothing to test right now...");
	}
	ImGui::End();
}

