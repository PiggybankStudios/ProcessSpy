/*
File:   ui_scroll_view.h
Author: Taylor Robbins
Date:   07\07\2024
*/

#ifndef _UI_SCROLL_VIEW_H
#define _UI_SCROLL_VIEW_H

#define SCROLL_VIEW_MIN_DELTA            0.1f //px
#define SCROLL_VIEW_DEFAULT_SCROLL_LAG   5 //divisor
#define SCROLL_VIEW_DEFAULT_SCROLL_DIST  50 //px/notch

#define SCROLL_VIEW_CONTENT_SIZE_DEFINITION(functionName) rec functionName(struct ScrollView_t* scrollView)
typedef SCROLL_VIEW_CONTENT_SIZE_DEFINITION(ScrollViewContentSize_f);

struct ScrollView_t
{
	u64 id;
	void* contextPntr;
	rec contentRec;
	v2 scrollMin;
	v2 scrollMax;
	r32 scrollLag;
	r32 scrollWheelDist; //px/notch
	
	v2 scroll;
	v2 scrollGoto;
	bool horiScrollBarGrabbed;
	r32 horiScrollBarHighlightAnim;
	v2 horiScrollBarGrabOffset;
	bool vertScrollBarGrabbed;
	r32 vertScrollBarHighlightAnim;
	v2 vertScrollBarGrabOffset;
	
	bool scrollChanged;
	v2 scrollDelta; //amount moved this frame
	
	rec mainRec;
	rec usableRec; //mainRec - scrollGutterRec
	rec horiScrollBarRec;
	rec horiScrollGutterRec;
	rec vertScrollBarRec;
	rec vertScrollGutterRec;
};

#endif //  _UI_SCROLL_VIEW_H
