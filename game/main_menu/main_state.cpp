/*
File:   main_state.cpp
Author: Taylor Robbins
Date:   07\02\2024
Description: 
	** We don't use app states in this tool, so this is the main and only app state
*/

#include "ui/ui_scroll_view.cpp"

#include "main_menu/main_helpers.cpp"
#include "main_menu/file_icon_cache.cpp"

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

#define PANEL_BACK_COLOR    NewColor(50, 50, 50) //0xFF323232
#define PANEL_OUTLINE_COLOR NewColor(85, 85, 85) //0xFF555555
#define DIVIDER_COLOR       NewColor(43, 43, 43) //0xFF2B2B2B (also used for scroll gutter)
#define HEADER_BACK_COLOR   NewColor(64, 67, 70) //0xFF404346 (also used for scroll bar)
#define TEXT_COLOR          NewColor(242, 242, 242) //0xFFF2F2F2
#define TEXT_GREY_COLOR     NewColor(128, 128, 128) //0xFF808080
#define OUTLINE_COLOR       NewColor(152, 118, 170) //0xFF9876AA
#define RULER_COLOR         NewColor(57, 57, 57) //0xFF393939
#define BTN_HOVER_COLOR     NewColor(76, 67, 80) //0xFF4C4350
#define HIGHLIGHT_COLOR     NewColor(119, 99, 132) //0xFF776384

void FreeFolderFileItem(MemArena_t* allocArena, FolderFileItem_t* item)
{
	NotNull2(allocArena, item);
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
		if (item->mainRec.width > maxWidth)
		{
			item->mainRec.width = maxWidth;
			item->nameRec.width = (item->mainRec.width - ITEM_INNER_PADDING_LEFT_RIGHT) - item->nameRec.x;
		}
		item->iconRec.y = item->mainRec.height/2 - item->iconRec.height/2;
		item->nameRec.y = item->mainRec.height/2 - item->nameRec.height/2;
		RecAlign(&item->iconRec);
		RecAlign(&item->nameRec);
		item->mainRec.x = 0;
		item->mainRec.y = contentRec.y + contentRec.height + (iIndex > 0 ? INTER_ITEM_MARGIN : 0);
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
		numFiles++;
	}
	
	PrintLine_I("Found %llu folder%s and %llu file%s", numFolders, Plural(numFolders, "s"), numFiles, Plural(numFiles, "s"));
	
	FreeScratchArena(scratch);
}

