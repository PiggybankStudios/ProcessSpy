/*
File:   app_state_list.cpp
Author: Taylor Robbins
Date:   01\31\2022
Description: 
	** This file holds the routing functions that allow us to call the various
	** Update, Render, etc. functions on any arbitrary AppState_t
NOTE: This file gets included by pig_main.cpp
*/

void StartAppState(AppState_t appState, AppState_t oldAppState, bool initialize)
{
	switch (appState)
	{
		case AppState_Main: StartMainAppState(oldAppState, initialize); break;
		default: DebugAssert(false); break;
	}
}

void StopAppState(AppState_t appState, AppState_t newAppState, bool deinitialize, bool shuttingDown)
{
	switch (appState)
	{
		case AppState_Main: StopMainAppState(newAppState, deinitialize, shuttingDown); break;
		default: DebugAssert(false); break;
	}
}

void UpdateAppState(AppState_t appState)
{
	switch (appState)
	{
		case AppState_Main: UpdateMainAppState(); break;
		default: DebugAssert(false); break;
	}
}

void RenderAppState(AppState_t appState, FrameBuffer_t* renderBuffer, bool bottomLayer)
{
	switch (appState)
	{
		case AppState_Main: RenderMainAppState(renderBuffer, bottomLayer); break;
		default: DebugAssert(false); break;
	}
}
