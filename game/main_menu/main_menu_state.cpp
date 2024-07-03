/*
File:   main_menu_state.cpp
Author: Taylor Robbins
Date:   08\13\2022
Description: 
	** Controls the main menu that first appears when the game opens and has buttons leading to singleplayer, multiplayer, settings, etc
*/

// +--------------------------------------------------------------+
// |                       Access Resources                       |
// +--------------------------------------------------------------+
void MainMenuAppStateAccessResources()
{
	AccessResource(&pig->resources.textures->mainMenuBackground);
	AccessResource(&pig->resources.textures->madeInPigEngine);
	AccessResource(&pig->resources.musics->testMusic);
	AccessResource(&pig->resources.sounds->oink);
	AccessResource(&pig->resources.sounds->click1);
}

// +--------------------------------------------------------------+
// |                            Start                             |
// +--------------------------------------------------------------+
void StartMainMenuAppState(AppState_t oldAppState, bool initialize)
{
	NotNull(mmenu);
	
	if (initialize)
	{
		ClearPointer(mmenu);
		
		mmenu->fadeInAnimProgress = 0.0f;
		mmenu->pigLogoAnimProgress = -0.6f;
		mmenu->lastBtnSoundTime = 0;
		
		mmenu->initialized = true;
	}
	
	MainMenuAppStateAccessResources();
	SoftPlayMusic(&pig->resources.musics->testMusic, MusicFade_LinearCrossfade, MMENU_MUSIC_FADE_TIME);
	mmenu->previousMouseHoverBtnIndex = -1;
	
	if (oldAppState == AppState_None)
	{
		//TODO: Implement me!
	}
}

// +--------------------------------------------------------------+
// |                             Stop                             |
// +--------------------------------------------------------------+
void StopMainMenuAppState(AppState_t newAppState, bool deinitialize, bool shuttingDown)
{
	UNUSED(newAppState);
	NotNull(mmenu);
	
	if (deinitialize && !shuttingDown)
	{
		//TODO: Implement me!
		
		ClearPointer(mmenu);
	}
}

// +--------------------------------------------------------------+
// |                   Layout and CaptureMouse                    |
// +--------------------------------------------------------------+
void LayoutMainMenuAppState()
{
	mmenu->backgroundRec.size = pig->resources.textures->mainMenuBackground.size;
	mmenu->backgroundScale = MaxR32(ScreenSize.width / mmenu->backgroundRec.width, ScreenSize.height / mmenu->backgroundRec.height);
	mmenu->backgroundRec.size = mmenu->backgroundRec.size * mmenu->backgroundScale;
	mmenu->backgroundRec.x = ScreenSize.width/2 - mmenu->backgroundRec.width/2;
	mmenu->backgroundRec.y = ScreenSize.height/2 - mmenu->backgroundRec.height/2;
	RecAlign(&mmenu->backgroundRec);
	
	mmenu->btnsStackRec.size = Vec2_Zero;
	for (u64 bIndex = 0; bIndex < MainMenuBtn_NumButtons; bIndex++)
	{
		rec* btnRec = &mmenu->btnRecs[bIndex];
		btnRec->size = MMENU_BTN_SIZE;
		if (btnRec->height > 0)
		{
			if (mmenu->btnsStackRec.width < btnRec->width) { mmenu->btnsStackRec.width = btnRec->width; }
			if (bIndex > 0) { mmenu->btnsStackRec.height += MMENU_BTN_PADDING; }
			mmenu->btnsStackRec.height += btnRec->height;
		}
	}
	mmenu->btnsStackRec.x = ScreenSize.width/2 - mmenu->btnsStackRec.width/2;
	mmenu->btnsStackRec.y = ScreenSize.height - MMENU_BTN_MARGIN.y - mmenu->btnsStackRec.height;
	if (mmenu->btnsStackRec.y < 0) { mmenu->btnsStackRec.y = 0; }
	// mmenu->btnsStackRec.y += (ScreenSize.height - mmenu->btnsStackRec.y) * (1 - EaseQuadraticOut(mmenu->fadeInAnimProgress));
	RecAlign(&mmenu->btnsStackRec);
	
	v2 btnPos = mmenu->btnsStackRec.topLeft;
	for (u64 bIndex = 0; bIndex < MainMenuBtn_NumButtons; bIndex++)
	{
		rec* btnRec = &mmenu->btnRecs[bIndex];
		btnRec->topLeft = btnPos;
		RecAlign(btnRec);
		if (btnRec->height > 0) { btnPos.y += btnRec->height + MMENU_BTN_PADDING; }
	}
	
	r32 pigLogoScale = 1.0f;
	mmenu->pigLogoRec.size = pig->resources.textures->madeInPigEngine.size * pigLogoScale;
	mmenu->pigLogoRec.x = ScreenSize.width - 10 - mmenu->pigLogoRec.width;
	mmenu->pigLogoRec.y = ScreenSize.height - 10 - mmenu->pigLogoRec.height;
	mmenu->pigLogoRec.y += (ScreenSize.height - mmenu->pigLogoRec.y) * (1 - EaseBackOut(mmenu->pigLogoAnimProgress));
	RecAlign(&mmenu->pigLogoRec);
}

