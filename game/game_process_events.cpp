/*
File:   game_process_events.cpp
Author: Taylor Robbins
Date:   07\21\2024
Description: 
	** Holds functions that help us process ProcmonEvent_t structures coming from the Open Procmon driver
	** and store them safely into data structures that we can access from the main thread
*/

#define PROCMON_HEAP_SIZE          Megabytes(1)
#define PROCMON_MAX_EVENTS_QUEUED  1000 //events/frame

void InitProcmonInfo(ProcmonInfo_t* procmon)
{
	plat->CreateMutex(&procmon->mutex);
	u8* heapSpacePntr = AllocArray(mainHeap, u8, PROCMON_HEAP_SIZE);
	NotNull(heapSpacePntr);
	InitMemArena_FixedHeap(&procmon->heap, PROCMON_HEAP_SIZE, heapSpacePntr);
	CreateBktArray(&procmon->processes, &procmon->heap, sizeof(ProcmonProcess_t));
	CreateVarArray(&procmon->itemEvents, &procmon->heap, sizeof(ProcmonItemEvent_t));
}

ProcmonProcess_t* FindProcessById(ProcmonInfo_t* procmon, u64 processId)
{
	for (u64 pIndex = 0; pIndex < procmon->processes.length; pIndex++)
	{
		ProcmonProcess_t* process = BktArrayGetHard(&procmon->processes, ProcmonProcess_t, pIndex);
		if (process->id == processId) { return process; }
	}
	return nullptr;
}

void HandleProcmonEvent(ProcmonInfo_t* procmon, ProcmonEvent_t* event)
{
	MemArena_t* scratch = GetScratchArena();
	MyStr_t filePathNormalized = AllocString(scratch, &event->path);
	StrReplaceInPlace(filePathNormalized, "\\", "/");
	if (StrEndsWith(filePathNormalized, "/")) { filePathNormalized.length--; }
	
	if (plat->LockMutex(&procmon->mutex, MUTEX_LOCK_INFINITE))
	{
		if (!IsEmptyStr(procmon->eventPathFilter) && StrStartsWith(filePathNormalized, procmon->eventPathFilter))
		{
			// PrintLine_D("%s (0x%X): \"%.*s\" \"%.*s\" %.*s", GetProcmonEventTypeStr(event->type), event->type, StrPrint(event->processImagePath), StrPrint(event->path), StrPrint(event->detail));
			ProcmonProcess_t* process = FindProcessById(procmon, event->processId);
			if (process == nullptr)
			{
				PrintLine_D("Discovered Process %llu \"%.*s\"", event->processId, StrPrint(event->processName));
				process = BktArrayAdd(&procmon->processes, ProcmonProcess_t);
				NotNull(process);
				ClearPointer(process);
				process->id = event->processId;
				process->name = AllocString(&procmon->heap, &event->processName);
				process->exePath = AllocString(&procmon->heap, &event->processImagePath);
			}
			
			NotNull(process);
			process->numEvents++;
			process->eventsSinceLastFrame++;
			
			if (IsAppStateInitialized(AppState_Main))
			{
				MyStr_t itemName = StrSubstring(&filePathNormalized, procmon->eventPathFilter.length);
				if (StrStartsWith(itemName, "/")) { itemName.chars++; itemName.length--; }
				u64 firstSlashCharIndex = 0;
				if (FindNextCharInStr(itemName, 0, "/", &firstSlashCharIndex)) { itemName.length = firstSlashCharIndex; }
				
				if (procmon->itemEvents.length < PROCMON_MAX_EVENTS_QUEUED)
				{
					ProcmonItemEvent_t* newEvent = VarArrayAdd(&procmon->itemEvents, ProcmonItemEvent_t);
					NotNull(newEvent);
					ClearPointer(newEvent);
					newEvent->itemName = AllocString(&procmon->heap, &itemName);
					newEvent->process = process;
				}
			}
			
		}
		plat->UnlockMutex(&procmon->mutex);
	}
	
	FreeScratchArena(scratch);
}
