/*
File:   imgui_window_resources.cpp
Author: Taylor Robbins
Date:   03\09\2024
Description: 
	** None 
*/

// +==============================+
// |    ResourcesWindowRender     |
// +==============================+
PIGGEN_RegisterFunc(ResourcesWindowRender)
// void ResourcesWindowRender(struct PigRegisteredImguiWindow_t* window)
IMGUI_WINDOW_RENDER_FUNC_DEF(ResourcesWindowRender)
{
	if (ImGui::Begin("Resources", &window->isOpen))
	{
		if (ImGui::Button("Load All"))
		{
			Pig_LoadAllResources();
		}
		
		for (u64 tIndex = (ResourceType_None + 1); tIndex < ResourceType_NumTypes; tIndex++)
		{
			ResourceType_t resourceType = (ResourceType_t)tIndex;
			u64 numOfType = GetNumResourcesOfType(resourceType);
			u64 numLoaded = 0;
			for (u64 rIndex = 0; rIndex < numOfType; rIndex++)
			{
				if (GetResourceStatus(resourceType, rIndex)->state == ResourceState_Loaded) { numLoaded++; }
			}
			
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			ImGui::PushID(GetResourceTypeStr(resourceType));
			if (ImGui::TreeNode(GetResourceTypeStr(resourceType), "%s (%llu/%llu)", GetResourceTypeStr(resourceType), numLoaded, numOfType))
			{
				for (u64 rIndex = 0; rIndex < numOfType; rIndex++)
				{
					ImGui::PushID((int)rIndex);
					const char* resourcePath = GetPathOrNameForResource(resourceType, rIndex);
					MyStr_t resourceName = GetFileNamePart(NewStr(resourcePath));
					ResourceStatus_t* status = GetResourceStatus(resourceType, rIndex);
					NotNull(status);
					Color_t textColor = MonokaiGray2;
					if (status->state == ResourceState_Error) { textColor = MonokaiMagenta; }
					else if (status->state == ResourceState_Warning) { textColor = MonokaiYellow; }
					else if (status->state == ResourceState_Loaded) { textColor = MonokaiWhite; }
					
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(textColor.r, textColor.g, textColor.b, textColor.a));
					ImGui::Text("%s[%llu]: \"%.*s\"", GetResourceTypeStr(resourceType), rIndex, StrPrint(resourceName));
					ImGui::PopStyleColor();
					ImGui::SetItemTooltip("%s\n%s", resourcePath, GetResourceStateStr(status->state));
					if (status->state != ResourceState_Loaded)
					{
						if (ImGui::BeginPopupContextItem("ResourceContextMenu"))
						{
							if (ImGui::Button("Load"))
							{
								Pig_LoadResource(resourceType, rIndex);
							}
							ImGui::EndPopup();
						}
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
	ImGui::End();
}
