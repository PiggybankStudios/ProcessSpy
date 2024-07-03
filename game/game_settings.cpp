/*
File:   game_settings.cpp
Author: Taylor Robbins
Date:   09\17\2022
Description: 
	** Holds functions that managing parsing setting entries from the engine layer
	** and turning them into globals that can be accessed by the game. And then
	** also functions that handle turning them back into entries that can be serialized
	** then the game is closing or when settings have been changed in the settings menu
*/

void GameLoadSettings(PigSettings_t* settingsOut, MemArena_t* memArena)
{
	NotNull(settingsOut);
	bool loadedSettings = false;
	
	MyStr_t settingsFilePath = PigGetSettingsFilePath(TempArena, mainHeap, NewStr(PROJECT_NAME_SAFE), NewStr(GAME_SETTINGS_FILE_NAME));
	if (settingsFilePath.length > 0)
	{
		if (plat->DoesFileExist(settingsFilePath, nullptr))
		{
			ProcessLog_t settingsParseLog;
			CreateProcessLog(&settingsParseLog, Kilobytes(8), mainHeap, mainHeap);
			if (PigTryLoadSettings(settingsFilePath, &settingsParseLog, settingsOut, memArena))
			{
				PrintLine_I("Loaded settings successfully from \"%.*s\"", settingsFilePath.length, settingsFilePath.pntr);
				loadedSettings = true;
			}
			else
			{
				NotifyPrint_E("Failed to parse settings from \"%.*s\"", settingsFilePath.length, settingsFilePath.pntr);
			}
			if (settingsParseLog.hadErrors || settingsParseLog.hadWarnings) { DumpProcessLog(&settingsParseLog, "Settings Parse Log"); }
			FreeProcessLog(&settingsParseLog);
		}
		else
		{
			PrintLine_W("No settings found at \"%.*s\"", settingsFilePath.length, settingsFilePath.pntr);
		}
	}
	else
	{
		NotifyWrite_E("Failed to get special folder to load settings!");
	}
	
	if (!loadedSettings)
	{
		//TODO: Should we set some sort of global saying not to overwrite settings unless the user has made specific changes? That way we postpone overwriting the corrupt settings as long as possible?
		WriteLine_Wx(DbgFlag_Inverted, "All settings are being cleared because we couldn't load settings!");
		PigInitSettings(settingsOut, memArena);
	}
}

void GameParseSettings()
{
	pig->musicEnabled  = PigGetSettingBool(&pig->settings, "MusicEnabled",  true);
	pig->soundsEnabled = PigGetSettingBool(&pig->settings, "SoundsEnabled", true);
	pig->masterVolume  = PigGetSettingR32(&pig->settings, "MasterVolume",  GAME_DEFAULT_MASTER_VOLUME);
	pig->musicVolume   = PigGetSettingR32(&pig->settings, "MusicVolume",   GAME_DEFAULT_MUSIC_VOLUME);
	pig->soundsVolume  = PigGetSettingR32(&pig->settings, "SoundsVolume",  GAME_DEFAULT_SOUNDS_VOLUME);
	MyStr_t prevDebugCommand  = PigGetSettingStr(&pig->settings, "PrevDebugCommand", MyStr_Empty);
	if (prevDebugCommand.length > 0)
	{
		DebugConsolePushInputHistory(&pig->debugConsole, prevDebugCommand);
	}
	
	if (plat->GetProgramArg(nullptr, NewStr("mute"),     nullptr)) { pig->masterVolume  = 0.0f;  }
	if (plat->GetProgramArg(nullptr, NewStr("nomusic"),  nullptr)) { pig->musicEnabled  = false; }
	if (plat->GetProgramArg(nullptr, NewStr("nosounds"), nullptr)) { pig->soundsEnabled = false; }
}

bool GameSaveSettings()
{
	bool result = false;
	UpdateSettingsWithWindowInfo(&pig->settings, platInfo->mainWindow, TempArena);
	
	if (pig->musicEnabled  != true || PigIsSettingSet(&pig->settings, "MusicEnabled"))  { PigSetSettingBool(&pig->settings, "MusicEnabled",  pig->musicEnabled);  }
	if (pig->soundsEnabled != true || PigIsSettingSet(&pig->settings, "SoundsEnabled")) { PigSetSettingBool(&pig->settings, "SoundsEnabled", pig->soundsEnabled); }
	if (!BasicallyEqualR32(pig->masterVolume, GAME_DEFAULT_MASTER_VOLUME) || PigIsSettingSet(&pig->settings, "MasterVolume")) { PigSetSettingR32(&pig->settings, "MasterVolume", pig->masterVolume); }
	if (!BasicallyEqualR32(pig->musicVolume,  GAME_DEFAULT_MUSIC_VOLUME)  || PigIsSettingSet(&pig->settings, "MusicVolume"))  { PigSetSettingR32(&pig->settings, "MusicVolume",  pig->musicVolume);  }
	if (!BasicallyEqualR32(pig->soundsVolume, GAME_DEFAULT_SOUNDS_VOLUME) || PigIsSettingSet(&pig->settings, "SoundsVolume")) { PigSetSettingR32(&pig->settings, "SoundsVolume", pig->soundsVolume); }
	if (pig->debugConsole.inputHistory.length > 0)
	{
		MyStr_t prevDebugCommand = *VarArrayGet(&pig->debugConsole.inputHistory, 0, MyStr_t);
		PigSetSettingStr(&pig->settings, "PrevDebugCommand", prevDebugCommand);
	}
	
	MyStr_t settingsFilePath = PigGetSettingsFilePath(TempArena, mainHeap, NewStr(PROJECT_NAME_SAFE), NewStr(GAME_SETTINGS_FILE_NAME));
	NotNullStr(&settingsFilePath);
	if (settingsFilePath.length > 0)
	{
		if (PigTrySaveSettings(settingsFilePath, &pig->settings, mainHeap))
		{
			PrintLine_I("Saved settings to \"%.*s\"", settingsFilePath.length, settingsFilePath.pntr);
			result = true;
		}
		else
		{
			NotifyPrint_E("Failed to save settings to \"%.*s\"", settingsFilePath.length, settingsFilePath.pntr);
		}
	}
	else
	{
		NotifyWrite_E("Failed to get special folder to save settings!");
	}
	
	return result;
}
