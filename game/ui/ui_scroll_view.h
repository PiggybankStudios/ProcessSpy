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
	void* contextPntr;
	rec contentRec;
	v2 scrollMin;
	v2 scrollMax;
	r32 scrollLag;
	r32 scrollWheelDist; //px/notch
	
	v2 scroll;
	v2 scrollGoto;
	
	bool scrollChanged;
	v2 scrollDelta; //amount moved this frame
	
	rec mainRec;
	rec usableRec; //mainRec - scrollGutterRec
	rec vertScrollBarRec;
	rec vertScrollGutterRec;
	rec horiScrollBarRec;
	rec horiScrollGutterRec;
};

#endif //  _UI_SCROLL_VIEW_H
