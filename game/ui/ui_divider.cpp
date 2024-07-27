/*
File:   ui_divider.cpp
Author: Taylor Robbins
Date:   07\25\2024
Description: 
	** A UI Divider takes a rectangle and splits it horizontally or vertically
	** into two sections and allows the user to grab the divider and change the
	** split between the two sections
*/

#define DEFAULT_DIVIDER_HIT_WIDTH   5 //px

void UiDividerApplySplitRestrictions(UiDivider_t* divider)
{
	divider->split = divider->targetSplit;
	if (divider->isHorizontal)
	{
		if (divider->mainRec.width <= 0) { divider->split = 0; return; } //degenerate case, avoid dividing by 0
		r32 splitPercent = divider->isProportional ? divider->split : (divider->split / divider->mainRec.width);
		r32 minSplitPx = (divider->minLeftSizePx > 0) ? MinR32(divider->minLeftSizePx, divider->mainRec.width) : 0;
		r32 minSplitPercent = (divider->minLeftSizePercent > 0) ? MinR32(divider->minLeftSizePercent, 1.0f) : 0.0f;
		r32 maxSplitPx = (divider->minRightSizePx > 0) ? MaxR32(0, divider->mainRec.width - divider->minRightSizePx) : divider->mainRec.width;
		r32 maxSplitPercent = (divider->minRightSizePercent > 0) ? MaxR32(0.0f, 1.0f - divider->minRightSizePercent) : 1.0f;
		minSplitPercent = MaxR32(minSplitPercent, minSplitPx / divider->mainRec.width);
		maxSplitPercent = MinR32(maxSplitPercent, maxSplitPx / divider->mainRec.width);
		//split the difference if both restrictions are being applied
		if (minSplitPercent > maxSplitPercent) { minSplitPercent = (minSplitPercent + maxSplitPercent)/2; maxSplitPercent = minSplitPercent; }
		splitPercent = ClampR32(splitPercent, minSplitPercent, maxSplitPercent);
		divider->split = (divider->isProportional ? splitPercent : splitPercent * divider->mainRec.width);
	}
	else
	{
		if (divider->mainRec.height <= 0) { divider->split = 0; return; } //degenerate case, avoid dividing by 0
		r32 splitPercent = divider->isProportional ? divider->split : (divider->split / divider->mainRec.height);
		r32 minSplitPx = (divider->minTopSizePx > 0) ? MinR32(divider->minTopSizePx, divider->mainRec.height) : 0;
		r32 minSplitPercent = (divider->minTopSizePercent > 0) ? MinR32(divider->minTopSizePercent, 1.0f) : 0.0f;
		r32 maxSplitPx = (divider->minBottomSizePx > 0) ? MaxR32(0, divider->mainRec.height - divider->minBottomSizePx) : divider->mainRec.height;
		r32 maxSplitPercent = (divider->minBottomSizePercent > 0) ? MaxR32(0.0f, 1.0f - divider->minBottomSizePercent) : 1.0f;
		minSplitPercent = MaxR32(minSplitPercent, minSplitPx / divider->mainRec.height);
		maxSplitPercent = MinR32(maxSplitPercent, maxSplitPx / divider->mainRec.height);
		//split the difference if both restrictions are being applied
		if (minSplitPercent > maxSplitPercent) { minSplitPercent = (minSplitPercent + maxSplitPercent)/2; maxSplitPercent = minSplitPercent; }
		splitPercent = ClampR32(splitPercent, minSplitPercent, maxSplitPercent);
		divider->split = (divider->isProportional ? splitPercent : splitPercent * divider->mainRec.height);
	}
}

void InitUiDivider(UiDivider_t* divider, bool isHorizontal, r32 isProportional, r32 initialSplit, rec mainRec = Rec_Zero_Const)
{
	NotNull(divider);
	ClearPointer(divider);
	divider->id = pig->nextUiId;
	pig->nextUiId++;
	divider->isHorizontal = isHorizontal;
	divider->isProportional = isProportional;
	divider->hitThickness = DEFAULT_DIVIDER_HIT_WIDTH;
	divider->mainRec = mainRec;
	divider->targetSplit = initialSplit;
	UiDividerApplySplitRestrictions(divider);
}

