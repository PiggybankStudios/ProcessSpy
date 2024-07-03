/*
File:   imgui_window_memory_debugger.cpp
Author: Taylor Robbins
Date:   03\14\2024
Description: 
	** This window takes advantage of the mainHeapDebug arena which is only enabled with PIG_MAIN_ARENA_DEBUG
	** We can use this to look at the metadata for any allocation in the mainHeap
	** When we have memory leaks we can peruse the remaining allocations and identify ones that seem inappropriate
*/

static int MemoryDebuggerAllocIndex = 0; //TODO: Move me to a imgui window state struct

// +==============================+
// |  MemoryDebuggerWindowRender  |
// +==============================+
PIGGEN_RegisterFunc(MemoryDebuggerWindowRender)
// void MemoryDebuggerWindowRender(struct PigRegisteredImguiWindow_t* window)
IMGUI_WINDOW_RENDER_FUNC_DEF(MemoryDebuggerWindowRender)
{
	MemArena_t* scratch = GetScratchArena();
	
	if (ImGui::Begin("Memory Debugger", &window->isOpen))
	{
		#if PIG_MAIN_ARENA_DEBUG
		
		ImGui::SeparatorText(PrintInArena(scratch, "mainHeap has %llu allocation%s", mainHeap->numAllocations, Plural(mainHeap->numAllocations, "s")));
		ImGui::InputInt("Alloc Index", &MemoryDebuggerAllocIndex);
		
		void* allocPntr = nullptr;
		u64 allocSize = 0;
		u64 allocPageIndex = 0;
		MemArenaAllocInfo_t* allocInfo = nullptr;
		
		{
			HeapPageHeader_t* prevPageHeader = nullptr;
			HeapPageHeader_t* pageHeader = (HeapPageHeader_t*)mainHeap->headerPntr;
			u64 pageIndex = 0;
			u64 allocIndex = 0;
			while (pageHeader != nullptr)
			{
				u8* pageBase = (u8*)(pageHeader + 1);
				u64 allocOffset = 0;
				u8* allocBytePntr = pageBase;
				u64 sectionIndex = 0;
				HeapAllocPrefix_t* prevPrefixPntr = nullptr;
				while (allocOffset < pageHeader->size)
				{
					HeapAllocPrefix_t* prefixPntr = (HeapAllocPrefix_t*)allocBytePntr;
					u8* afterPrefixPntr = (allocBytePntr + sizeof(HeapAllocPrefix_t));
					bool isSectionFilled = IsAllocPrefixFilled(prefixPntr->size);
					u64 sectionSize = UnpackAllocPrefixSize(prefixPntr->size);
					AssertMsg(sectionSize >= sizeof(HeapAllocPrefix_t), "Found an allocation header that claimed to be smaller than the header itself in Paged Heap");
					u64 afterPrefixSize = sectionSize - sizeof(HeapAllocPrefix_t);
					
					if (isSectionFilled)
					{
						if (allocIndex == MemoryDebuggerAllocIndex)
						{
							allocPntr = afterPrefixPntr;
							allocSize = afterPrefixSize;
							allocPageIndex = pageIndex;
							allocInfo = FindAllocInfoFor(&pig->mainHeapDebug, allocPntr);
							break;
						}
						allocIndex++;
					}
					
					prevPrefixPntr = prefixPntr;
					allocOffset += sectionSize;
					allocBytePntr += sectionSize;
					sectionIndex++;
				}
				if (allocPntr != nullptr) { break; }
				
				prevPageHeader = pageHeader;
				pageHeader = pageHeader->next;
				pageIndex++;
			}
		}
		
		if (allocPntr != nullptr)
		{
			// const char* FormatBytesNt(u64 numBytes, MemArena_t* memArena)
			ImGui::SeparatorText("Allocation Info");
			ImGui::Text("Pntr: %p", allocPntr);
			ImGui::Text("Size: %llu (%s)", allocSize, FormatBytesNt(allocSize, scratch));
			ImGui::Text("Page: %llu/%llu", allocPageIndex+1, mainHeap->numPages);
			if (allocInfo != nullptr)
			{
				ImGui::Text("File: \"%s\"", allocInfo->filePath);
				ImGui::Text("Line: %llu", allocInfo->lineNumber);
				ImGui::Text("Function: %s", allocInfo->funcName);
			}
			else
			{
				ImGui::Text("No allocInfo");
			}
		}
		else
		{
			ImGui::Text("Couldn't find allocation %d", MemoryDebuggerAllocIndex);
		}
		
		#else
		ImGui::Text("Enable PIG_MAIN_ARENA_DEBUG #define to make this window usable");
		#endif
	}
	ImGui::End();
	
	FreeScratchArena(scratch);
}
