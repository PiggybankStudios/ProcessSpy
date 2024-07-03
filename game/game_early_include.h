/*
File:   game_early_include.h
Author: Taylor Robbins
Date:   03\05\2024
*/

#ifndef _GAME_EARLY_INCLUDE_H
#define _GAME_EARLY_INCLUDE_H

#define GAME_USES_FUNC_TABLE true

#if PIGGEN
@FuncTableCount
@FillGlobalFuncTable_Declaration
#else
#include "game_early_include_h_0.h"
#endif

#endif //  _GAME_EARLY_INCLUDE_H
