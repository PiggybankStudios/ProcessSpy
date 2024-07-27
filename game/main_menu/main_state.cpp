/*
File:   main_state.cpp
Author: Taylor Robbins
Date:   07\02\2024
Description: 
	** We don't use app states in this tool, so this is the main and only app state
*/

#include "ui/ui_scroll_view.cpp"
#include "ui/ui_divider.cpp"

#include "main_menu/main_helpers.cpp"

#define RESTART_IN_ADMIN_MODE_MESSAGE "The program needs to be restarted in Administrator Mode so that we can enable the Open Procmon system driver that tracks file access events."

#define PANEL_MARGIN        1   //px
#define PANEL_DIVIDER_WIDTH 5   //px
#define MIN_SIDEBAR_WIDTH   100 //px
#define MIN_VIEWPORT_WIDTH  250  //px
#define TOPBAR_HEIGHT       40  //px
#define NAV_BTN_SIZE        NewVec2(32, 32) //px
#define PATH_PIECE_MARGIN   4 //px

#define ICON_ITEM_NAME_MARGIN          10 //px
#define INTER_ITEM_MARGIN              2 //px
#define ITEM_INNER_PADDING_LEFT_RIGHT  15 //px
#define ITEM_INNER_PADDING_TOP_BOTTOM  3 //px
#define INTER_BTN_MARGIN               4

void FreeFolderFileItem(MemArena_t* allocArena, FolderFileItem_t* item)
{
	NotNull2(allocArena, item);
	FreeVarArray(&item->processRefs);
	FreeString(allocArena, &item->name);
	FreeString(allocArena, &item->path);
	ClearPointer(item);
}

rec MainLayoutItems(r32 maxWidth)
{
	rec contentRec = Rec_Zero;
	RcBindFont(&pig->resources.fonts->files, SelectDefaultFontFace());
	VarArrayLoop(&main->items, iIndex)
	{
		VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex);
		item->iconRec.size = ToVec2(pig->resources.sheets->fileIcons.frameSize);
		item->iconRec.x = ITEM_INNER_PADDING_LEFT_RIGHT;
		item->nameMeasure = RcMeasureText(item->name);
		item->nameRec.size = item->nameMeasure.size;
		item->nameRec.x = item->iconRec.x + item->iconRec.width + ICON_ITEM_NAME_MARGIN;
		item->mainRec.height = MaxR32(item->iconRec.height, item->nameRec.height) + ITEM_INNER_PADDING_TOP_BOTTOM*2;
		item->mainRec.width = item->nameRec.x + item->nameRec.width + ITEM_INNER_PADDING_LEFT_RIGHT;
		// if (item->mainRec.width > maxWidth)
		// {
		// 	item->mainRec.width = maxWidth;
		// 	item->nameRec.width = (item->mainRec.width - ITEM_INNER_PADDING_LEFT_RIGHT) - item->nameRec.x;
		// }
		item->iconRec.y = item->mainRec.height/2 - item->iconRec.height/2;
		item->nameRec.y = item->mainRec.height/2 - item->nameRec.height/2;
		RecAlign(&item->iconRec);
		RecAlign(&item->nameRec);
		item->mainRec.x = 0;
		item->mainRec.y = contentRec.y + contentRec.height + (iIndex > 0 ? INTER_ITEM_MARGIN : 0);
		RecAlign(&item->mainRec);
		item->hitRec = NewRec(0, 0, maxWidth - item->mainRec.x, item->mainRec.height + INTER_ITEM_MARGIN);
		RecAlign(&item->hitRec);
		if (contentRec == Rec_Zero) { contentRec = item->mainRec; }
		else { contentRec = RecBoth(contentRec, item->mainRec); }
	}
	return contentRec;
}

// rec MainContentSizeFunc(ScrollView_t* scrollView)
SCROLL_VIEW_CONTENT_SIZE_DEFINITION(MainContentSizeFunc)
{
	rec result = MainLayoutItems(scrollView->usableRec.width);
	return result;
}

