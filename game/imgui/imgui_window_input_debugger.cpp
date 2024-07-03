/*
File:   imgui_window_input_debugger.cpp
Author: Taylor Robbins
Date:   03\09\2024
Description: 
	** An Imgui window that helps us debug common problems with Input by displaying
	** and graphing a bunch of info in an inspectable way. For example input handling
	** over recent frames for particular keys\buttons is displayed as a nice row of
	** boxes with styling to indicate whether that key was handled on that frame
*/

struct InputDebuggerKeyHistory_t
{
	Key_t key;
	u64 historyIndex;
	bool handledHistory[64];
	bool downHistory[64];
};

struct InputDebuggerWindowState_t
{
	bool initialized;
	bool isPaused;
	char inputBuffer[64];
	InputDebuggerKeyHistory_t histories[8];
};

// +==============================+
// |  InputDebuggerWindowRender   |
// +==============================+
PIGGEN_RegisterFunc(InputDebuggerWindowRender)
// void InputDebuggerWindowRender(struct PigRegisteredImguiWindow_t* window)
IMGUI_WINDOW_RENDER_FUNC_DEF(InputDebuggerWindowRender)
{
	NotNull(window->contextPntr);
	Assert(window->contextAllocSize == sizeof(InputDebuggerWindowState_t));
	InputDebuggerWindowState_t* state = (InputDebuggerWindowState_t*)window->contextPntr;
	
	if (ImGui::Begin("Input Debugger", &window->isOpen))
	{
		if (!state->initialized)
		{
			state->initialized = true;
		}
		
		ImGui::SeparatorText("Key Handling");
		
		if (ImGui::Button(state->isPaused ? ">" : "||"))
		{
			state->isPaused = !state->isPaused;
		}
		ImGui::SameLine(100);
		ImGui::InputTextWithHint("###Key", "Enter key name", &state->inputBuffer[0], ArrayCount(state->inputBuffer));
		ImGui::SameLine();
		if (ImGui::Button("Track"))
		{
			MyStr_t inputStr = NewStr(&state->inputBuffer[0]);
			Key_t key = Key_None;
			if (TryParseEnum(inputStr, &key, Key_NumKeys, GetKeyStr))
			{
				bool foundEmptySlot = false;
				u64 emptySlotIndex = 0;
				bool foundFilledSlot = false;
				u64 filledSlotIndex = 0;
				bool foundMatchingKey = false;
				for (u64 hIndex = 0; hIndex < ArrayCount(state->histories); hIndex++)
				{
					InputDebuggerKeyHistory_t* history = &state->histories[hIndex];
					if (history->key == Key_None && !foundEmptySlot)
					{
						emptySlotIndex = hIndex;
						foundEmptySlot = true;
					}
					else if (history->key == key)
					{
						foundMatchingKey = true;
						break;
					}
					else if (!foundFilledSlot)
					{
						foundFilledSlot = true;
						filledSlotIndex = hIndex;
					}
				}
				if (foundMatchingKey) { NotifyWrite_E("That key is already being tracked"); }
				else if (!foundEmptySlot) { NotifyPrint_E("We can only track %llu keys at a time. Please remove a key tracker before adding another one", ArrayCount(state->histories)); }
				else
				{
					InputDebuggerKeyHistory_t* history = &state->histories[emptySlotIndex];
					ClearPointer(history);
					history->key = key;
					if (foundFilledSlot) { history->historyIndex = state->histories[filledSlotIndex].historyIndex; }
					state->inputBuffer[0] = '\0';
				}
			}
			else { NotifyPrint_E("There is no key named \"%.*s\"", StrPrint(inputStr)); }
		}
		
		for (u64 keyIndex = 0; keyIndex < ArrayCount(state->histories); keyIndex++)
		{
			InputDebuggerKeyHistory_t* history = &state->histories[keyIndex];
			ImGui::PushID((void*)keyIndex);
			if (history->key != Key_None)
			{
				if (!state->isPaused)
				{
					history->handledHistory[history->historyIndex] = (pig->prevKeyHandled[history->key].pressHandled || pig->prevKeyHandled[history->key].releaseHandled);
					history->downHistory[history->historyIndex] = pigIn->keyStates[history->key].wasDown;
					history->historyIndex = (history->historyIndex + 1) % ArrayCount(history->handledHistory);
				}
				
				if (ImGui::Button("X"))
				{
					history->key = Key_None;
				}
				ImGui::SameLine();
				ImGui::Text("%s:", GetKeyStr(history->key));
				for (u64 hIndex = 0; hIndex < ArrayCount(history->handledHistory); hIndex++)
				{
					bool handledThisFrame = history->handledHistory[hIndex];
					bool downThisFrame = history->downHistory[hIndex];
					bool isCurrentFrame = history->historyIndex == hIndex;
					Color_t buttonColor = isCurrentFrame ? MonokaiWhite : (handledThisFrame ? (downThisFrame ? MonokaiGreen : MonokaiYellow) : (downThisFrame ? MonokaiBlue : MonokaiGray2));
					ImGui::SameLine((r32)(100 + hIndex*(6+1)));
					ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a));
					ImGui::Button("###frame", ImVec2(6, 25));
					ImGui::SetItemTooltip("State: %s\nHandled: %s", downThisFrame ? "Down" : "Up", handledThisFrame ? "Yes" : "No");
					ImGui::PopStyleColor();
				}
			}
			ImGui::PopID();
		}
	}
	ImGui::End();
}
