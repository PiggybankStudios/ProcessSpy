/*
File:   game_startup_options.cpp
Author: Taylor Robbins
Date:   01\31\2022
Description: 
	** Holds the GetStartupOptions function which provides the platform with a bunch of choices about what we want from it
	** that may change from project to project or based on files on the user's computer that need to be parsed by the game code
*/

#define GAME_WINDOW_TITLE                    PROJECT_NAME //comes from build.bat
#define GAME_WINDOW_ALWAYS_ON_TOP            false
#define GAME_WINDOW_DEFAULT_RESOLUTION       NewVec2i(1600, 900)
#define GAME_FOLLOW_SAVED_SETTING_RESOLUTION true
#define GAME_WINDOW_MIN_SIZE                 NewVec2i(200, 100)
#define GAME_ANTI_ALIASING_SAMPLES           4
#define GAME_THREAD_POOL_SIZE                4 //threads
#define GAME_FIXED_HEAP_MEM_SIZE             Kilobytes(1024)
#define GAME_TEMP_ARENA_SIZE                 Megabytes(16)
#define GAME_THREAD_TEMP_ARENA_SIZE          Kilobytes(128)
#define GAME_THREAD_TEMP_NUM_MARKS           32 //marks
#define GAME_THREAD_SCRATCH_ARENA_MAX_SIZE   Gigabytes(1)
#define GAME_THREAD_SCRATCH_NUM_MARKS        32 //marks
#define GAME_SETTINGS_FILE_NAME              "settings.txt"
#define GAME_DBG_BINDINGS_FILE_NAME          "debug_bindings.txt"
#define GAME_INITIAL_APP_STATE               AppState_Main

#define GAME_DEFAULT_MASTER_VOLUME     0.8f
#define GAME_DEFAULT_MUSIC_VOLUME      1.0f
#define GAME_DEFAULT_SOUNDS_VOLUME     1.0f
#define GAME_DEFAULT_MOUSE_SENSITIVITY 5.0f

