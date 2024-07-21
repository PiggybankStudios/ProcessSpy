/*
File:   main_state.h
Author: Taylor Robbins
Date:   07\02\2024
*/

#ifndef _MAIN_STATE_H
#define _MAIN_STATE_H

#include "ui/ui_scroll_view.h"

struct ItemProcessRef_t
{
	u64 numEvents;
	u64 numEventsSinceLastFrame;
	u64 lastAccessTime;
	ProcmonProcess_t* process;
};

struct FolderFileItem_t
{
	bool isFolder;
	MyStr_t name;
	MyStr_t path;
	u64 iconId;
	FileIcon_t* fileIcon;
	
	bool isHovered;
	bool isSelected;
	u64 lastClickTime;
	u64 quickClickCount;
	
	VarArray_t processRefs; //ItemProcessRef_t
	
	TextMeasure_t nameMeasure;
	rec mainRec;
	//These are all relative to mainRec.topLeft
	rec hitRec; //used for mouse hit testing, spans full width
	rec nameRec;
	rec iconRec;
};

struct MainAppState_t
{
	bool initialized;
	
	MyStr_t currentPath;
	u64 numSelectedItems;
	i64 primarySelectedItemIndex;
	bool softScrollToSelected;
	bool scrollToSelected;
	bool instantScrollToSelected;
	VarArray_t items; //FolderFileItem_t
	FileIconCache_t iconCache;
	
	r32 sidebarWidth;
	r32 userSetSidebarWidth;
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
	rec restartWithAdminBtnRec;
};

#endif //  _MAIN_STATE_H
