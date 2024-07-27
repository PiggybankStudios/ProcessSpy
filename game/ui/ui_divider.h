/*
File:   ui_divider.h
Author: Taylor Robbins
Date:   07\25\2024
*/

#ifndef _UI_DIVIDER_H
#define _UI_DIVIDER_H

struct UiDivider_t
{
	u64 id;
	
	bool isHorizontal;
	bool isProportional;
	r32 thickness;
	r32 hitThickness;
	union { r32 minLeftSizePx;       r32 minTopSizePx;         };
	union { r32 minLeftSizePercent;  r32 minTopSizePercent;    };
	union { r32 minRightSizePx;      r32 minBottomSizePx;      };
	union { r32 minRightSizePercent; r32 minBottomSizePercent; };
	
	r32 split; //holds the size of the left/top portion (as percentage or pixels, depending on isProportional)
	r32 targetSplit; //set by the user, but restrictions might make actual split differ
	bool splitChanged;
	
	bool isMouseDragging;
	r32 mouseDraggingOffset;
	
	rec mainRec;
	//The following are all relative to mainRec.topLeft
	rec dividerRec;
	rec dividerHitRec;
	union { rec leftRec; rec topRec; };
	union { rec rightRec; rec bottomRec; };
};

#endif //  _UI_DIVIDER_H
