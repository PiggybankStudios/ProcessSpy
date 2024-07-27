/*
File:   game_main.h
Author: Taylor Robbins
Date:   01\31\2022
Description: 
	** Holds all the .h includes for the game portion of the engine DLL
*/

#ifndef _GAME_MAIN_H
#define _GAME_MAIN_H

#include "game_version.h"
#include "game_defines.h"

#include "game_tasks.h"
#include "file_icon_cache.h"
#include "game_process_events.h"

#include "app_state_list.h"
#include "main_menu/main_state.h"

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
struct AppStateStructs_t
{
	u64 mainAppStateSize;
	MainAppState_t* mainAppStatePntr;
};

struct AppGlobals_t
{
	#if PROCMON_SUPPORTED
	ProcmonInfo_t procmon;
	#endif
};

static AppGlobals_t*   gl   = nullptr;
static MainAppState_t* main = nullptr;

//game_debug_commands.cpp
ResourcePool_t* GameGetCurrentResourcePool();
void GameAddDebugVarsToExpContext(ExpContext_t* context);
void GameAddDebugCommandsToExpContext(ExpContext_t* context);

#endif //  _GAME_MAIN_H