void LayoutUiDivider(UiDivider_t* divider)
{
	NotNull(divider);
	UiDividerApplySplitRestrictions(divider);
	if (divider->isHorizontal)
	{
		r32 offset = ClampR32(divider->isProportional ? divider->mainRec.width * divider->split : divider->split, 0, divider->mainRec.width);
		divider->dividerRec = NewRec(offset, 0, 0, divider->mainRec.height);
		RecAlign(&divider->dividerRec);
		divider->dividerHitRec = RecInflateX(divider->dividerRec, divider->hitThickness/2);
		RecAlign(&divider->dividerHitRec);
		divider->leftRec = NewRec(0, divider->mainRec.y, divider->dividerRec.x, divider->mainRec.height);
		RecAlign(&divider->leftRec);
		divider->rightRec = NewRec(divider->dividerRec.x, divider->mainRec.y, divider->mainRec.width - divider->dividerRec.x, divider->mainRec.height);
		RecAlign(&divider->rightRec);
	}
	else
	{
		r32 offset = ClampR32(divider->isProportional ? divider->mainRec.height * divider->split : divider->split, 0, divider->mainRec.height);
		divider->dividerRec = NewRec(0, offset, divider->mainRec.width, 0);
		RecAlign(&divider->dividerRec);
		divider->dividerHitRec = RecInflateY(divider->dividerRec, divider->hitThickness/2);
		RecAlign(&divider->dividerHitRec);
		divider->topRec = NewRec(divider->mainRec.x, 0, divider->mainRec.width, divider->dividerRec.y);
		RecAlign(&divider->topRec);
		divider->bottomRec = NewRec(divider->mainRec.x, divider->dividerRec.y, divider->mainRec.width, divider->mainRec.height - divider->dividerRec.y);
		RecAlign(&divider->bottomRec);
	}
}

void UiDividerCaptureMouse(UiDivider_t* divider, bool capturePanels = false)
{
	NotNull(divider);
	LayoutUiDivider(divider);
	if (divider->isMouseDragging) { MouseHitPrint("Divider%lluDragging", divider->id); }
	MouseHitRecPrint(divider->dividerHitRec + divider->mainRec.topLeft, "Divider%lluDivide", divider->id);
	if (capturePanels)
	{
		MouseHitRecPrint(divider->leftRec, "Divider%llu%s", divider->id, divider->isHorizontal ? "Left" : "Top");
		MouseHitRecPrint(divider->rightRec, "Divider%llu%s", divider->id, divider->isHorizontal ? "Right" : "Bottom");
		MouseHitRecPrint(divider->mainRec, "Divider%llu", divider->id); //really we shouldn't ever see this be the capturing name, but better to be safe
	}
}

void UpdateUiDivider(UiDivider_t* divider)
{
	// +==============================+
	// |   Handle Mouse Interaction   |
	// +==============================+
	if (IsMouseOverPrint("Divider%lluDivide", divider->id) || divider->isMouseDragging)
	{
		pigOut->cursorType = divider->isHorizontal ? PlatCursor_ResizeHorizontal : PlatCursor_ResizeVertical;
		if (MousePressed(MouseBtn_Left))
		{
			HandleMouse(MouseBtn_Left);
			divider->isMouseDragging = true;
			if (divider->isHorizontal)
			{
				divider->mouseDraggingOffset = MousePos.x - (divider->mainRec.x + divider->dividerRec.x);
			}
			else
			{
				divider->mouseDraggingOffset = MousePos.y - (divider->mainRec.y + divider->dividerRec.y);
			}
		}
	}
	
	// +==============================+
	// |      Handle Mouse Drag       |
	// +==============================+
	if (divider->isMouseDragging)
	{
		if (MouseDownRaw(MouseBtn_Left))
		{
			r32 newSplit = 0;
			if (divider->isHorizontal)
			{
				newSplit = ClampR32((MousePos.x - divider->mouseDraggingOffset) - divider->mainRec.x, 0, divider->mainRec.width);
				if (divider->isProportional) { newSplit = newSplit / divider->mainRec.width; }
			}
			else
			{
				newSplit = ClampR32((MousePos.y - divider->mouseDraggingOffset) - divider->mainRec.y, 0, divider->mainRec.height);
				if (divider->isProportional) { newSplit = newSplit / divider->mainRec.height; }
			}
			if (divider->targetSplit != newSplit)
			{
				divider->targetSplit = newSplit;
				r32 oldRestrictedSplit = divider->split;
				UiDividerApplySplitRestrictions(divider);
				if (divider->split != oldRestrictedSplit) { divider->splitChanged = true; }
			}
		}
		else { divider->isMouseDragging = false; }
		
		HandleMouse(MouseBtn_Left);
	}
	
	
	// +==============================+
	// |     Handle Window Resize     |
	// +==============================+
	// if (pig->currentWindow->input.resized)
	// {
	// 	if (main->sidebarWidth < MIN_SIDEBAR_WIDTH) { main->sidebarWidth = MIN_SIDEBAR_WIDTH; }
	// 	if (main->sidebarWidth < main->userSetSidebarWidth) { main->sidebarWidth = main->userSetSidebarWidth; }
	// 	if (main->sidebarWidth > ScreenSize.width - MIN_VIEWPORT_WIDTH)
	// 	{
	// 		main->sidebarWidth = ScreenSize.width - MIN_VIEWPORT_WIDTH;
	// 		if (main->sidebarWidth < 10) { main->sidebarWidth = 10; }
	// 	}
	// }
}

void RenderUiDivider(UiDivider_t* divider, Color_t dividerColor)
{
	LayoutUiDivider(divider);
	RcDrawRectangle(divider->dividerHitRec + divider->mainRec.topLeft, dividerColor);
}