void GameGetStartupOptions(StartupOptions_t* options)
{
	//TODO: Assertions in here are unclear to the user. Try to limit failure cases as much as possible
	//TODO: We need some way to relay errors/warnings to the application from here!
	NotNull(startup);
	NotNull(options);
	
	bool savedFullscreen = false;
	v2i savedResolution = GAME_WINDOW_DEFAULT_RESOLUTION;
	MyStr_t savedMonitorName = MyStr_Empty;
	u64 savedMonitorNumber = 0;
	const PlatMonitorInfo_t* savedMonitorInfo = nullptr;
	u64 savedFramerate = PIG_DEFAULT_FRAMERATE;
	v2i savedWindowedPosition = NewVec2i(-1, -1);
	bool savedMaximizedWindow = false;
	bool makeTopmostWindow = GAME_WINDOW_ALWAYS_ON_TOP;
	
	if (startup->GetProgramArg(startup->platTempArena, NewStr("top"), nullptr))
	{
		makeTopmostWindow = true;
	}
	
	// +==============================+
	// |        Load Settings         |
	// +==============================+
	MyStr_t settingsFilePath = PigGetSettingsFilePath(startup->platTempArena, startup->platTempArena, NewStr(PROJECT_NAME_SAFE), NewStr(GAME_SETTINGS_FILE_NAME), startup->GetSpecialFolderPath);
	if (settingsFilePath.length > 0)
	{
		if (startup->DoesFileExist(settingsFilePath, nullptr))
		{
			PigSettings_t settings = {};
			ProcessLog_t settingsParseLog;
			CreateProcessLogStub(&settingsParseLog);
			// settingsParseLog.debugBreakOnWarningsAndErrors = true;
			if (PigTryLoadSettings(settingsFilePath, &settingsParseLog, &settings, startup->platTempArena, startup->ReadFileContents, startup->FreeFileContents))
			{
				#if GAME_FOLLOW_SAVED_SETTING_RESOLUTION
				TryParseFailureReason_t parseFailureReason = TryParseFailureReason_None;
				if (PigTryGetSettingV2i(&settings, "Resolution", &savedResolution, true, &parseFailureReason) == TryGetSettingResult_ParseError)
				{
					MyStr_t settingValueStr = PigGetSettingStr(&settings, "Resolution", MyStr_Empty);
					startup->DebugOutput(PrintInArenaStr(startup->platTempArena, "Resolution setting has invalid (unparsable) value: \"%.*s\" error: %s", StrPrint(settingValueStr), GetTryParseFailureReasonStr(parseFailureReason)), true);
				}
				if (PigTryGetSettingBool(&settings, "Fullscreen", &savedFullscreen, true, &parseFailureReason) == TryGetSettingResult_ParseError)
				{
					MyStr_t settingValueStr = PigGetSettingStr(&settings, "Fullscreen", MyStr_Empty);
					startup->DebugOutput(PrintInArenaStr(startup->platTempArena, "Fullscreen setting has invalid (unparsable) value: \"%.*s\" error: %s", StrPrint(settingValueStr), GetTryParseFailureReasonStr(parseFailureReason)), true);
				}
				if (PigTryGetSettingU64(&settings, "Framerate", &savedFramerate, true, &parseFailureReason) == TryGetSettingResult_ParseError)
				{
					MyStr_t settingValueStr = PigGetSettingStr(&settings, "Framerate", MyStr_Empty);
					startup->DebugOutput(PrintInArenaStr(startup->platTempArena, "Framerate setting has invalid (unparsable) value: \"%.*s\" error: %s", StrPrint(settingValueStr), GetTryParseFailureReasonStr(parseFailureReason)), true);
				}
				PigTryGetSettingStr(&settings, "Monitor", &savedMonitorName, true);
				if (PigTryGetSettingU64(&settings, "MonitorNumber", &savedMonitorNumber, true, &parseFailureReason) == TryGetSettingResult_ParseError)
				{
					MyStr_t settingValueStr = PigGetSettingStr(&settings, "MonitorNumber", MyStr_Empty);
					startup->DebugOutput(PrintInArenaStr(startup->platTempArena, "MonitorNumber setting has invalid (unparsable) value: \"%.*s\" error: %s", StrPrint(settingValueStr), GetTryParseFailureReasonStr(parseFailureReason)), true);
				}
				if (PigTryGetSettingBool(&settings, "MaximizedWindow", &savedMaximizedWindow, true, &parseFailureReason) == TryGetSettingResult_ParseError)
				{
					MyStr_t settingValueStr = PigGetSettingStr(&settings, "MaximizedWindow", MyStr_Empty);
					startup->DebugOutput(PrintInArenaStr(startup->platTempArena, "MaximizedWindow setting has invalid (unparsable) value: \"%.*s\" error: %s", StrPrint(settingValueStr), GetTryParseFailureReasonStr(parseFailureReason)), true);
				}
				if (PigTryGetSettingV2i(&settings, "WindowedPosition", &savedWindowedPosition, true, &parseFailureReason) == TryGetSettingResult_ParseError)
				{
					MyStr_t settingValueStr = PigGetSettingStr(&settings, "WindowedPosition", MyStr_Empty);
					startup->DebugOutput(PrintInArenaStr(startup->platTempArena, "WindowedPosition setting has invalid (unparsable) value: \"%.*s\" error: %s", StrPrint(settingValueStr), GetTryParseFailureReasonStr(parseFailureReason)), true);
				}
				#endif
			}
			else
			{
				startup->DebugOutput(
					PrintInArenaStr(startup->platTempArena, "Failed to parse settings file at \"%.*s\": %s",
						StrPrint(settingsFilePath),
						GetResultStr((Result_t)settingsParseLog.errorCode)),
					true
				);
			}
		}
		else
		{
			startup->DebugOutput(PrintInArenaStr(startup->platTempArena, "No settings file found at \"%.*s\"", StrPrint(settingsFilePath)), true);
		}
	}
	else
	{
		startup->DebugOutput(NewStr("Failed to get special folder path from platform/OS. We can't load settings!"), true);
	}
	
	// +==============================+
	// |      Find Saved Monitor      |
	// +==============================+
	if (!IsEmptyStr(savedMonitorName) || savedMonitorNumber != 0)
	{
		const PlatMonitorInfo_t* monitorInfo = LinkedListFirst(&startup->monitors->list, PlatMonitorInfo_t);
		for (u64 mIndex = 0; mIndex < startup->monitors->list.count; mIndex++)
		{
			if ((IsEmptyStr(savedMonitorName) || StrEquals(savedMonitorName, monitorInfo->name)) &&
				(savedMonitorNumber == 0 || monitorInfo->designatedNumber == savedMonitorNumber))
			{
				savedMonitorInfo = monitorInfo;
				break;
			}
			monitorInfo = LinkedListNext(&startup->monitors->list, PlatMonitorInfo_t, monitorInfo);
		}
		if (savedMonitorInfo == nullptr)
		{
			startup->DebugOutput(NewStr("The monitor we were on when we last closed either doesn't exist or changed name/number"), true);
		}
	}
	
	options->mainMemoryRequest = GAME_FIXED_HEAP_MEM_SIZE;
	options->tempMemoryRequest = GAME_TEMP_ARENA_SIZE;
	
	options->render.api = RenderApi_OpenGL;
	options->render.opengl.requestCoreProfile = true;
	//NOTE: Requesting OpenGL 3.2 or earlier requires us to change the profile type to COMPAT (or ANY)
	options->render.opengl.requestVersionMajor = 4;
	options->render.opengl.requestVersionMinor = 6;
	options->render.opengl.minVersionMajor = 3;
	options->render.opengl.minVersionMinor = 3;
	//NOTE: Setting this to true causes weird texture problems when we run with the Steam Overlay on top.
	//      This setting is supposed to make sure that all deprecated functionality in version 3.0 of OpenGL
	//      are unsupported. Maybe we are using some deprecated features that we shouldn't be?
	options->render.opengl.forwardCompat = true;
	options->render.opengl.debugEnabled = (true && DEBUG_BUILD);
	options->openDebugConsole = false;
	if (startup->GetProgramArg(startup->platTempArena, NewStr("debug"), nullptr))
	{
		options->openDebugConsole = true;
	}
	
	options->audioDeviceIndex = startup->defaultAudioDeviceIndex;
	options->audioOutputFormat.bitsPerSample    = 16;
	options->audioOutputFormat.numChannels      = 2;
	options->audioOutputFormat.samplesPerSecond = 44100;
	
	options->threadPoolSize = GAME_THREAD_POOL_SIZE;
	options->threadPoolTempArenasSize = GAME_THREAD_TEMP_ARENA_SIZE;
	options->threadPoolTempArenasNumMarks = GAME_THREAD_TEMP_NUM_MARKS;
	options->threadPoolScratchArenasMaxSize = GAME_THREAD_SCRATCH_ARENA_MAX_SIZE;
	options->threadPoolScratchArenasNumMarks = GAME_THREAD_SCRATCH_NUM_MARKS;
	
	options->numIconFiles = 5;
	options->iconFilePaths = AllocArray(startup->platTempArena, MyStr_t, options->numIconFiles);
	NotNull(options->iconFilePaths);
	options->iconFilePaths[0] = NewStr("Resources/icon16.png");
	options->iconFilePaths[1] = NewStr("Resources/icon24.png");
	options->iconFilePaths[2] = NewStr("Resources/icon32.png");
	options->iconFilePaths[3] = NewStr("Resources/icon64.png");
	options->iconFilePaths[4] = NewStr("Resources/icon128.png");
	
	options->numWindows = 1;
	options->windowOptions = AllocArray(startup->platTempArena, PlatWindowOptions_t, options->numWindows);
	NotNull(options->windowOptions);
	MyMemSet(options->windowOptions, 0x00, sizeof(PlatWindowOptions_t) * options->numWindows);
	
	options->windowOptions[0].create.resizableWindow = true;
	options->windowOptions[0].create.topmostWindow = makeTopmostWindow;
	options->windowOptions[0].create.decoratedWindow = true;
	options->windowOptions[0].create.antialiasingNumSamples = GAME_ANTI_ALIASING_SAMPLES;
	options->windowOptions[0].create.autoIconify = true;
	options->windowOptions[0].create.windowTitle = NewStringInArenaNt(startup->platTempArena, GAME_WINDOW_TITLE);
	options->windowOptions[0].enforceMinSize = true;
	options->windowOptions[0].minWindowSize = GAME_WINDOW_MIN_SIZE;
	options->windowOptions[0].enforceMaxSize = false;
	options->windowOptions[0].maxWindowSize = Vec2i_Zero;
	options->windowOptions[0].forceAspectRatio = false;
	options->windowOptions[0].aspectRatio = Vec2i_Zero;
	
	options->windowOptions[0].create.fullscreen = savedFullscreen;
	if (savedFullscreen)
	{
		if (savedMonitorInfo == nullptr)
		{
			savedMonitorInfo = LinkedListGet(&startup->monitors->list, PlatMonitorInfo_t, startup->monitors->primaryIndex);
			NotNull(savedMonitorInfo);
		}
		
		options->windowOptions[0].create.fullscreenMonitor = savedMonitorInfo;
		options->windowOptions[0].create.fullscreenVideoMode = nullptr;
		options->windowOptions[0].create.fullscreenFramerateIndex = 0;
		
		bool foundSavedVideoMode = false;
		bool foundSavedFramerate = false;
		VarArrayLoop(&savedMonitorInfo->videoModes, vIndex)
		{
			VarArrayLoopGet(PlatMonitorVideoMode_t, videoMode, &savedMonitorInfo->videoModes, vIndex);
			if (videoMode->resolution == savedResolution)
			{
				options->windowOptions[0].create.fullscreenVideoMode = videoMode;
				foundSavedVideoMode = true;
				for (u64 fIndex = 0; fIndex < videoMode->numFramerates; fIndex++)
				{
					if ((u64)videoMode->framerates[fIndex] == savedFramerate)
					{
						options->windowOptions[0].create.fullscreenFramerateIndex = fIndex;
						foundSavedFramerate = true;
						break;
					}
				}
				break;
			}
		}
		if (!foundSavedVideoMode)
		{
			startup->DebugOutput(NewStr("The monitor we were on when we last closed doesn't support the previously used resolution. Maybe it changed if you moved which port your monitor is connected to and therefore what video modes it supports"), true);
		}
		else if (!foundSavedFramerate)
		{
			startup->DebugOutput(NewStr("The monitor we were on when we last closed doesn't support the previously used framerate. Maybe it changed if you moved which port your monitor is connected to and therefore what video modes it supports"), true);
		}
		
		if (!foundSavedVideoMode || !foundSavedFramerate)
		{
			//If we couldn't find the saved resolution and framerate, default back to windowed mode with no consideration for saved settings
			options->windowOptions[0].create.fullscreen = false;
			options->windowOptions[0].create.fullscreenMonitor = nullptr;
			options->windowOptions[0].create.fullscreenVideoMode = nullptr;
			options->windowOptions[0].create.fullscreenFramerateIndex = 0;
			options->windowOptions[0].create.windowedResolution = GAME_WINDOW_DEFAULT_RESOLUTION;
			options->windowOptions[0].create.windowedLocation = NewVec2i(-1, -1);
			options->windowOptions[0].create.windowedMaximized = false;
			options->windowOptions[0].create.windowedFramerate = PIG_DEFAULT_FRAMERATE;
		}
	}
	else
	{
		//TODO: Maybe we should default to whichever monitor the mouse is currently in?
		//default to primary monitor if one is not specified
		if (savedMonitorInfo == nullptr)
		{
			savedMonitorInfo = LinkedListGet(&startup->monitors->list, PlatMonitorInfo_t, startup->monitors->primaryIndex);
			NotNull(savedMonitorInfo);
		}
		
		options->windowOptions[0].create.windowedResolution = savedResolution;
		options->windowOptions[0].create.windowedLocation = NewVec2i(-1, -1);
		options->windowOptions[0].create.windowedMaximized = savedMaximizedWindow;
		options->windowOptions[0].create.windowedFramerate = savedFramerate;
		if (savedWindowedPosition != NewVec2i(-1, -1))
		{
			options->windowOptions[0].create.windowedLocation = savedMonitorInfo->desktopSpaceRec.topLeft + savedWindowedPosition;
		}
		else
		{
			//We will default to the center of the monitor
			options->windowOptions[0].create.windowedLocation = NewVec2i(
				savedMonitorInfo->desktopSpaceRec.x + savedMonitorInfo->desktopSpaceRec.width/2 - savedResolution.width/2,
				savedMonitorInfo->desktopSpaceRec.y + savedMonitorInfo->desktopSpaceRec.height/2 - savedResolution.height/2
			);
		}
	}
	
	options->loadingBackgroundColor = NewColor(0xFF361935);
	options->loadingBarColor = NewColor(0xFFA25B95);
	options->loadingImagePath = NewStringInArenaNt(startup->platTempArena, RESOURCE_FOLDER_SPRITES "/pig_loading_image.png");
	options->loadingBackPath = NewStringInArenaNt(startup->platTempArena, RESOURCE_FOLDER_TEXTURES "/pig_loading_back.png");
	options->loadingBackScale = 0.0f;
	options->loadingBackTiling = false;
}

