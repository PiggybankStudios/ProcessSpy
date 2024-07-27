/*
File:   game_debug_commands.cpp
Author: Taylor Robbins
Date:   06\17\2022
Description: 
	** Holds the GameHandleDebugCommand function which is where the game is
	** able to try processing a debug command input to the console.
	** See pig_debug_commands.cpp for engine level debug commands
*/

#pragma warning(push)
#pragma warning(disable: 4100) //'context': unreferenced formal parameter

// +--------------------------------------------------------------+
// |                       Command Helpers                        |
// +--------------------------------------------------------------+
ResourcePool_t* GameGetCurrentResourcePool() //pre-declared in game_main.h
{
	return nullptr; //TODO: Implement me!
}

// +--------------------------------------------------------------+
// |                             test                             |
// +--------------------------------------------------------------+
#define Debug_Test_Def  "void test()"
#define Debug_Test_Desc "Serves as a dedicated spot to place temporary test code"
void Debug_Test()
{
	Unimplemented(); //TODO: Implement me!
}
EXPRESSION_FUNC_DEFINITION(Debug_Test_Glue) { Debug_Test(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                         Registration                         |
// +--------------------------------------------------------------+
void GameAddDebugVarsToExpContext(ExpContext_t* context) //pre-declared in game_main.h
{
	const bool read = false;
	const bool write = true;
	UNUSED(read);
	UNUSED(write);
	
	// Constants
	AddExpConstantDef(context, "version_major", NewExpValueU8(GAME_VERSION_MAJOR));
	AddExpConstantDef(context, "version_minor", NewExpValueU8(GAME_VERSION_MINOR));
	AddExpConstantDef(context, "version_build", NewExpValueU16(GAME_VERSION_BUILD));
}

void GameAddDebugCommandsToExpContext(ExpContext_t* context) //pre-declared in game_main.h
{
	AddDebugCommandDef(context, Debug_Test_Def, Debug_Test_Glue, Debug_Test_Desc);
}

#pragma warning(pop)
