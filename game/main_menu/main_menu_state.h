/*
File:   main_menu_state.h
Author: Taylor Robbins
Date:   08\13\2022
*/

#ifndef _MAIN_MENU_STATE_H
#define _MAIN_MENU_STATE_H

#define MMENU_TITLE_TEXT  "Pig Parts"

#define MMENU_BTN_MARGIN                NewVec2(20, 40)
#define MMENU_BTN_PADDING               5
#define MMENU_BTN_SIZE                  NewVec2(380, 60)
#define MMENU_BTN_SND_DEDUPLICATE_TIME  100 //ms

#define MMENU_FADE_IN_ANIM_TIME     3000 //ms
#define MMENU_MUSIC_FADE_TIME       1000 //ms
#define MMENU_LOGO_ANIM_TIME        1000 //ms
#define MMENU_LOGO_CLICK_ANIM_TIME  1000 //ms

enum MainMenuBtn_t
{
	MainMenuBtn_Play = 0,
	MainMenuBtn_Settings,
	MainMenuBtn_Exit,
	MainMenuBtn_NumButtons,
};
const char* GetMainMenuBtnStr(MainMenuBtn_t mainMenuBtn)
{
	switch (mainMenuBtn)
	{
		case MainMenuBtn_Play:     return "Play";
		case MainMenuBtn_Settings: return "Settings";
		case MainMenuBtn_Exit:     return "Exit";
		default: return "Unknown";
	}
}
const char* GetMainMenuBtnDisplayStr(MainMenuBtn_t mainMenuBtn)
{
	switch (mainMenuBtn)
	{
		case MainMenuBtn_Play:     return "Play";
		case MainMenuBtn_Settings: return "Settings";
		case MainMenuBtn_Exit:     return "Exit";
		default: return "Unknown";
	}
}

struct MainMenuAppState_t
{
	bool initialized;
	
	r32 pigLogoAnimProgress;
	rec pigLogoRec;
	r32 pigLogoClickAnimProgress;
	u64 pigClickCount;
	
	rec btnsStackRec;
	rec btnRecs[MainMenuBtn_NumButtons];
	r32 fadeInAnimProgress; //TODO: move me
	u64 lastBtnSoundTime;
	i64 mouseHoverBtnIndex;
	i64 previousMouseHoverBtnIndex;
	
	r32 backgroundScale;
	rec backgroundRec;
};

#endif //  _MAIN_MENU_STATE_H