void UpdateFolderFileItemsAtCurrentPath()
{
	VarArrayLoop(&main->items, iIndex)
	{
		VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex);
		FreeFolderFileItem(mainHeap, item);
	}
	VarArrayClear(&main->items);
	
	//TODO: We should probably try and maintain the selection through a refresh if we are actually in the same folder as we were before
	main->primarySelectedItemIndex = -1;
	main->numSelectedItems = 0;
	
	MemArena_t* scratch = GetScratchArena();
	
	PlatFileEnumerator_t folderEnumerator = plat->StartEnumeratingFiles(main->currentPath, false, true);
	u64 numFolders = 0;
	MyStr_t folderName = MyStr_Empty;
	while (plat->EnumerateFiles(&folderEnumerator, &folderName, scratch, false))
	{
		FolderFileItem_t* newFolderItem = VarArrayAdd(&main->items, FolderFileItem_t);
		NotNull(newFolderItem);
		ClearPointer(newFolderItem);
		newFolderItem->isFolder = true;
		newFolderItem->name = AllocString(mainHeap, &folderName);
		newFolderItem->path = PrintInArenaStr(mainHeap, "%.*s/%.*s", StrPrint(main->currentPath), StrPrint(folderName));
		newFolderItem->iconId = 0;
		CreateVarArray(&newFolderItem->processRefs, mainHeap, sizeof(ItemProcessRef_t));
		numFolders++;
	}
	
	PlatFileEnumerator_t fileEnumerator = plat->StartEnumeratingFiles(main->currentPath, true, false);
	MyStr_t fileName = MyStr_Empty;
	u64 numFiles = 0;
	while (plat->EnumerateFiles(&fileEnumerator, &fileName, scratch, false))
	{
		FolderFileItem_t* newFileItem = VarArrayAdd(&main->items, FolderFileItem_t);
		NotNull(newFileItem);
		ClearPointer(newFileItem);
		newFileItem->isFolder = false;
		newFileItem->name = AllocString(mainHeap, &fileName);
		newFileItem->path = PrintInArenaStr(mainHeap, "%.*s/%.*s", StrPrint(main->currentPath), StrPrint(fileName));
		newFileItem->iconId = plat->GetFileIconId(newFileItem->path);
		newFileItem->fileIcon = FindFileIconById(&main->iconCache, newFileItem->iconId, newFileItem->path);
		CreateVarArray(&newFileItem->processRefs, mainHeap, sizeof(ItemProcessRef_t));
		numFiles++;
	}
	
	PrintLine_I("Found %llu folder%s and %llu file%s in \"%.*s\"", numFolders, Plural(numFolders, "s"), numFiles, Plural(numFiles, "s"), StrPrint(main->currentPath));
	
	FreeScratchArena(scratch);
}

void MoveToPath(MyStr_t newPath)
{
	while (StrEndsWith(newPath, "/")) { newPath.length--; }
	if (StrEquals(main->currentPath, newPath)) { return; }
	//NOTE: Alloc has to happen BEFORE freeing of currentPath, because newPath might be dependent on currentPath's allocation
	MyStr_t newPathAllocated = AllocString(mainHeap, &newPath);
	MyStr_t oldPath = main->currentPath;
	main->currentPath = newPathAllocated;
	UpdateFolderFileItemsAtCurrentPath();
	if (oldPath.length > newPath.length && StrStartsWith(oldPath, newPath))
	{
		MyStr_t exitedFolderStr = StrSubstring(&oldPath, newPath.length);
		while (StrStartsWith(exitedFolderStr, "/")) { exitedFolderStr.chars++; exitedFolderStr.length--; }
		while (StrEndsWith(exitedFolderStr, "/")) { exitedFolderStr.length--; }
		u64 firstSlashCharIndex = 0;
		if (FindNextCharInStr(exitedFolderStr, 0, "/", &firstSlashCharIndex))
		{
			exitedFolderStr = StrSubstring(&exitedFolderStr, 0, firstSlashCharIndex);
		}
		bool foundExitedFolder = false;
		u64 exitedFolderItemIndex = 0;
		VarArrayLoop(&main->items, iIndex)
		{
			VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex);
			if (item->isFolder && StrEqualsIgnoreCase(item->name, exitedFolderStr))
			{
				exitedFolderItemIndex = iIndex;
				foundExitedFolder = true;
				break;
			}
		}
		if (foundExitedFolder)
		{
			VarArrayLoop(&main->items, iIndex)
			{
				VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex);
				item->isSelected = (iIndex == exitedFolderItemIndex);
				// if (iIndex == exitedFolderItemIndex) { PrintLine_I("Scrolling to item[%llu] \"%.*s\"", exitedFolderItemIndex, StrPrint(item->name)); }
			}
			main->numSelectedItems = 1;
			main->primarySelectedItemIndex = (i64)exitedFolderItemIndex;
			main->instantScrollToSelected = true;
		}
	}
	FreeString(mainHeap, &oldPath);
	
	#if PROCMON_SUPPORTED
	if (plat->LockMutex(&gl->procmon.mutex, MUTEX_LOCK_INFINITE))
	{
		FreeString(mainHeap, &gl->procmon.eventPathFilter);
		gl->procmon.eventPathFilter = AllocString(mainHeap, &main->currentPath);
		plat->UnlockMutex(&gl->procmon.mutex);
	}
	#endif
}

// +--------------------------------------------------------------+
// |                       Access Resources                       |
// +--------------------------------------------------------------+
void MainAppStateAccessResources()
{
	AccessResource(&pig->resources.fonts->files);
	AccessResource(&pig->resources.sheets->fileIcons);
	AccessResource(&pig->resources.sheets->buttonIcons);
}

// +--------------------------------------------------------------+
// |                            Start                             |
// +--------------------------------------------------------------+
void StartMainAppState(AppState_t oldAppState, bool initialize)
{
	UNUSED(oldAppState);
	NotNull(main);
	MemArena_t* scratch = GetScratchArena();
	
	if (initialize)
	{
		ClearPointer(main);
		
		CreateVarArray(&main->items, mainHeap, sizeof(FolderFileItem_t));
		InitUiDivider(&main->sidebarDivider, true, false, ScreenSize.width*0.25f, ScreenRec);
		main->sidebarDivider.minLeftSizePx = MIN_SIDEBAR_WIDTH;
		main->sidebarDivider.minRightSizePx = MIN_VIEWPORT_WIDTH;
		InitUiScrollView(&main->viewportScroll);
		InitFileIconCache(&main->iconCache, mainHeap);
		
		MyStr_t homePath = plat->GetSpecialFolderPath(SpecialFolder_Home, NewStr(PROJECT_NAME), scratch);
		MoveToPath(homePath);
		
		main->initialized = true;
	}
	
	MainAppStateAccessResources();
	
	FreeScratchArena(scratch);
}

