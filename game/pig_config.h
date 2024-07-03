/*
File:   pig_config.h
Author: Taylor Robbins
Date:   04\07\2024
Description:
	** This file is included by pig_main.cpp and win32_main.cpp BEFORE any other files are included.
	** It's also scraped by a python script in build.bat to inform which libraries to link and compiler options.
	** This allows us to configure Pig Engine features\options on a per-game basis in a single
	** file and without having to change the the build.bat as often
*/

#ifndef _PIG_CONFIG_H
#define _PIG_CONFIG_H

//NOTE: These must be 1/0 (not true/false) so the batch file can parse them properly

#define DEBUG_BUILD         1
#define ASSERTIONS_ENABLED  1
#define RUN_PIG_GEN         1

#define DEVELOPER_BUILD     1
#define DEMO_BUILD          0
#define STEAM_BUILD         0

#define OPENGL_SUPPORTED    1
#define WEBGL_SUPPORTED     0
#define VULKAN_SUPPORTED    0
#define DIRECTX_SUPPORTED   0

#define PROCMON_SUPPORTED   0
#define SOCKETS_SUPPORTED   1
#define BOX2D_SUPPORTED     0
#define SLUG_SUPPORTED      0
#define JSON_SUPPORTED      0
#define LUA_SUPPORTED       0
#define BULLET_SUPPORTED    0
#define PYTHON_SUPPORTED    0

#endif //  _PIG_CONFIG_H
