/*
File:   ui_scroll_view.cpp
Author: Taylor Robbins
Date:   07\07\2024
Description: 
	** A scroll view helps manage the viewport into a 2D set of content that is potentially larger
	** than an allotted rectangle on screen through which we can view and interact with that content.
	** This doesn't really "manage" the contents of the view, it simply provides a framework to make
	** make it easy to manage the contents, and give it the info it needs to behave properly inside
	** that alloted view space. This allows us to be pretty hands-off about what kinds of things can
	** be put inside a scroll view, but it also puts the burden on the caller to route info around.
*/

#define UI_SCROLL_VIEW_GUTTER_WIDTH               20 //px
#define UI_SCROLL_VIEW_SCROLLBAR_HIGHLIGHT_TIME   300 //ms

//TODO: Horizontal scroll bar overlaps vertical scroll bar in bottom right

void ClampUiScrollViewScroll(ScrollView_t* scrollView)
{
	NotNull(scrollView);
	scrollView->scroll.x = ClampR32(scrollView->scroll.x, scrollView->scrollMin.x, scrollView->scrollMax.x);
	scrollView->scroll.y = ClampR32(scrollView->scroll.y, scrollView->scrollMin.y, scrollView->scrollMax.y);
	scrollView->scrollGoto.x = ClampR32(scrollView->scrollGoto.x, scrollView->scrollMin.x, scrollView->scrollMax.x);
	scrollView->scrollGoto.y = ClampR32(scrollView->scrollGoto.y, scrollView->scrollMin.y, scrollView->scrollMax.y);
}

void LayoutUiScrollView(ScrollView_t* scrollView)
{
	bool isHoriScrollBarVisible = (scrollView->scrollMax.x > scrollView->scrollMin.x);
	bool isVertScrollBarVisible = (scrollView->scrollMax.y > scrollView->scrollMin.y);
	
	scrollView->horiScrollGutterRec.height = UI_SCROLL_VIEW_GUTTER_WIDTH;
	scrollView->horiScrollGutterRec.x = scrollView->mainRec.x;
	scrollView->horiScrollGutterRec.y = (scrollView->mainRec.y + scrollView->mainRec.height) - scrollView->horiScrollGutterRec.height;
	scrollView->horiScrollGutterRec.width = scrollView->mainRec.width;
	RecAlign(&scrollView->horiScrollGutterRec);
	
	scrollView->vertScrollGutterRec.width = UI_SCROLL_VIEW_GUTTER_WIDTH;
	scrollView->vertScrollGutterRec.x = (scrollView->mainRec.x + scrollView->mainRec.width) - scrollView->vertScrollGutterRec.width;
	scrollView->vertScrollGutterRec.y = scrollView->mainRec.y;
	scrollView->vertScrollGutterRec.height = scrollView->mainRec.height;
	RecAlign(&scrollView->vertScrollGutterRec);
	
	if (isHoriScrollBarVisible && isVertScrollBarVisible)
	{
		scrollView->horiScrollGutterRec.width -= scrollView->vertScrollGutterRec.width;
		scrollView->vertScrollGutterRec.height -= scrollView->horiScrollGutterRec.height;
		RecAlign(&scrollView->horiScrollGutterRec);
		RecAlign(&scrollView->vertScrollGutterRec);
	}
	
	scrollView->usableRec = scrollView->mainRec;
	//TODO: Can we make these scroll bars actually disappear in the layout when they are not needed?
	scrollView->usableRec.width = scrollView->vertScrollGutterRec.x - scrollView->usableRec.x;
	scrollView->usableRec.height = scrollView->horiScrollGutterRec.y - scrollView->usableRec.y;
	RecAlign(&scrollView->usableRec);
	
	scrollView->horiScrollBarRec = scrollView->horiScrollGutterRec;
	r32 scrollableWidth = (scrollView->scrollMax.x - scrollView->scrollMin.x) + scrollView->usableRec.width;
	r32 horiScrollBarScale = scrollView->usableRec.width / scrollableWidth;
	scrollView->horiScrollBarRec.width *= horiScrollBarScale;
	r32 horiScrollBarMoveRoom = scrollView->horiScrollGutterRec.width - scrollView->horiScrollBarRec.width;
	r32 horiScrollBarScrollAmount = InverseLerpR32(scrollView->scrollMin.x, scrollView->scrollMax.x, scrollView->scroll.x, true);
	scrollView->horiScrollBarRec.x += horiScrollBarMoveRoom * horiScrollBarScrollAmount;
	RecAlign(&scrollView->horiScrollBarRec);
	
	scrollView->vertScrollBarRec = scrollView->vertScrollGutterRec;
	r32 scrollableHeight = (scrollView->scrollMax.y - scrollView->scrollMin.y) + scrollView->usableRec.height;
	r32 vertScrollBarScale = scrollView->usableRec.height / scrollableHeight;
	scrollView->vertScrollBarRec.height *= vertScrollBarScale;
	r32 vertScrollBarMoveRoom = scrollView->vertScrollGutterRec.height - scrollView->vertScrollBarRec.height;
	r32 vertScrollBarScrollAmount = InverseLerpR32(scrollView->scrollMin.y, scrollView->scrollMax.y, scrollView->scroll.y, true);
	scrollView->vertScrollBarRec.y += vertScrollBarMoveRoom * vertScrollBarScrollAmount;
	RecAlign(&scrollView->vertScrollBarRec);
}

