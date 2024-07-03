/*
File:   imgui_window_bindings.cpp
Author: Taylor Robbins
Date:   03\09\2024
Description: 
	** A Imgui window that shows all the debug bindings that are currently active,
	** and allows the user to delete, add, or edit bindings
*/

struct BindingsWindowState_t
{
	char searchBuffer[256];
	char bindingStrBuffer[256];
	char commandStrBuffer[256];
};

// returns true when the item was removed
bool RenderDebugBindingEntryInImgui(BindingsWindowState_t* state, MemArena_t* scratch, PigDebugBindings_t* bindings, PigDebugBindingsEntry_t* binding)
{
	MyStr_t bindingStr = GetDebugBindingStr(scratch, binding);
	Color_t bindingColor = MonokaiWhite;
	Color_t commandColor = MonokaiLightGray;
	MyStr_t searchStr = NewStr(&state->searchBuffer[0]);
	
	if (IsEmptyStr(searchStr) || FindSubstring(bindingStr, searchStr, nullptr, true) || FindSubstring(binding->commandStr, searchStr, nullptr, true))
	{
		ImGui::PushID(bindingStr.chars);
		
		r32 availableWidth = ImGui::GetContentRegionAvail().x;
		
		if (ImGui::Button("X"))
		{
			PigFreeDebugBindingsEntry(binding, bindings->allocArena);
			VarArrayRemoveByPntrTyped(&bindings->entries, binding, PigDebugBindingsEntry_t);
			ImGui::PopID();
			return true;
		}
		
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(bindingColor.r, bindingColor.g, bindingColor.b, bindingColor.a));
		ImGui::PushTextWrapPos(200);
		ImGui::Text("%.*s:", StrPrint(bindingStr));
		ImGui::PopTextWrapPos();
		ImGui::PopStyleColor();
		
		ImGui::SameLine(200);
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(commandColor.r, commandColor.g, commandColor.b, commandColor.a));
		ImGui::Text("%.*s", StrPrint(binding->commandStr));
		ImGui::PopStyleColor();
		
		if (binding->isUserBinding)
		{
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(MonokaiGray2.r, MonokaiGray2.g, MonokaiGray2.b, MonokaiGray2.a));
			ImGui::Text("(User)");
			ImGui::PopStyleColor();
		}
		
		ImGui::SameLine(availableWidth - 30);
		ImGui::SetNextItemWidth(30);
		if (ImGui::Button("Edit"))
		{
			MyStr_t bindingStrTruncated = bindingStr;
			bindingStrTruncated.length = MinU64(bindingStrTruncated.length, ArrayCount(state->bindingStrBuffer)-1);
			MyMemCopy(&state->bindingStrBuffer[0], bindingStrTruncated.chars, bindingStrTruncated.length);
			state->bindingStrBuffer[bindingStrTruncated.length] = '\0';
			MyStr_t commandStrTruncated = binding->commandStr;
			commandStrTruncated.length = MinU64(commandStrTruncated.length, ArrayCount(state->commandStrBuffer)-1);
			MyMemCopy(&state->commandStrBuffer[0], commandStrTruncated.chars, commandStrTruncated.length);
			state->commandStrBuffer[commandStrTruncated.length] = '\0';
		}
		
		ImGui::PopID();
	}
	
	return false;
}