// +--------------------------------------------------------------+
// |                             Stop                             |
// +--------------------------------------------------------------+
void StopMainAppState(AppState_t newAppState, bool deinitialize, bool shuttingDown)
{
	UNUSED(newAppState);
	NotNull(main);
	
	if (deinitialize && !shuttingDown)
	{
		FreeString(mainHeap, &main->currentPath);
		VarArrayLoop(&main->items, iIndex)
		{
			VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex);
			FreeFolderFileItem(mainHeap, item);
		}
		FreeVarArray(&main->items);
		FreeFileIconCache(&main->iconCache);
		ClearPointer(main);
	}
}

// +--------------------------------------------------------------+
// |                   Layout and CaptureMouse                    |
// +--------------------------------------------------------------+
void LayoutMainAppState()
{
	RcBindFont(&pig->resources.fonts->files, SelectDefaultFontFace());
	
	main->sidebarDivider.mainRec = ScreenRec;
	LayoutUiDivider(&main->sidebarDivider);
	rec sidebarRec = main->sidebarDivider.leftRec + main->sidebarDivider.mainRec.topLeft;
	main->viewportRec = main->sidebarDivider.rightRec + main->sidebarDivider.mainRec.topLeft;
	
	main->topbarRec = main->viewportRec;
	main->topbarRec.height = TOPBAR_HEIGHT;
	RecAlign(&main->topbarRec);
	main->viewportRec.y = main->topbarRec.y + main->topbarRec.height;
	main->viewportRec.height = (ScreenSize.height - PANEL_MARGIN) - main->viewportRec.y;
	RecAlign(&main->viewportRec);
	
	main->backBtnRec.size = NAV_BTN_SIZE;
	main->backBtnRec.y = main->topbarRec.y + main->topbarRec.height/2 - main->backBtnRec.height/2;
	main->backBtnRec.x = main->topbarRec.x + INTER_BTN_MARGIN;
	RecAlign(&main->backBtnRec);
	
	main->forwardBtnRec.size = NAV_BTN_SIZE;
	main->forwardBtnRec.y = main->topbarRec.y + main->topbarRec.height/2 - main->forwardBtnRec.height/2;
	main->forwardBtnRec.x = main->backBtnRec.x + main->backBtnRec.width + INTER_BTN_MARGIN;
	RecAlign(&main->forwardBtnRec);
	
	main->upBtnRec.size = NAV_BTN_SIZE;
	main->upBtnRec.y = main->topbarRec.y + main->topbarRec.height/2 - main->upBtnRec.height/2;
	main->upBtnRec.x = main->forwardBtnRec.x + main->forwardBtnRec.width + INTER_BTN_MARGIN;
	RecAlign(&main->upBtnRec);
	
	main->pathTextboxRec.x = main->upBtnRec.x + main->upBtnRec.width + INTER_BTN_MARGIN;
	main->pathTextboxRec.width = main->topbarRec.x + main->topbarRec.width - main->pathTextboxRec.x;
	main->pathTextboxRec.height = RcGetLineHeight() + 4*2;
	main->pathTextboxRec.y = main->topbarRec.y + main->topbarRec.height/2 - main->pathTextboxRec.height/2;
	RecAlign(&main->pathTextboxRec);
	
	if (!platInfo->wasRunInAdministratorMode)
	{
		v2 adminTextPos = NewVec2(sidebarRec.x + 5, sidebarRec.y + 5 + RcGetMaxAscend());
		Vec2Align(&adminTextPos);
		r32 adminTextMaxWidth = sidebarRec.x + sidebarRec.width - adminTextPos.x;
		TextMeasure_t adminTextMeasure = RcMeasureText(NewStr(RESTART_IN_ADMIN_MODE_MESSAGE), adminTextMaxWidth);
		
		main->restartWithAdminBtnRec.width = sidebarRec.width - 5*2;
		main->restartWithAdminBtnRec.height = 200;
		main->restartWithAdminBtnRec.topLeft = sidebarRec.topLeft + sidebarRec.size/2 - main->restartWithAdminBtnRec.size/2;
		if (main->restartWithAdminBtnRec.y < adminTextPos.y + adminTextMeasure.size.height - adminTextMeasure.offset.y)
		{
			main->restartWithAdminBtnRec.y = adminTextPos.y + adminTextMeasure.size.height - adminTextMeasure.offset.y;
		}
		RecAlign(&main->restartWithAdminBtnRec);
	}
	
	MoveUiScrollView(&main->viewportScroll, main->viewportRec, MainContentSizeFunc);
}

