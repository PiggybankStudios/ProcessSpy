/*
File:   game_main.cpp
Author: Taylor Robbins
Date:   01\31\2022
Description: 
	** Holds all the .cpp includes for the game portion of the engine DLL
*/

#include "app_func_defs.h"
#include "game_startup_options.cpp"

#include "file_icon_cache.cpp"

#include "main_menu/main_state.cpp"
#include "app_state_list.cpp"

#include "game_settings.cpp"
#include "game_tasks.cpp"
#include "game_process_events.cpp"
#include "game_debug_commands.cpp"
#include "imgui/imgui_windows.cpp"

// +--------------------------------------------------------------+
// |                          Functions                           |
// +--------------------------------------------------------------+
void GameInitAppGlobals(AppGlobals_t* globals) //pre-declared in pig_func_defs.h
{
	NotNull(globals);
	ClearPointer(globals);
	InitProcmonInfo(&globals->procmon);
}

//NOTE: GameLoadSettings is in game_settings.cpp

void GameAllocateAppStateStructs(AppStateStructs_t* appStateStructs) //pre-declared in pig_func_defs.h
{
	NotNull(appStateStructs);
	ClearPointer(appStateStructs);
	
	#define ALLOCATE_APP_STATE_STRUCT_CODE(structType, sizeVar, pntrVar) do \
	{                                                                       \
		appStateStructs->sizeVar = sizeof(structType);                      \
		appStateStructs->pntrVar = AllocStruct(fixedHeap, structType);      \
		NotNull(appStateStructs->pntrVar);                                  \
		ClearPointer(appStateStructs->pntrVar);                             \
	} while(0)
	
	ALLOCATE_APP_STATE_STRUCT_CODE(MainAppState_t, mainAppStateSize, mainAppStatePntr);
	
	#undef ALLOCATE_APP_STATE_STRUCT_CODE
}

void GameUpdateGlobals() //pre-declared in pig_func_defs.h
{
	gl   = ((pig != nullptr) ? &pig->appGlobals                      : nullptr);
	main = ((pig != nullptr) ? pig->appStateStructs.mainAppStatePntr : nullptr);
}

void GameLoadDebugBindings(PigDebugBindings_t* bindings) //pre-declared in pig_func_defs.h
{
	MyStr_t builtinBindingsFilePath = NewStr(GAME_DBG_BINDINGS_FILE_NAME);
	if (plat->DoesFileExist(builtinBindingsFilePath, nullptr))
	{
		PigLoadDebugBindingsFullService(bindings, builtinBindingsFilePath, false);
	}
	else
	{
		PrintLine_D("No local debug bindings file found at \"%.*s\"", StrPrint(builtinBindingsFilePath));
	}
	
	MyStr_t userBindingsFilePath = PigGetDebugBindingsFilePath(TempArena, NewStr(PROJECT_NAME_SAFE), NewStr(GAME_DBG_BINDINGS_FILE_NAME));
	if (userBindingsFilePath.length > 0)
	{
		PigLoadDebugBindingsFullService(bindings, userBindingsFilePath, true);
	}
	else
	{
		WriteLine_E("Failed to get special folder path to load debug bindings from!");
	}
}

//This happens after all the above functions but before the first AppState gets initalized
void GameGeneralInit() //pre-declared in pig_func_defs.h
{
	WriteLine_N("=====GameGeneralInit Start=====");
	
	GameParseSettings();
	
	AppState_t firstAppState = GAME_INITIAL_APP_STATE;
	PrintLine_N("First AppState is %s", GetAppStateStr(firstAppState));
	PushAppState(firstAppState);
	Pig_HandleAppStateChanges(true);
	
	GameRegisterImguiWindows();
	
	WriteLine_N("=====GameGeneralInit Finish=====");
}

void GameGeneralUpdate() //pre-declared in pig_func_defs.h
{
	// +==============================+
	// |    F11 Toggles Fullscreen    |
	// +==============================+
	if (KeyPressed(Key_F11))
	{
		HandleKeyExtended(Key_F11);
		WriteLine_Ix(DbgFlag_Inverted, "User pressed fullscreen toggle button...");
		ToggleFullscreen();
	}
}

void GameHandleReload() //pre-declared in pig_func_defs.h
{
	#define RELOAD_APP_STATE_STRUCT_CODE(structType, sizeVar, pntrVar) do                                                                                                                    \
	{                                                                                                                                                                                        \
		if (pig->appStateStructs.sizeVar != sizeof(structType))                                                                                                                              \
		{                                                                                                                                                                                    \
			u64 oldSize = pig->appStateStructs.sizeVar;                                                                                                                                      \
			u64 newSize = sizeof(structType);                                                                                                                                                \
			NotifyPrint_I(#structType " resized %llu -> %llu (%s%llu)", oldSize, newSize, (oldSize > newSize ? "-" : "+"), (oldSize > newSize ? (oldSize - newSize) : (newSize - oldSize))); \
			structType* newState = AllocStruct(fixedHeap, structType);                                                                                                                       \
			NotNull(newState);                                                                                                                                                               \
			MyMemCopy(newState, pig->appStateStructs.pntrVar, MinU64(oldSize, newSize));                                                                                                     \
			FreeMem(fixedHeap, pig->appStateStructs.pntrVar, oldSize);                                                                                                                       \
			pig->appStateStructs.sizeVar = newSize;                                                                                                                                          \
			pig->appStateStructs.pntrVar = newState;                                                                                                                                         \
		}                                                                                                                                                                                    \
	} while(0)
	
	RELOAD_APP_STATE_STRUCT_CODE(MainAppState_t, mainAppStateSize, mainAppStatePntr);
	
	#undef RELOAD_APP_STATE_STRUCT_CODE
}

void GamePinResources() //pre-declared in pig_func_defs.h
{
	PinAllResourcesOfType(ResourceType_Font);
	
	PinResource(&pig->resources.textures->alpha);
	PinResource(&pig->resources.textures->defaultPink);
	PinResource(&pig->resources.textures->defaultBlue);
	PinResource(&pig->resources.textures->defaultGreen);
	PinResource(&pig->resources.textures->defaultPurple);
	PinResource(&pig->resources.textures->defaultRed);
	PinResource(&pig->resources.textures->defaultYellow);
	
	PinResource(&pig->resources.sheets->vectorIcons64);
	PinResource(&pig->resources.sheets->controllerBtns);
	
	PinResource(&pig->resources.shaders->main2D);
	PinResource(&pig->resources.shaders->main2DArray);
	PinResource(&pig->resources.shaders->main3D);
	PinResource(&pig->resources.shaders->main3DArray);
	PinResource(&pig->resources.shaders->gradient2D);
	PinResource(&pig->resources.shaders->roundedCorners);
	PinResource(&pig->resources.shaders->pieChart);
	PinResource(&pig->resources.shaders->bezier3);
	PinResource(&pig->resources.shaders->bezier4);
	PinResource(&pig->resources.shaders->ellipseArc);
	
	PinResource(&pig->resources.sounds->notification);
}

//AppStates will all be stopped/deinitialized after this
void GamePrepareForClose() //pre-declared in pig_func_defs.h
{
	Pig_ChangeWindow(platInfo->mainWindow);
	GameSaveSettings();
}

//NOTE: We are on a non-main thread here, be careful what we do
void GameHandleProcmonEvent(ProcmonEvent_t* event)
{
	HandleProcmonEvent(&gl->procmon, event);
}

// +--------------------------------------------------------------+
// |                    Function Table Filling                    |
// +--------------------------------------------------------------+
#include "game_func_table_filling.cpp"