void UpdateUiScrollViewContentSize(ScrollView_t* scrollView, ScrollViewContentSize_f* sizeFunc)
{
	NotNull(scrollView);
	if (sizeFunc != nullptr)
	{
		scrollView->contentRec = sizeFunc(scrollView);
	}
	scrollView->scrollMax = (scrollView->contentRec.topLeft + scrollView->contentRec.size) - scrollView->usableRec.size;
	scrollView->scrollMin = NewVec2(MaxR32(scrollView->contentRec.x, 0), MaxR32(scrollView->contentRec.y, 0));
	scrollView->scrollMax.x = MaxR32(scrollView->scrollMax.x, scrollView->scrollMin.x);
	scrollView->scrollMax.y = MaxR32(scrollView->scrollMax.y, scrollView->scrollMin.y);
	ClampUiScrollViewScroll(scrollView);
}

//We sort of expect you to call this every frame, even if you don't move mainRec, so the content gets a chance to update it's size/placement
void MoveUiScrollView(ScrollView_t* scrollView, rec mainRec, ScrollViewContentSize_f* sizeFunc)
{
	NotNull(scrollView);
	scrollView->mainRec = mainRec;
	RecAlign(&scrollView->mainRec);
	LayoutUiScrollView(scrollView);
	UpdateUiScrollViewContentSize(scrollView, sizeFunc);
}

void InitUiScrollView(ScrollView_t* scrollView, rec mainRec = Rec_Zero_Const, ScrollViewContentSize_f* sizeFunc = nullptr)
{
	NotNull(scrollView);
	ClearPointer(scrollView);
	scrollView->id = pig->nextUiId;
	pig->nextUiId++;
	scrollView->scrollLag = SCROLL_VIEW_DEFAULT_SCROLL_LAG;
	scrollView->scrollWheelDist = SCROLL_VIEW_DEFAULT_SCROLL_DIST;
	scrollView->contentRec = Rec_Zero;
	MoveUiScrollView(scrollView, mainRec, sizeFunc);
	//Start the scroll at the top left of the content
	scrollView->scroll = scrollView->scrollMin;
	scrollView->scrollGoto = scrollView->scroll;
}

