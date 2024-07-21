/*
File:   game_process_events.h
Author: Taylor Robbins
Date:   07\21\2024
*/

#ifndef _GAME_PROCESS_EVENTS_H
#define _GAME_PROCESS_EVENTS_H

struct ProcmonProcess_t
{
	u64 id;
	MyStr_t name;
	MyStr_t exePath;
	bool triedToLoadIcon;
	FileIcon_t* icon;
	u64 numEvents;
	u64 eventsSinceLastFrame;
};

struct ProcmonItemEvent_t
{
	MyStr_t itemName;
	ProcmonProcess_t* process;
};

struct ProcmonInfo_t
{
	PlatMutex_t mutex;
	MemArena_t heap;
	MyStr_t eventPathFilter;
	BktArray_t processes; //ProcmonProcess_t
	VarArray_t itemEvents; //ProcmonItemEvent_t
};

#endif //  _GAME_PROCESS_EVENTS_H