// +==============================+
// |     BindingsWindowRender     |
// +==============================+
PIGGEN_RegisterFunc(BindingsWindowRender)
// void BindingsWindowRender(struct PigRegisteredImguiWindow_t* window)
IMGUI_WINDOW_RENDER_FUNC_DEF(BindingsWindowRender)
{
	MemArena_t* scratch = GetScratchArena();
	Assert(window->contextAllocSize == sizeof(BindingsWindowState_t));
	BindingsWindowState_t* state = (BindingsWindowState_t*)window->contextPntr;
	NotNull(state);
	
	if (ImGui::Begin("Bindings", &window->isOpen))
	{
		if (ImGui::Button("Edit (Builtin)"))
		{
			MyStr_t bindingsFilePath = NewStr(GAME_DBG_BINDINGS_FILE_NAME);
			if (!plat->ShowFile(bindingsFilePath)) { NotifyPrint_E("Failed to open builtin bindings file at \"%.*s\"", StrPrint(bindingsFilePath)); }
		}
		ImGui::SameLine();
		if (ImGui::Button("Edit (User)"))
		{
			MyStr_t bindingsFilePath = PigGetDebugBindingsFilePath(scratch, NewStr(PROJECT_NAME_SAFE), NewStr(GAME_DBG_BINDINGS_FILE_NAME));
			if (!plat->DoesFileExist(bindingsFilePath, nullptr))
			{
				MyStr_t newBindingFileContents = NewStr("# [Pig Bindings]\n// Put your personal debug bindings here");
				plat->WriteEntireFile(bindingsFilePath, newBindingFileContents.chars, newBindingFileContents.length);
			}
			if (!plat->ShowFile(bindingsFilePath)) { NotifyPrint_E("Failed to open user bindings file at \"%.*s\"", StrPrint(bindingsFilePath)); }
		}
		ImGui::SameLine();
		if (ImGui::Button("Reload"))
		{
			PigClearDebugBindings(&pig->debugBindings);
			GameLoadDebugBindings(&pig->debugBindings);
		}
		
		ImGui::InputTextWithHint("###Filter", "Filter", &state->searchBuffer[0], ArrayCount(state->searchBuffer));
		
		ImGui::PushID("AddBinding");
		ImGui::SeparatorText("Add Binding:");
		if (ImGui::Button("X"))
		{
			state->bindingStrBuffer[0] = '\0';
			state->commandStrBuffer[0] = '\0';
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		ImGui::InputTextWithHint("###Binding", "Binding", &state->bindingStrBuffer[0], ArrayCount(state->bindingStrBuffer));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		ImGui::InputTextWithHint("###Command", "Command", &state->commandStrBuffer[0], ArrayCount(state->commandStrBuffer));
		ImGui::SameLine();
		if (ImGui::Button("Add"))
		{
			PigDebugBindingsEntry_t newBinding = {};
			newBinding.isUserBinding = true;
			if (PigTryDeserBindingStr(NewStr(&state->bindingStrBuffer[0]), &newBinding))
			{
				PigDebugBindingsEntry_t* newEntry = nullptr;
				switch (newBinding.type)
				{
					case PigDebugBindingType_Keyboard:   newEntry = PigAddDebugBindingsEntryKey(&pig->sessionDebugBindings,        true, newBinding.modifiers, newBinding.key, NewStr(&state->commandStrBuffer[0])); break;
					case PigDebugBindingType_Mouse:      newEntry = PigAddDebugBindingsEntryMouse(&pig->sessionDebugBindings,      true, newBinding.mouseBtn, NewStr(&state->commandStrBuffer[0])); break;
					case PigDebugBindingType_Controller: newEntry = PigAddDebugBindingsEntryController(&pig->sessionDebugBindings, true, newBinding.controllerBtn, NewStr(&state->commandStrBuffer[0])); break;
					default: DebugAssert(false); break;
				}
				NotNull(newEntry);
				state->bindingStrBuffer[0] = '\0';
				state->commandStrBuffer[0] = '\0';
			}
			else
			{
				NotifyPrint_E("Invalid binding string: \"%s\"", &state->bindingStrBuffer[0]);
			}
		}
		ImGui::PopID();
		
		ImGui::PushID("SessionBindings");
		ImGui::SeparatorText("Session Bindings:");
		VarArrayLoop(&pig->sessionDebugBindings.entries, bIndex)
		{
			VarArrayLoopGet(PigDebugBindingsEntry_t, binding, &pig->sessionDebugBindings.entries, bIndex);
			if (RenderDebugBindingEntryInImgui(state, scratch, &pig->sessionDebugBindings, binding)) { bIndex--; }
		}
		ImGui::PopID();
		
		ImGui::PushID("GlobalBindings");
		ImGui::SeparatorText("Global Bindings:");
		VarArrayLoop(&pig->debugBindings.entries, bIndex)
		{
			VarArrayLoopGet(PigDebugBindingsEntry_t, binding, &pig->debugBindings.entries, bIndex);
			if (RenderDebugBindingEntryInImgui(state, scratch, &pig->debugBindings, binding)) { bIndex--; }
		}
		ImGui::PopID();
	}
	ImGui::End();
	FreeScratchArena(scratch);
}
