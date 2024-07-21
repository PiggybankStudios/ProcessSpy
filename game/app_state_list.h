/*
File:   app_state_list.h
Author: Taylor Robbins
Date:   01\31\2022
NOTE: This file gets included by pig_main.cpp
*/

#ifndef _APP_STATE_LIST_H
#define _APP_STATE_LIST_H

enum AppState_t
{
	AppState_None = 0,
	AppState_Main,
	AppState_NumStates,
};

const char* GetAppStateStr(AppState_t appState)
{
	switch (appState)
	{
		case AppState_None: return "None";
		case AppState_Main: return "Main";
		default: return "Unknown";
	}
}

bool DoesAppStateCoverBelow(AppState_t appState)
{
#pragma warning(push)
#pragma warning(disable: 4065) //switch statement contains 'default' but no 'case' labels
	switch (appState)
	{
		default: return true;
	}
#pragma warning(pop)
}

#endif //  _APP_STATE_LIST_H
