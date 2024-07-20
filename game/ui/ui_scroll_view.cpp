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

void ClampScrollViewScroll(ScrollView_t* scrollView)
{
	NotNull(scrollView);
	scrollView->scroll.x = ClampR32(scrollView->scroll.x, scrollView->scrollMin.x, scrollView->scrollMax.x);
	scrollView->scroll.y = ClampR32(scrollView->scroll.y, scrollView->scrollMin.y, scrollView->scrollMax.y);
	scrollView->scrollGoto.x = ClampR32(scrollView->scrollGoto.x, scrollView->scrollMin.x, scrollView->scrollMax.x);
	scrollView->scrollGoto.y = ClampR32(scrollView->scrollGoto.y, scrollView->scrollMin.y, scrollView->scrollMax.y);
}

void UpdateScrollViewContentSize(ScrollView_t* scrollView, ScrollViewContentSize_f* sizeFunc)
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
	ClampScrollViewScroll(scrollView);
}

void InitUiScrollView(ScrollView_t* scrollView, rec mainRec = Rec_Zero_Const, ScrollViewContentSize_f* sizeFunc = nullptr)
{
	NotNull(scrollView);
	ClearPointer(scrollView);
	scrollView->scrollLag = SCROLL_VIEW_DEFAULT_SCROLL_LAG;
	scrollView->scrollWheelDist = SCROLL_VIEW_DEFAULT_SCROLL_DIST;
	scrollView->mainRec = mainRec;
	scrollView->usableRec = scrollView->mainRec;
	scrollView->contentRec = Rec_Zero;
	UpdateScrollViewContentSize(scrollView, sizeFunc);
	//Start the scroll at the top left of the content
	scrollView->scroll = scrollView->scrollMin;
	scrollView->scrollGoto = scrollView->scroll;
}

//We sort of expect you to call this every frame, even if you don't move mainRec, so the content gets a chance to update it's size/placement
void UpdateScrollViewMainRec(ScrollView_t* scrollView, rec mainRec, ScrollViewContentSize_f* sizeFunc)
{
	NotNull(scrollView);
	scrollView->mainRec = mainRec;
	scrollView->usableRec = scrollView->mainRec;
	UpdateScrollViewContentSize(scrollView, sizeFunc);
}

void UpdateScrollView(ScrollView_t* scrollView, bool isMouseInside)
{
	NotNull(scrollView);
	
	// +==============================+
	// |      Handle Mouse Wheel      |
	// +==============================+
	if (isMouseInside)
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
	
	// +==============================+
	// |        Update Scroll         |
	// +==============================+
	if (scrollView->scroll != scrollView->scrollGoto)
	{
		v2 delta = (scrollView->scrollGoto - scrollView->scroll);
		
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
	}
	
	ClampScrollViewScroll(scrollView);
}