void CaptureMouseMainAppState()
{
	UiDividerCaptureMouse(&main->sidebarDivider);
	MouseHitRecNamed(RecInflate(main->backBtnRec, INTER_BTN_MARGIN/2, 0), "BackBtn");
	MouseHitRecNamed(RecInflate(main->forwardBtnRec, INTER_BTN_MARGIN/2, 0), "ForwardBtn");
	MouseHitRecNamed(RecInflate(main->upBtnRec, INTER_BTN_MARGIN/2, 0), "UpBtn");
	UiScrollViewCaptureMouse(&main->viewportScroll);
	if (!platInfo->wasRunInAdministratorMode)
	{
		MouseHitRecNamed(main->restartWithAdminBtnRec, "SidebarRestartWithAdminBtn");
	}
	if (IsMouseInsideRec(main->viewportRec))
	{
		v2 itemsOffset = main->viewportRec.topLeft - main->viewportScroll.scroll;
		VarArrayLoop(&main->items, iIndex)
		{
			VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex);
			rec hitRec = item->hitRec + item->mainRec.topLeft + itemsOffset;
			MouseHitRecPrint(hitRec, "ViewportItem%llu", iIndex);
		}
	}
	MouseHitRecNamed(RecInflate(main->viewportRec, PANEL_MARGIN, PANEL_MARGIN), "Viewport");
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateMainAppState()
{
	NotNull(main);
	MemArena_t* scratch = GetScratchArena();
	MainAppStateAccessResources();
	LayoutMainAppState();
	CaptureMouseMainAppState();
	
	// +==============================+
	// |         F5 Refreshes         |
	// +==============================+
	if (KeyPressed(Key_F5))
	{
		HandleKeyExtended(Key_F5);
		UpdateFolderFileItemsAtCurrentPath();
	}
	
	UpdateUiDivider(&main->sidebarDivider);
	
	// +==============================+
	// |        Handle Buttons        |
	// +==============================+
	if (IsMouseOverNamed("BackBtn"))
	{
		// pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			NotifyWrite_W("This button isn't implemented yet!"); //TODO: Implement me!
		}
	}
	if (IsMouseOverNamed("ForwardBtn"))
	{
		// pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			NotifyWrite_W("This button isn't implemented yet!"); //TODO: Implement me!
		}
	}
	if (IsMouseOverNamed("UpBtn"))
	{
		// pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			MyStr_t newPath = main->currentPath;
			while (StrEndsWith(newPath, "/")) { newPath.length--; }
			u64 lastSlashIndex = 0;
			for (u64 cIndex = newPath.length; cIndex > 0; cIndex--)
			{
				if (newPath.chars[cIndex-1] == '/') { lastSlashIndex = cIndex-1; break; }
			}
			if (lastSlashIndex > 0)
			{
				newPath = StrSubstring(&newPath, 0, lastSlashIndex);
				MoveToPath(newPath);
			}
		}
	}
	if (IsMouseOverNamed("SidebarRestartWithAdminBtn"))
	{
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			plat->RestartWithAdminPrivileges();
		}
	}
	
	UpdateUiScrollView(&main->viewportScroll, IsMouseOverNamedPartial("Viewport"));
	
	// +==============================+
	// |      Scroll to Selected      |
	// +==============================+
	//TODO: Somehow this is getting run BEFORE we think we have any scrollable area available. We either need to wait 1 more frame, or we need to change order of operations so we are fully aware of scrollable area here
	if (main->scrollToSelected || main->instantScrollToSelected || main->softScrollToSelected)
	{
		// PrintLine_D("Handling scroll to selected request: (%f, %f) %llu items", main->viewportScroll.scrollMax.x, main->viewportScroll.scrollMax.y, main->items.length);
		if (main->primarySelectedItemIndex >= 0)
		{
			FolderFileItem_t* selectedItem = VarArrayGetHard(&main->items, (u64)main->primarySelectedItemIndex, FolderFileItem_t);
			bool shouldScrollX = true;
			bool shouldScrollY = true;
			if (!main->scrollToSelected && !main->instantScrollToSelected)
			{
				if (((selectedItem->mainRec.x + selectedItem->mainRec.width) - main->viewportScroll.scroll.x) > main->viewportScroll.usableRec.width) { shouldScrollX = true; }
				else if ((selectedItem->mainRec.x - main->viewportScroll.scroll.x) < 0) { shouldScrollX = true; }
				else { shouldScrollX = false; }
				
				if (((selectedItem->mainRec.y + selectedItem->mainRec.height) - main->viewportScroll.scroll.y) > main->viewportScroll.usableRec.height) { shouldScrollY = true; }
				else if ((selectedItem->mainRec.y - main->viewportScroll.scroll.y) < 0) { shouldScrollY = true; }
				else { shouldScrollY = false; }
			}
			if (shouldScrollX) { main->viewportScroll.scrollGoto.x = selectedItem->mainRec.x + selectedItem->mainRec.width/2 - main->viewportScroll.usableRec.width/2; }
			if (shouldScrollY) { main->viewportScroll.scrollGoto.y = selectedItem->mainRec.y + selectedItem->mainRec.height/2 - main->viewportScroll.usableRec.height/2; }
			if (main->instantScrollToSelected) { main->viewportScroll.scroll = main->viewportScroll.scrollGoto; }
		}
		
		main->scrollToSelected = false;
		main->instantScrollToSelected = false;
		main->softScrollToSelected = false;
	}
	
	// +==============================+
	// |      Ctrl+A Selects All      |
	// +==============================+
	if (KeyPressed(Key_A) && KeyDownRaw(Key_Control))
	{
		HandleKeyExtended(Key_A);
		VarArrayLoop(&main->items, iIndex) { VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex); item->isSelected = true; }
		main->numSelectedItems = main->items.length;
		if (main->primarySelectedItemIndex < 0 && main->items.length > 0) { main->primarySelectedItemIndex = 0; }
	}
	
	// +==============================+
	// | Arrow Up/Down Move Selection |
	// +==============================+
	if (KeyPressedRepeating(Key_Up, 400, 30))
	{
		HandleKey(Key_Up);
		if (main->items.length > 0)
		{
			main->primarySelectedItemIndex = (main->primarySelectedItemIndex > 0) ? (main->primarySelectedItemIndex-1) : (i64)(main->items.length-1);
			VarArrayLoop(&main->items, iIndex) { VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex); item->isSelected = (main->primarySelectedItemIndex == (i64)iIndex); }
			main->numSelectedItems = 1;
			main->softScrollToSelected = true;
		}
	}
	if (KeyPressedRepeating(Key_Down, 400, 30))
	{
		HandleKey(Key_Down);
		if (main->items.length > 0)
		{
			main->primarySelectedItemIndex = (main->primarySelectedItemIndex >= 0) ? ((main->primarySelectedItemIndex+1) % main->items.length) : 0;
			VarArrayLoop(&main->items, iIndex) { VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex); item->isSelected = (main->primarySelectedItemIndex == (i64)iIndex); }
			main->numSelectedItems = 1;
			main->softScrollToSelected = true;
		}
	}
	
	// +==============================+
	// |  Consume Item Access Events  |
	// +==============================+
	#if PROCMON_SUPPORTED
	if (plat->LockMutex(&gl->procmon.mutex, MUTEX_LOCK_INFINITE))
	{
		VarArrayLoop(&gl->procmon.itemEvents, eIndex)
		{
			VarArrayLoopGet(ProcmonItemEvent_t, event, &gl->procmon.itemEvents, eIndex);
			VarArrayLoop(&main->items, iIndex)
			{
				VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex);
				if (StrEqualsIgnoreCase(item->name, event->itemName))
				{
					ItemProcessRef_t* processRef = FindItemProcessRef(item, event->process);
					if (processRef == nullptr)
					{
						processRef = VarArrayAdd(&item->processRefs, ItemProcessRef_t);
						NotNull(processRef);
						ClearPointer(processRef);
						processRef->process = event->process;
					}
					processRef->numEvents++;
					processRef->numEventsSinceLastFrame++;
					processRef->lastAccessTime = ProgramTime;
				}
			}
			
			FreeString(&gl->procmon.heap, &event->itemName);
		}
		VarArrayClear(&gl->procmon.itemEvents);
		
		plat->UnlockMutex(&gl->procmon.mutex);
	}
	#endif
	
	// +==============================+
	// |         Update Items         |
	// +==============================+
	VarArrayLoop(&main->items, iIndex)
	{
		VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex);
		item->isHovered = IsMouseOverPrint("ViewportItem%llu", iIndex);
		
		// +==============================+
		// |     Handle Item Clicked      |
		// +==============================+
		if (item->isHovered && MousePressedAndHandleExtended(MouseBtn_Left))
		{
			if (KeyDownRaw(Key_Shift) && main->primarySelectedItemIndex >= 0)
			{
				//Select range of items
				main->numSelectedItems = 0;
				u64 minIndex = MinU64((u64)main->primarySelectedItemIndex, iIndex);
				u64 maxIndex = MaxU64((u64)main->primarySelectedItemIndex, iIndex);
				VarArrayLoop(&main->items, iIndex2)
				{
					VarArrayLoopGet(FolderFileItem_t, item2, &main->items, iIndex2);
					if (iIndex2 >= minIndex && iIndex2 <= maxIndex)
					{
						item2->isSelected = true;
						main->numSelectedItems++;
					}
					else { item2->isSelected = false; }
				}
				// main->primarySelectedItemIndex = (i64)iIndex;
			}
			else if (KeyDownRaw(Key_Control))
			{
				//Toggle selection on this item
				item->isSelected = !item->isSelected;
				if (item->isSelected) { main->numSelectedItems++; }
				else { main->numSelectedItems--; }
				main->primarySelectedItemIndex = (i64)iIndex;
			}
			else
			{
				if (item->isSelected && TimeSince(item->lastClickTime) < plat->GetRapidClickMaxTime())
				{
					item->quickClickCount++;
				}
				else { item->quickClickCount = 1; }
				item->lastClickTime = ProgramTime;
				
				if (item->quickClickCount == 2)
				{
					if (item->isFolder)
					{
						MyStr_t newPath = PrintInArenaStr(scratch, "%.*s/%.*s", StrPrint(main->currentPath), StrPrint(item->name));
						MoveToPath(newPath);
					}
					else
					{
						NotifyWrite_W("This tool doesn't support opening files yet");
					}
				}
				else
				{
					//Deselect all items
					VarArrayLoop(&main->items, iIndex2) { VarArrayLoopGet(FolderFileItem_t, item2, &main->items, iIndex2); item2->isSelected = false; }
					//Select this specific item
					item->isSelected = true;
					main->primarySelectedItemIndex = (i64)iIndex;
					main->numSelectedItems = 1;
				}
			}
		}
	}
	
	// +==================================+
	// | Enter Key Opens Selected Folder  |
	// +==================================+
	//TODO: This eventually should open multiple tabs if multiple folders are selected
	if (KeyPressed(Key_Enter) && main->primarySelectedItemIndex >= 0 && main->numSelectedItems == 1)
	{
		HandleKeyExtended(Key_Enter);
		FolderFileItem_t* selectedItem = VarArrayGetHard(&main->items, (u64)main->primarySelectedItemIndex, FolderFileItem_t);
		if (selectedItem->isFolder)
		{
			MyStr_t newPath = PrintInArenaStr(scratch, "%.*s/%.*s", StrPrint(main->currentPath), StrPrint(selectedItem->name));
			MoveToPath(newPath);
		}
	}
	
	FreeScratchArena(scratch);
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void RenderMainAppState(FrameBuffer_t* renderBuffer, bool bottomLayer)
{
	UNUSED(bottomLayer);
	NotNull(main);
	MemArena_t* scratch = GetScratchArena();
	MainAppStateAccessResources();
	LayoutMainAppState();
	
	RcBegin(pig->currentWindow, renderBuffer, &pig->resources.shaders->main2D, Black);
	
	// +==============================+
	// |        Render Panels         |
	// +==============================+
	rec sidebarRec = main->sidebarDivider.leftRec + main->sidebarDivider.mainRec.topLeft;
	RcDrawRectangle(sidebarRec, pig->theme.background);
	RcDrawRectangleOutline(sidebarRec, pig->theme.panelOutline, 1);
	
	RcDrawRectangle(main->viewportRec, pig->theme.background);
	
	RcDrawRectangle(main->topbarRec, pig->theme.background);
	
	// +==============================+
	// |         Render Items         |
	// +==============================+
	v2 itemsOffset = main->viewportRec.topLeft - main->viewportScroll.scroll;
	RcSetViewport(main->viewportRec);
	RcBindFont(&pig->resources.fonts->files, SelectDefaultFontFace());
	VarArrayLoop(&main->items, iIndex)
	{
		VarArrayLoopGet(FolderFileItem_t, item, &main->items, iIndex);
		rec mainRec = item->mainRec + itemsOffset;
		rec iconRec = item->iconRec + mainRec.topLeft;
		rec nameRec = item->nameRec + mainRec.topLeft;
		rec hitRec = item->hitRec + mainRec.topLeft;
		hitRec.height -= 1; //Leave 1px space between selected items so they are visually separated
		
		if (RecsIntersect(hitRec, main->viewportRec))
		{
			v2 namePos = nameRec.topLeft + item->nameMeasure.offset;
			Vec2Align(&namePos);
			v2i builtinIconFrame;
			if (item->isFolder) { builtinIconFrame = NewVec2i(2, 0); }
			else
			{
				MyStr_t extensionStr = MyStr_Empty;
				SplitFilePath(item->name, nullptr, nullptr, &extensionStr);
				builtinIconFrame = IsEmptyStr(extensionStr) ? NewVec2i(0, 0) : NewVec2i(1, 0);
			}
			bool isHidden = StrStartsWith(item->name, ".");
			Color_t nameColor = pig->theme.text;
			Color_t iconColor = White;
			Texture_t* osIconTexture = nullptr;
			if (item->fileIcon != nullptr)
			{
				if (item->fileIcon->texture.isValid)
				{
					osIconTexture = &item->fileIcon->texture;
				}
				else { iconColor = item->fileIcon->color; }
			}
			if (isHidden && !item->isHovered && !item->isSelected)
			{
				nameColor = pig->theme.textFaded;
				iconColor = ColorTransparent(iconColor, 0.75f);
			}
			
			// RcDrawRectangle(mainRec, (iIndex%2) ? MonokaiGray2 : MonokaiGray1);
			if (item->isSelected)
			{
				RcDrawRectangle(hitRec, pig->theme.selected);
			}
			else if (item->isHovered)
			{
				RcDrawRectangle(hitRec, pig->theme.highlight);
			}
			else if (main->primarySelectedItemIndex >= 0 && (u64)main->primarySelectedItemIndex == iIndex)
			{
				RcDrawRectangleOutline(hitRec, pig->theme.highlight, 1);
			}
			
			rec oldViewportRec = RcAndViewport(mainRec);
			
			//If the OS provided an icon for this file, then let's use that icon instead of our builtin default one
			if (osIconTexture != nullptr)
			{
				RcBindTexture1(osIconTexture);
				RcDrawTexturedRectangle(iconRec, iconColor);
			}
			else
			{
				RcBindSpriteSheet(&pig->resources.sheets->fileIcons);
				RcDrawSheetFrame(builtinIconFrame, iconRec, iconColor);
			}
			
			RcDrawText(item->name, namePos, nameColor);
			
			RcSetViewport(oldViewportRec);
			
			if (item->processRefs.length > 0)
			{
				rec processIconRec = NewRec(rc->flowInfo.endPos.x + 5, mainRec.y + mainRec.height/2 - 16, 32, 32);
				VarArrayLoop(&item->processRefs, pIndex)
				{
					VarArrayLoopGet(ItemProcessRef_t, processRef, &item->processRefs, pIndex);
					if (processRef->process->icon != nullptr && processRef->process->icon->texture.isValid)
					{
						RcBindTexture1(&processRef->process->icon->texture);
						RcDrawTexturedRectangle(processIconRec, White);
					}
					else
					{
						RcDrawRectangle(processIconRec, GetPredefPalColorByIndex(pIndex));
					}
					processIconRec.x += processIconRec.width + 2;
				}
			}
		}
	}
	RcSetViewport(ScreenRec);
	
	// +==============================+
	// |     Render Process Items     |
	// +==============================+
	#if PROCMON_SUPPORTED
	if (platInfo->wasRunInAdministratorMode)
	{
		RcSetViewport(sidebarRec);
		rec iconRec = NewRec(sidebarRec.topLeft + NewVec2(5, 5), 32, 32);
		for (u64 pIndex = 0; pIndex < gl->procmon.processes.length; pIndex++)
		{
			if (plat->LockMutex(&gl->procmon.mutex, MUTEX_LOCK_INFINITE))
			{
				ProcmonProcess_t* process = BktArrayGetHard(&gl->procmon.processes, ProcmonProcess_t, pIndex);
				plat->UnlockMutex(&gl->procmon.mutex);
				
				if (!process->triedToLoadIcon)
				{
					u64 iconId = plat->GetFileIconId(process->exePath);
					process->icon = FindFileIconById(&main->iconCache, iconId, process->exePath);
					process->triedToLoadIcon = true;
				}
				
				v2 namePos = NewVec2(
					iconRec.x + iconRec.width + 2,
					iconRec.y + iconRec.height/2 - RcGetLineHeight()/2 + RcGetMaxAscend()
				);
				Vec2Align(&namePos);
				
				if (process->icon != nullptr && process->icon->texture.isValid)
				{
					RcBindTexture1(&process->icon->texture);
					RcDrawTexturedRectangle(iconRec, White);
				}
				else
				{
					RcDrawRectangle(iconRec, pig->theme.highlight);
				}
				RcDrawTextPrint(namePos, pig->theme.text, "[0x%04X] %.*s (%llu+%llu)", process->id, StrPrint(process->name), process->numEvents, process->eventsSinceLastFrame);
				process->eventsSinceLastFrame = 0;
				
				iconRec.y += iconRec.height + 5;
			}
		}
		RcSetViewport(ScreenRec);
	}
	else
	{
		bool isMouseOverBtn = IsMouseOverNamed("SidebarRestartWithAdminBtn");
		v2 adminTextPos = NewVec2(sidebarRec.x + 5, sidebarRec.y + 5 + RcGetMaxAscend());
		Vec2Align(&adminTextPos);
		r32 adminTextMaxWidth = sidebarRec.x + sidebarRec.width - adminTextPos.x;
		RcDrawText(RESTART_IN_ADMIN_MODE_MESSAGE,
			adminTextPos,
			pig->theme.text,
			TextAlignment_Left,
			adminTextMaxWidth
		);
		
		MyStr_t btnText = NewStr("\bRestart in Administrator Mode\b");
		r32 btnTextMaxWidth = main->restartWithAdminBtnRec.width - pig->resources.sheets->buttonIcons.frameSize.width*2;
		TextMeasure_t btnTextMeasure = RcMeasureText(btnText, btnTextMaxWidth);
		v2 btnTextPos = NewVec2(
			main->restartWithAdminBtnRec.x + main->restartWithAdminBtnRec.width/2,
			main->restartWithAdminBtnRec.y + main->restartWithAdminBtnRec.height/2 - btnTextMeasure.size.height/2 + btnTextMeasure.offset.y
		);
		Color_t btnColor = isMouseOverBtn ? pig->theme.selected : pig->theme.highlight;
		rec shieldIconRec = NewRec(btnTextPos, ToVec2(pig->resources.sheets->buttonIcons.frameSize));
		
		RcDrawRectangle(main->restartWithAdminBtnRec, btnColor);
		RcDrawText(btnText, btnTextPos, pig->theme.text, TextAlignment_Center, btnTextMaxWidth);
		
		shieldIconRec.x = rc->flowInfo.renderRec.x - shieldIconRec.width;
		shieldIconRec.y = btnTextPos.y - shieldIconRec.height;
		RcBindSpriteSheet(&pig->resources.sheets->buttonIcons);
		RcDrawSheetFrame(NewVec2i(1, 1), shieldIconRec, White);
	}
	#endif
	
	// +==============================+
	// |      Render ScrollBars       |
	// +==============================+
	RcDrawRectangleOutline(main->viewportRec, pig->theme.panelOutline, 1);
	RenderUiScrollView(&main->viewportScroll);
	
	// +==============================+
	// |    Render Topbar Buttons     |
	// +==============================+
	for (u64 bIndex = 0; bIndex < 3; bIndex++)
	{
		rec btnRec = Rec_Zero;
		v2i iconFrame = Vec2i_Zero;
		const char* btnHoverName = "";
		bool isEnabled = true;
		switch (bIndex)
		{
			case 0: btnRec = main->backBtnRec;    btnHoverName = "BackBtn";    iconFrame = NewVec2i(0, 0); isEnabled = true;  break;
			case 1: btnRec = main->forwardBtnRec; btnHoverName = "ForwardBtn"; iconFrame = NewVec2i(1, 0); isEnabled = false; break;
			case 2: btnRec = main->upBtnRec;      btnHoverName = "UpBtn";      iconFrame = NewVec2i(0, 1); isEnabled = true;  break;
		}
		bool isHovered = IsMouseOverNamed(btnHoverName);
		Color_t backColor = (isHovered && isEnabled) ? pig->theme.hover : Transparent;
		
		RcDrawRectangle(RecInflate(btnRec, INTER_BTN_MARGIN/2, 0), backColor);
		RcBindSpriteSheet(&pig->resources.sheets->buttonIcons);
		RcDrawSheetFrame(iconFrame, btnRec, isEnabled ? pig->theme.text : pig->theme.textFaded);
	}
	
	// RcDrawRectangle(main->pathTextboxRec, MonokaiGray1);
	v2 piecePos = NewVec2(main->pathTextboxRec.x, main->pathTextboxRec.y + main->pathTextboxRec.height/2 - RcGetMaxAscend() + RcGetLineHeight());
	Vec2Align(&piecePos);
	u64 numPathPieces = 0;
	MyStr_t* pathPieces = SplitString(scratch, main->currentPath, "/", &numPathPieces);
	for (u64 pIndex = 0; pIndex < numPathPieces; pIndex++)
	{
		Color_t pieceColor = (pIndex == numPathPieces-1) ? pig->theme.text : pig->theme.textFaded;
		RcDrawText(pathPieces[pIndex], piecePos, pieceColor);
		piecePos.x = rc->flowInfo.endPos.x + PATH_PIECE_MARGIN;
		RcDrawText("/", piecePos, pieceColor);
		piecePos.x = rc->flowInfo.endPos.x + PATH_PIECE_MARGIN;
	}
	
	RenderUiDivider(&main->sidebarDivider, pig->theme.backgroundDark);
	
	// +==============================+
	// | Render Assertion Status Text |
	// +==============================+
	if (false)
	{
		RcBindFont(&pig->resources.fonts->debug, SelectDefaultFontFace());
		v2 textPos = NewVec2(10, ScreenSize.height - 10 - RcGetMaxDescend());
		Vec2Align(&textPos);
		
		if (pig->debugOverlay.enabled || DEVELOPER_BUILD)
		{
			if (GYLIB_ASSERTIONS_ENABLED)
			{
				RcDrawTextPrint(textPos, MonokaiWhite, "Assertions \b\fenabled\f\b %s \b\f%s\f\b", pig->dontExitOnAssert ? "but" : "and", pig->dontExitOnAssert ? "won't exit" : "will exit");
				textPos.y -= RcGetLineHeight();
			}
			else
			{
				RcDrawText("Assertions \b\fdisabled\f\b", textPos, MonokaiWhite);
				textPos.y -= RcGetLineHeight();
			}
		}
		if (DEVELOPER_BUILD)
		{
			RcDrawText("DEVELOPER_BUILD", textPos, MonokaiWhite);
			textPos.y -= RcGetLineHeight();
		}
		if (DEBUG_BUILD)
		{
			RcDrawText("DEBUG_BUILD", textPos, MonokaiWhite);
			textPos.y -= RcGetLineHeight();
		}
		if (DEMO_BUILD)
		{
			RcDrawText("DEMO_BUILD", textPos, MonokaiWhite);
			textPos.y -= RcGetLineHeight();
		}
		#if STEAM_BUILD
		{
			if (pig->debugOverlay.enabled || DEVELOPER_BUILD)
			{
				RcDrawTextPrint(textPos, MonokaiWhite, "STEAM_BUILD (%llu)", platInfo->steamAppId);
			}
			else
			{
				RcDrawText("STEAM_BUILD", textPos, MonokaiWhite);
			}
			textPos.y -= RcGetLineHeight();
		}
		#endif
		
		RcDrawTextPrint(textPos, MonokaiWhite, "Platform v%u.%02u(%03u)", platInfo->version.major, platInfo->version.minor, platInfo->version.build);
		textPos.y -= RcGetLineHeight();
		RcDrawTextPrint(textPos, MonokaiWhite, "Engine   v%llu.%02llu(%03llu)", (u64)ENGINE_VERSION_MAJOR, (u64)ENGINE_VERSION_MINOR, (u64)ENGINE_VERSION_BUILD);
		textPos.y -= RcGetLineHeight();
		if (pig->reloadIndex > 1)
		{
			RcDrawTextPrint(textPos, MonokaiWhite, "Game     v%llu.%02llu(%03llu) (reload %llu)", (u64)GAME_VERSION_MAJOR, (u64)GAME_VERSION_MINOR, (u64)GAME_VERSION_BUILD, pig->reloadIndex);
			textPos.y -= RcGetLineHeight();
		}
		else
		{
			RcDrawTextPrint(textPos, MonokaiWhite, "Game     v%llu.%02llu(%03llu)", (u64)GAME_VERSION_MAJOR, (u64)GAME_VERSION_MINOR, (u64)GAME_VERSION_BUILD);
			textPos.y -= RcGetLineHeight();
		}
	}
	
	FreeScratchArena(scratch);
}