void UiScrollViewCaptureMouse(ScrollView_t* scrollView, bool captureBackground = false)
{
	NotNull(scrollView);
	if (scrollView->scrollMax.x > scrollView->scrollMin.x)
	{
		MouseHitRecPrint(scrollView->horiScrollBarRec,    "ScrollView%lluHoriScrollBar",    scrollView->id);
		MouseHitRecPrint(scrollView->horiScrollGutterRec, "ScrollView%lluHoriScrollGutter", scrollView->id);
	}
	if (scrollView->scrollMax.y > scrollView->scrollMin.y)
	{
		MouseHitRecPrint(scrollView->vertScrollBarRec,    "ScrollView%lluVertScrollBar",    scrollView->id);
		MouseHitRecPrint(scrollView->vertScrollGutterRec, "ScrollView%lluVertScrollGutter", scrollView->id);
	}
	if (captureBackground)
	{
		MouseHitRecPrint(scrollView->mainRec, "ScrollView%llu", scrollView->id);
	}
}

void UpdateUiScrollView(ScrollView_t* scrollView, bool overrideIsMouseOver = false)
{
	NotNull(scrollView);
	LayoutUiScrollView(scrollView);
	
	// +==============================+
	// |      Handle Mouse Wheel      |
	// +==============================+
	if (IsMouseOverPrintPartial("ScrollView%llu", scrollView->id) || overrideIsMouseOver)
	{
		if (MouseScrolledX())
		{
			HandleMouseScrollX();
			scrollView->scrollGoto.x += (pigIn->scrollDelta.x * scrollView->scrollWheelDist);
		}
		if (MouseScrolledY())
		{
			HandleMouseScrollY();
			scrollView->scrollGoto.y -= (pigIn->scrollDelta.y * scrollView->scrollWheelDist);
		}
	}
	
	// +================================================+
	// | Handle Horizontal Scrollbar Mouse Interaction  |
	// +================================================+
	if (scrollView->horiScrollBarGrabbed || IsMouseOverPrint("ScrollView%lluHoriScrollBar", scrollView->id)) { UpdateAnimationUp(&scrollView->horiScrollBarHighlightAnim, UI_SCROLL_VIEW_SCROLLBAR_HIGHLIGHT_TIME); }
	else { UpdateAnimationDown(&scrollView->horiScrollBarHighlightAnim, UI_SCROLL_VIEW_SCROLLBAR_HIGHLIGHT_TIME); }
	if (scrollView->horiScrollBarGrabbed)
	{
		if (MouseDownRaw(MouseBtn_Left))
		{
			HandleMouse(MouseBtn_Left);
			r32 scrollableAmount = scrollView->horiScrollGutterRec.width - scrollView->horiScrollBarRec.width;
			r32 newBarX = (MousePos.x - scrollView->horiScrollBarGrabOffset.x) - scrollView->horiScrollGutterRec.x;
			r32 newScrollPercentage = ClampR32(newBarX / scrollableAmount, 0.0f, 1.0f);
			scrollView->scrollGoto.x = LerpR32(scrollView->scrollMin.x, scrollView->scrollMax.x, newScrollPercentage);
		}
		else
		{
			HandleMouse(MouseBtn_Left);
			scrollView->horiScrollBarGrabbed = false;
		}
	}
	else if (IsMouseOverPrint("ScrollView%lluHoriScrollBar", scrollView->id))
	{
		if (MousePressed(MouseBtn_Left))
		{
			HandleMouse(MouseBtn_Left);
			scrollView->horiScrollBarGrabbed = true;
			scrollView->horiScrollBarGrabOffset = MousePos - scrollView->horiScrollBarRec.topLeft;
		}
	}
	
	// +==============================================+
	// | Handle Vertical Scrollbar Mouse Interaction  |
	// +==============================================+
	if (scrollView->vertScrollBarGrabbed || IsMouseOverPrint("ScrollView%lluVertScrollBar", scrollView->id)) { UpdateAnimationUp(&scrollView->vertScrollBarHighlightAnim, UI_SCROLL_VIEW_SCROLLBAR_HIGHLIGHT_TIME); }
	else { UpdateAnimationDown(&scrollView->vertScrollBarHighlightAnim, UI_SCROLL_VIEW_SCROLLBAR_HIGHLIGHT_TIME); }
	if (scrollView->vertScrollBarGrabbed)
	{
		if (MouseDownRaw(MouseBtn_Left))
		{
			HandleMouse(MouseBtn_Left);
			r32 scrollableAmount = scrollView->vertScrollGutterRec.height - scrollView->vertScrollBarRec.height;
			r32 newBarY = (MousePos.y - scrollView->vertScrollBarGrabOffset.y) - scrollView->vertScrollGutterRec.y;
			r32 newScrollPercentage = ClampR32(newBarY / scrollableAmount, 0.0f, 1.0f);
			scrollView->scrollGoto.y = LerpR32(scrollView->scrollMin.y, scrollView->scrollMax.y, newScrollPercentage);
		}
		else
		{
			HandleMouse(MouseBtn_Left);
			scrollView->vertScrollBarGrabbed = false;
		}
	}
	else if (IsMouseOverPrint("ScrollView%lluVertScrollBar", scrollView->id))
	{
		if (MousePressed(MouseBtn_Left))
		{
			HandleMouse(MouseBtn_Left);
			scrollView->vertScrollBarGrabbed = true;
			scrollView->vertScrollBarGrabOffset = MousePos - scrollView->vertScrollBarRec.topLeft;
		}
	}
	
	// +==============================+
	// |        Update Scroll         |
	// +==============================+
	if (scrollView->scroll != scrollView->scrollGoto)
	{
		v2 delta = (scrollView->scrollGoto - scrollView->scroll);
		v2 oldScroll = scrollView->scroll;
		
		if (AbsR32(delta.x) > SCROLL_VIEW_MIN_DELTA)
		{
			scrollView->scroll.x += delta.x / scrollView->scrollLag;
		}
		else
		{
			scrollView->scroll.x = scrollView->scrollGoto.x;
		}
		
		if (AbsR32(delta.y) > SCROLL_VIEW_MIN_DELTA)
		{
			scrollView->scroll.y += delta.y / scrollView->scrollLag;
		}
		else
		{
			scrollView->scroll.y = scrollView->scrollGoto.y;
		}
		
		scrollView->scrollChanged = true;
		scrollView->scrollDelta = scrollView->scroll - oldScroll;
	}
	else { scrollView->scrollDelta = Vec2_Zero; }
	
	ClampUiScrollViewScroll(scrollView);
}