void CaptureMouseMainMenuAppState()
{
	for (u64 bIndex = 0; bIndex < MainMenuBtn_NumButtons; bIndex++)
	{
		MainMenuBtn_t btn = (MainMenuBtn_t)bIndex;
		rec btnRec = mmenu->btnRecs[bIndex];
		MouseHitRecPrint(btnRec, "%sBtn", GetMainMenuBtnStr(btn));
	}
	MouseHitRecNamed(mmenu->pigLogoRec, "PigLogo");
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void UpdateMainMenuAppState()
{
	NotNull(mmenu);
	MainMenuAppStateAccessResources();
	LayoutMainMenuAppState();
	CaptureMouseMainMenuAppState();
	
	// +==============================+
	// |        Key_R to Reset        |
	// +==============================+
	if (KeyPressed(Key_R))
	{
		HandleKeyExtended(Key_R);
		mmenu->fadeInAnimProgress = 0.0f;
		mmenu->pigLogoAnimProgress = -0.6f;
		mmenu->pigLogoClickAnimProgress = 0.0f;
		mmenu->pigClickCount = 0;
		PlayMusic(&pig->resources.musics->testMusic);
	}
	
	// +==============================+
	// |   Button Mouse Interaction   |
	// +==============================+
	bool buttonPressed = false;
	MainMenuBtn_t buttonPressedBtn = MainMenuBtn_NumButtons;
	mmenu->mouseHoverBtnIndex = -1;
	for (u64 bIndex = 0; bIndex < MainMenuBtn_NumButtons; bIndex++)
	{
		MainMenuBtn_t btn = (MainMenuBtn_t)bIndex;
		// rec btnRec = mmenu->btnRecs[bIndex];
		if (IsMouseOverPrint("%sBtn", GetMainMenuBtnStr(btn)))
		{
			pigOut->cursorType = PlatCursor_Pointer;
			mmenu->mouseHoverBtnIndex = (i64)bIndex;
			if (MousePressedAndHandleExtended(MouseBtn_Left))
			{
				PrintLine_D("Player clicked %s btn...", GetMainMenuBtnStr(btn));
				buttonPressed = true;
				buttonPressedBtn = btn;
			}
		}
	}
	
	// +==============================+
	// |     Handle Button Logic      |
	// +==============================+
	if (buttonPressed)
	{
		switch (buttonPressedBtn)
		{
			// +==============================+
			// |   Handle Play Btn Clicked    |
			// +==============================+
			// case MainMenuBtn_Play:
			// {
			// 	PushAppState(AppState_LevelSelect); //TODO: Make this go to some AppState!
			// } break;
			// +==============================+
			// | Handle Settings Btn Clicked  |
			// +==============================+
			// case MainMenuBtn_Settings:
			// {
			// 	PushAppState(AppState_SettingsMenu);
			// } break;
			// +==============================+
			// |   Handle Exit Btn Clicked    |
			// +==============================+
			case MainMenuBtn_Exit:
			{
				WriteLine_W("Player clicked exit main menu button. Closing the game! Goodbye!");
				pigOut->exit = true;
			} break;
			// +==============================+
			// |  Handle Unknown Btn Clicked  |
			// +==============================+
			default:
			{
				NotifyWrite_E("This button has not been implemented yet!");
			} break;
		}
	}
	
	// +==============================+
	// |   Play Button Hover Sound    |
	// +==============================+
	if (mmenu->mouseHoverBtnIndex != mmenu->previousMouseHoverBtnIndex)
	{
		if (mmenu->lastBtnSoundTime == 0 || TimeSince(mmenu->lastBtnSoundTime) >= MMENU_BTN_SND_DEDUPLICATE_TIME)
		{
			PlaySound(&pig->resources.sounds->click1, 0.7f);
		}
		if (mmenu->mouseHoverBtnIndex == -1) { mmenu->lastBtnSoundTime = ProgramTime; }
		else { mmenu->lastBtnSoundTime = 0; }
	}
	mmenu->previousMouseHoverBtnIndex = mmenu->mouseHoverBtnIndex;
	
	// +==============================+
	// |    Handle PigLogo Clicked    |
	// +==============================+
	if (IsMouseOverNamed("PigLogo"))
	{
		if (MousePressedAndHandleExtended(MouseBtn_Left))
		{
			mmenu->pigLogoClickAnimProgress = 1.0f;
			PlaySound(&pig->resources.sounds->oink, 0.4f);
			mmenu->pigClickCount++;
		}
	}
	
	// +==============================+
	// |      Update Animations       |
	// +==============================+
	UpdateAnimationUp(&mmenu->fadeInAnimProgress, MMENU_FADE_IN_ANIM_TIME);
	UpdateAnimationUp(&mmenu->pigLogoAnimProgress, MMENU_LOGO_ANIM_TIME);
	UpdateAnimationDown(&mmenu->pigLogoClickAnimProgress, MMENU_LOGO_CLICK_ANIM_TIME);
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void RenderMainMenuAppState(FrameBuffer_t* renderBuffer, bool bottomLayer)
{
	UNUSED(bottomLayer);
	NotNull(mmenu);
	MainMenuAppStateAccessResources();
	LayoutMainMenuAppState();
	
	RcBegin(pig->currentWindow, renderBuffer, &pig->resources.shaders->main2D, Black);
	// RcClearColor(Grey3); //TODO: Remove me!
	// return; //TODO: Remove me!
	
	// +==============================+
	// |      Render Background       |
	// +==============================+
	RcBindTexture1(&pig->resources.textures->mainMenuBackground);
	r32 mainMenuBrightness = EaseQuadraticInOut(SubAnimAmountR32(mmenu->fadeInAnimProgress, 0.0f, 1.0f));
	RcDrawTexturedRectangle(mmenu->backgroundRec, ColorLerp(Black, Grey11, mainMenuBrightness));
	
	// +==============================+
	// |    Render Pig Engine Logo    |
	// +==============================+
	RcBindTexture1(&pig->resources.textures->madeInPigEngine);
	obb2 pigLogoObb = ToObb2D(mmenu->pigLogoRec);
	pigLogoObb.rotation = EaseBackInOut(mmenu->pigLogoClickAnimProgress) * TwoPi32;
	RcDrawTexturedObb2(pigLogoObb, White);
	if (mmenu->pigClickCount >= 10)
	{
		RcBindFont(&pig->resources.fonts->pixel, SelectDefaultFontFace(), 2.0f);
		MyStr_t clickCountStr = TempPrintStr("You have now clicked the pig %llu times...", mmenu->pigClickCount);
		v2 clickCountPos = NewVec2(
			mmenu->pigLogoRec.x + mmenu->pigLogoRec.width,
			mmenu->pigLogoRec.y - 4 - RcGetMaxDescend()
		);
		RcDrawText(clickCountStr, clickCountPos, White, TextAlignment_Right);
	}
	
	// +==============================+
	// |        Render Buttons        |
	// +==============================+
	r32 btnsAlpha = SubAnimAmountR32(mmenu->fadeInAnimProgress, 0.2f, 0.4f);
	Color_t btnsBackColor = MonokaiWhite;
	Color_t btnsTextColor = ColorTransparent(Black, btnsAlpha);
	// FontFaceSelector_t SelectFontFace(i32 fontSize, bool bold = false, bool italic = false)
	RcBindFont(&pig->resources.fonts->large, SelectFontFace(48, true));
	for (u64 bIndex = 0; bIndex < MainMenuBtn_NumButtons; bIndex++)
	{
		MainMenuBtn_t btn = (MainMenuBtn_t)bIndex;
		rec btnRec = mmenu->btnRecs[bIndex];
		bool isHovered = IsMouseOverPrint("%sBtn", GetMainMenuBtnStr(btn));
		MyStr_t buttonText = NewStr(GetMainMenuBtnDisplayStr(btn));
		v2 btnTextPos = NewVec2(btnRec.x + btnRec.width/2, btnRec.y + btnRec.height/2 - RcGetLineHeight()/2 + RcGetMaxAscend() - 5);
		Vec2Align(&btnTextPos);
		if (btnRec.height > 0)
		{
			RcDrawRoundedRectangle(btnRec, 10.0f, ColorTransparent(btnsBackColor, btnsAlpha * (isHovered ? 0.95f: 0.7f)));
			RcDrawText(buttonText, btnTextPos, btnsTextColor, TextAlignment_Center);
		}
	}
	
	// +==============================+
	// | Render Assertion Status Text |
	// +==============================+
	if (true)
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
}
