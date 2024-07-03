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

#include "app_state_list.h"
#include "main_menu/main_menu_state.h"

// +--------------------------------------------------------------+
// |                           Globals                            |
// +--------------------------------------------------------------+
struct AppStateStructs_t
{
	u64 mainMenuAppStateSize;
	MainMenuAppState_t* mainMenuAppStatePntr;
};

struct AppGlobals_t
{
	bool placeholder; //TODO: Remove me!
};

static AppGlobals_t*          gl        = nullptr;
static MainMenuAppState_t*    mmenu     = nullptr;

//game_debug_commands.cpp
ResourcePool_t* GameGetCurrentResourcePool();
void GameAddDebugVarsToExpContext(ExpContext_t* context);
void GameAddDebugCommandsToExpContext(ExpContext_t* context);

#endif //  _GAME_MAIN_H