void RenderUiScrollView(ScrollView_t* scrollView, Color_t scrollGutterColor, Color_t scrollBarColor, Color_t scrollBarHighlightColor)
{
	LayoutUiScrollView(scrollView);
	bool isHoriScrollBarVisible = (scrollView->scrollMax.x > scrollView->scrollMin.x);
	bool isVertScrollBarVisible = (scrollView->scrollMax.y > scrollView->scrollMin.y);
	
	rec oldViewport = RcAndViewport(scrollView->mainRec);
	
	if (isHoriScrollBarVisible)
	{
		Color_t barColor = ColorLerp(scrollBarColor, scrollBarHighlightColor, scrollView->horiScrollBarGrabbed ? 1.0f : (scrollView->horiScrollBarHighlightAnim * 0.75f));
		RcDrawRectangle(scrollView->horiScrollGutterRec, scrollGutterColor);
		RcDrawRectangle(scrollView->horiScrollBarRec, barColor);
	}
	if (isVertScrollBarVisible)
	{
		Color_t barColor = ColorLerp(scrollBarColor, scrollBarHighlightColor, scrollView->vertScrollBarGrabbed ? 1.0f : (scrollView->vertScrollBarHighlightAnim * 0.75f));
		RcDrawRectangle(scrollView->vertScrollGutterRec, scrollGutterColor);
		RcDrawRectangle(scrollView->vertScrollBarRec, barColor);
	}
	if (isHoriScrollBarVisible && isVertScrollBarVisible)
	{
		rec cornerRec = NewRec(
			scrollView->vertScrollGutterRec.x,
			scrollView->vertScrollGutterRec.y + scrollView->vertScrollGutterRec.height,
			scrollView->vertScrollGutterRec.width,
			scrollView->horiScrollGutterRec.height
		);
		RecAlign(&cornerRec);
		RcDrawRectangle(cornerRec, scrollBarColor);
	}
	
	RcSetViewport(oldViewport);
}