void MoveToPath(MyStr_t newPath)
{
	while (StrEndsWith(newPath, "/")) { newPath.length--; }
	if (StrEquals(main->currentPath, newPath)) { return; }
	FreeString(mainHeap, &main->currentPath);
	main->currentPath = AllocString(mainHeap, &newPath);
	UpdateFolderFileItemsAtCurrentPath();
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
	NotNull(main);
	MemArena_t* scratch = GetScratchArena();
	
	if (initialize)
	{
		ClearPointer(main);
		
		main->sidebarWidth = ScreenSize.width * 0.25f;
		if (main->sidebarWidth < MIN_SIDEBAR_WIDTH) { main->sidebarWidth = MIN_SIDEBAR_WIDTH; }
		main->userSetSidebarWidth = main->sidebarWidth;
		
		CreateVarArray(&main->items, mainHeap, sizeof(FolderFileItem_t));
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
	
	main->sidebarRec = NewRec(PANEL_MARGIN, PANEL_MARGIN, main->sidebarWidth - (PANEL_DIVIDER_WIDTH/2.0f), ScreenSize.height - PANEL_MARGIN*2);
	RecAlign(&main->sidebarRec);
	main->dividerRec = NewRec(main->sidebarRec.x + main->sidebarRec.width, 0, PANEL_DIVIDER_WIDTH, ScreenSize.height);
	RecAlign(&main->dividerRec);
	
	main->viewportRec = NewRec(main->dividerRec.x + main->dividerRec.width, PANEL_MARGIN, 0, 0);
	main->viewportRec.width = (ScreenSize.width - PANEL_MARGIN) - main->viewportRec.x;
	RecAlign(&main->viewportRec);
	
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
	
	UpdateScrollViewMainRec(&main->viewportScroll, main->viewportRec, MainContentSizeFunc);
}

void CaptureMouseMainAppState()
{
	if (main->resizingSidebar) { MouseHitNamed("Divider"); }
	MouseHitRecNamed(RecInflate(main->backBtnRec, INTER_BTN_MARGIN/2, 0), "BackBtn");
	MouseHitRecNamed(RecInflate(main->forwardBtnRec, INTER_BTN_MARGIN/2, 0), "ForwardBtn");
	MouseHitRecNamed(RecInflate(main->upBtnRec, INTER_BTN_MARGIN/2, 0), "UpBtn");
	MouseHitRecNamed(RecInflate(main->dividerRec, 2, 0), "Divider");
	if (main->viewportScroll.scrollMax.x > main->viewportScroll.scrollMin.x)
	{
		MouseHitRecNamed(main->viewportScroll.horiScrollBarRec, "ViewportHoriScrollBar");
		MouseHitRecNamed(main->viewportScroll.horiScrollGutterRec, "ViewportHoriScrollGutter");
	}
	if (main->viewportScroll.scrollMax.y > main->viewportScroll.scrollMin.y)
	{
		MouseHitRecNamed(main->viewportScroll.vertScrollBarRec, "ViewportVertScrollBar");
		MouseHitRecNamed(main->viewportScroll.vertScrollGutterRec, "ViewportVertScrollGutter");
	}
	MouseHitRecNamed(RecInflate(main->viewportRec, PANEL_MARGIN, PANEL_MARGIN), "Viewport");
	MouseHitRecNamed(RecInflate(main->sidebarRec, PANEL_MARGIN, PANEL_MARGIN), "Sidebar");
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateMainAppState()
{
	NotNull(main);
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
	
	// +==============================+
	// |     Handle Window Resize     |
	// +==============================+
	if (pig->currentWindow->input.resized)
	{
		if (main->sidebarWidth < MIN_SIDEBAR_WIDTH) { main->sidebarWidth = MIN_SIDEBAR_WIDTH; }
		if (main->sidebarWidth < main->userSetSidebarWidth) { main->sidebarWidth = main->userSetSidebarWidth; }
		if (main->sidebarWidth > ScreenSize.width - MIN_VIEWPORT_WIDTH)
		{
			main->sidebarWidth = ScreenSize.width - MIN_VIEWPORT_WIDTH;
			if (main->sidebarWidth < 10) { main->sidebarWidth = 10; }
		}
	}
	
	// +==================================+
	// | Handle Divider Mouse Interaction |
	// +==================================+
	if (IsMouseOverNamed("Divider"))
	{
		pigOut->cursorType = PlatCursor_ResizeHorizontal;
		
		if (main->resizingSidebar)
		{
			HandleMouse(MouseBtn_Left);
			if (MouseDownRaw(MouseBtn_Left))
			{
				main->sidebarWidth = MousePos.x - main->resizingSidebarGrabOffset;
				main->userSetSidebarWidth = main->sidebarWidth;
				if (main->sidebarWidth < MIN_SIDEBAR_WIDTH) { main->sidebarWidth = MIN_SIDEBAR_WIDTH; }
				else if (main->sidebarWidth > ScreenSize.width - MIN_VIEWPORT_WIDTH) { main->sidebarWidth = ScreenSize.width - MIN_VIEWPORT_WIDTH; }
			}
			else
			{
				main->resizingSidebar = false;
			}
		}
		else if (MousePressed(MouseBtn_Left))
		{
			HandleMouse(MouseBtn_Left);
			main->resizingSidebar = true;
			main->resizingSidebarGrabOffset = MousePos.x - main->sidebarWidth;
		}
	}
	
	// +==============================+
	// |        Handle Buttons        |
	// +==============================+
	if (IsMouseOverNamed("BackBtn"))
	{
		// pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			Unimplemented(); //TODO: Implement me!
		}
	}
	if (IsMouseOverNamed("ForwardBtn"))
	{
		// pigOut->cursorType = PlatCursor_Pointer;
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			Unimplemented(); //TODO: Implement me!
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
	
	UpdateScrollView(&main->viewportScroll, IsMouseOverNamedPartial("Viewport"), IsMouseOverNamed("ViewportHoriScrollBar"), IsMouseOverNamed("ViewportVertScrollBar"));
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
	RcDrawRectangle(main->sidebarRec, PANEL_BACK_COLOR);
	RcDrawRectangleOutline(main->sidebarRec, PANEL_OUTLINE_COLOR, 1);
	
	RenderScrollView(&main->viewportScroll, PANEL_BACK_COLOR, DIVIDER_COLOR, HEADER_BACK_COLOR, HIGHLIGHT_COLOR);
	// RcDrawRectangle(main->viewportRec, PANEL_BACK_COLOR);
	RcDrawRectangleOutline(main->viewportRec, PANEL_OUTLINE_COLOR, 1);
	
	RcDrawRectangle(main->topbarRec, PANEL_BACK_COLOR);
	
	RcDrawRectangle(main->dividerRec, DIVIDER_COLOR);
	
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
		Color_t nameColor = isHidden ? TEXT_GREY_COLOR : TEXT_COLOR;
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
		if (isHidden) { iconColor = ColorTransparent(iconColor, 0.75f); }
		
		// RcDrawRectangle(mainRec, (iIndex%2) ? MonokaiGray2 : MonokaiGray1);
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
	}
	RcSetViewport(ScreenRec);
	
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
		Color_t backColor = (isHovered && isEnabled) ? BTN_HOVER_COLOR : Transparent;
		
		RcDrawRectangle(RecInflate(btnRec, INTER_BTN_MARGIN/2, 0), backColor);
		RcBindSpriteSheet(&pig->resources.sheets->buttonIcons);
		RcDrawSheetFrame(iconFrame, btnRec, isEnabled ? TEXT_COLOR : TEXT_GREY_COLOR);
	}
	
	// RcDrawRectangle(main->pathTextboxRec, MonokaiGray1);
	v2 piecePos = NewVec2(main->pathTextboxRec.x, main->pathTextboxRec.y + main->pathTextboxRec.height/2 - RcGetMaxAscend() + RcGetLineHeight());
	Vec2Align(&piecePos);
	u64 numPathPieces = 0;
	MyStr_t* pathPieces = SplitString(scratch, main->currentPath, "/", &numPathPieces);
	for (u64 pIndex = 0; pIndex < numPathPieces; pIndex++)
	{
		Color_t pieceColor = (pIndex == numPathPieces-1) ? TEXT_COLOR : TEXT_GREY_COLOR;
		RcDrawText(pathPieces[pIndex], piecePos, pieceColor);
		piecePos.x = rc->flowInfo.endPos.x + PATH_PIECE_MARGIN;
		RcDrawText("/", piecePos, pieceColor);
		piecePos.x = rc->flowInfo.endPos.x + PATH_PIECE_MARGIN;
	}
	
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
