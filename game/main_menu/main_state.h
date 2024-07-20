/*
File:   main_state.h
Author: Taylor Robbins
Date:   07\02\2024
*/

#ifndef _MAIN_STATE_H
#define _MAIN_STATE_H

#include "ui/ui_scroll_view.h"

struct FolderFileItem_t
{
	bool isFolder;
	MyStr_t name;
	
	TextMeasure_t nameMeasure;
	rec mainRec;
	//These are both relative to mainRec.topLeft
	rec nameRec;
	rec iconRec;
};

struct MainAppState_t
{
	bool initialized;
	
	MyStr_t currentPath;
	VarArray_t items; //FolderFileItem_t
	
	r32 sidebarWidth;
	bool resizingSidebar;
	r32 resizingSidebarGrabOffset;
	
	rec sidebarRec;
	rec topbarRec;
	rec dividerRec;
	rec viewportRec;
	ScrollView_t viewportScroll;
	rec pathTextboxRec;
	rec backBtnRec;
	rec forwardBtnRec;
	rec upBtnRec;
};

#endif //  _MAIN_STATE_H
