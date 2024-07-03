/*
File:   imgui_window_opengl_extensions.cpp
Author: Taylor Robbins
Date:   04\05\2024
Description: 
	** None 
*/

struct OpenGlExtensionsWindowState_t
{
	bool initialized;
	char searchBuffer[256];
};

// +==============================+
// | OpenGlExtensionsWindowRender |
// +==============================+
PIGGEN_RegisterFunc(OpenGlExtensionsWindowRender)
// void OpenGlExtensionsWindowRender(struct PigRegisteredImguiWindow_t* window)
IMGUI_WINDOW_RENDER_FUNC_DEF(OpenGlExtensionsWindowRender)
{
	MemArena_t* scratch = GetScratchArena();
	NotNull(window->contextPntr);
	Assert(window->contextAllocSize == sizeof(OpenGlExtensionsWindowState_t));
	OpenGlExtensionsWindowState_t* state = (OpenGlExtensionsWindowState_t*)window->contextPntr;
	
	if (ImGui::Begin("OpenGL Extensions", &window->isOpen))
	{
		if (!state->initialized)
		{
			state->initialized = true;
		}
		
		ImGui::InputText("Search", &state->searchBuffer[0], ArrayCount(state->searchBuffer));
		MyStr_t searchStr = NewStr(&state->searchBuffer[0]);
		
		u64 numKnownTotalExtensions = 0;
		u64 numKnownSupportedExtensions = 0;
		u64 numCoreAdoptedTotalExtensions = 0;
		u64 numCoreAdoptedSupportedExtensions = 0;
		for (u64 eIndex = 1; eIndex < OpenGlExtension_NumExtensions; eIndex++)
		{
			OpenGlExtension_t extension = (OpenGlExtension_t)eIndex;
			MyStr_t extensionNameStr = NewStr(GetOpenGlExtensionStr(extension));
			if (IsEmptyStr(searchStr) || FindSubstring(extensionNameStr, searchStr, nullptr, true))
			{
				numKnownTotalExtensions++;
				bool isCoreAdopted = (GetOpenGlExtensionCoreAdoptionVersion(extension).major != 0);
				if (isCoreAdopted) { numCoreAdoptedTotalExtensions++; }
				if (platInfo->opengl.extensionSupported[eIndex])
				{
					if (isCoreAdopted) { numCoreAdoptedSupportedExtensions++; }
					numKnownSupportedExtensions++;
				}
			}
		}
		
		ImGui::Separator();
		if (ImGui::TreeNode("KnownExtensionsNode", "%llu/%llu Known Extensions", numKnownSupportedExtensions, numKnownTotalExtensions))
		{
			for (u64 eIndex = 1; eIndex < OpenGlExtension_NumExtensions; eIndex++)
			{
				OpenGlExtension_t extension = (OpenGlExtension_t)eIndex;
				MyStr_t extensionNameStr = NewStr(GetOpenGlExtensionStr(extension));
				if (IsEmptyStr(searchStr) || FindSubstring(extensionNameStr, searchStr, nullptr, true))
				{
					bool isSupported = platInfo->opengl.extensionSupported[eIndex];
					ImGui::TextColored(ToImVec4(isSupported ? MonokaiGreen : MonokaiGray1), "[%llu] %s", eIndex, GetOpenGlExtensionStr(extension));
				}
			}
			ImGui::TreePop();
		}
		
		ImGui::Separator();
		if (ImGui::TreeNode("CoreAdoptedExtensionsNode", "%llu/%llu Core Adopted Extensions", numCoreAdoptedSupportedExtensions, numCoreAdoptedTotalExtensions))
		{
			bool isVersionOpen = false;
			Version_t currentVersion = NewVersion(0, 0);
			for (u64 eIndex = 1; eIndex < OpenGlExtension_NumExtensions; eIndex++)
			{
				OpenGlExtension_t extension = (OpenGlExtension_t)eIndex;
				MyStr_t extensionNameStr = NewStr(GetOpenGlExtensionStr(extension));
				if (IsEmptyStr(searchStr) || FindSubstring(extensionNameStr, searchStr, nullptr, true))
				{
					bool heavilyModified = false;
					Version_t extensionVersion = GetOpenGlExtensionCoreAdoptionVersion(extension, &heavilyModified);
					if (extensionVersion.major != 0)
					{
						bool isSupported = platInfo->opengl.extensionSupported[eIndex];
						
						if (extensionVersion.major != currentVersion.major || extensionVersion.minor != currentVersion.minor)
						{
							if (isVersionOpen) { ImGui::TreePop(); }
							isVersionOpen = ImGui::TreeNode(PrintInArena(scratch, "OpenGL %u.%u", extensionVersion.major, extensionVersion.minor));
							currentVersion = extensionVersion;
						}
						
						if (isVersionOpen)
						{
							ImGui::TextColored(ToImVec4(isSupported ? MonokaiGreen : MonokaiGray1), "[%llu] %s%s", eIndex, GetOpenGlExtensionStr(extension), heavilyModified ? " (Heavily Modified)" : "");
						}
					}
				}
			}
			if (isVersionOpen) { ImGui::TreePop(); }
			ImGui::TreePop();
		}
		
		ImGui::Separator();
		if (ImGui::TreeNode("Unknown Supported Extensions"))
		{
			u64 numUnknownExtensions = 0;
			GLint numExtensions = 0;
			glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions); AssertNoOpenGlError();
			for (u32 sIndex = 0; sIndex < (u32)numExtensions; sIndex++)
			{
				const GLubyte* extensionStr = glGetStringi(GL_EXTENSIONS, sIndex);
				OpenGlExtension_t extension = OpenGlExtension_NumExtensions;
				if (!TryParseEnum(NewStr((const char*)extensionStr), &extension, OpenGlExtension_NumExtensions, GetOpenGlExtensionStr)) { numUnknownExtensions++; }
			}
			
			if (ImGui::TreeNode("UnknownExtensionsNode", "%llu/%llu supported extensions are unknown", numUnknownExtensions, (u64)numExtensions))
			{
				for (u32 sIndex = 0; sIndex < (u32)numExtensions; sIndex++)
				{
					const GLubyte* extensionStr = glGetStringi(GL_EXTENSIONS, sIndex);
					OpenGlExtension_t extension = OpenGlExtension_NumExtensions;
					if (!TryParseEnum(NewStr((const char*)extensionStr), &extension, OpenGlExtension_NumExtensions, GetOpenGlExtensionStr))
					{
						ImGui::TextColored(ToImVec4(MonokaiGreen), "[%u] %s", sIndex, (const char*)extensionStr);
					}
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
	
	FreeScratchArena(scratch);
}