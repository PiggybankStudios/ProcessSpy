/*
File:   game_debug_commands.cpp
Author: Taylor Robbins
Date:   06\17\2022
Description: 
	** Holds the GameHandleDebugCommand function which is where the game is
	** able to try processing a debug command input to the console.
	** See pig_debug_commands.cpp for engine level debug commands
*/

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
	// Unimplemented(); //TODO: Implement me!
	PrintLine_D("IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION = 0x%X (%d)", IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION);
	PrintLine_D("IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION = 0x%X (%d)", IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION);
	PrintLine_D("IRP_MJ_ACQUIRE_FOR_MOD_WRITE = 0x%X (%d)", IRP_MJ_ACQUIRE_FOR_MOD_WRITE);
	PrintLine_D("IRP_MJ_RELEASE_FOR_MOD_WRITE = 0x%X (%d)", IRP_MJ_RELEASE_FOR_MOD_WRITE);
	PrintLine_D("IRP_MJ_ACQUIRE_FOR_CC_FLUSH = 0x%X (%d)", IRP_MJ_ACQUIRE_FOR_CC_FLUSH);
	PrintLine_D("IRP_MJ_RELEASE_FOR_CC_FLUSH = 0x%X (%d)", IRP_MJ_RELEASE_FOR_CC_FLUSH);
	PrintLine_D("IRP_MJ_QUERY_OPEN = 0x%X (%d)", IRP_MJ_QUERY_OPEN);
	PrintLine_D("IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE = 0x%X (%d)", IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE);
	PrintLine_D("IRP_MJ_NETWORK_QUERY_OPEN = 0x%X (%d)", IRP_MJ_NETWORK_QUERY_OPEN);
	PrintLine_D("IRP_MJ_MDL_READ = 0x%X (%d)", IRP_MJ_MDL_READ);
	PrintLine_D("IRP_MJ_MDL_READ_COMPLETE = 0x%X (%d)", IRP_MJ_MDL_READ_COMPLETE);
	PrintLine_D("IRP_MJ_PREPARE_MDL_WRITE = 0x%X (%d)", IRP_MJ_PREPARE_MDL_WRITE);
	PrintLine_D("IRP_MJ_MDL_WRITE_COMPLETE = 0x%X (%d)", IRP_MJ_MDL_WRITE_COMPLETE);
	PrintLine_D("IRP_MJ_VOLUME_MOUNT = 0x%X (%d)", IRP_MJ_VOLUME_MOUNT);
	PrintLine_D("IRP_MJ_VOLUME_DISMOUNT = 0x%X (%d)", IRP_MJ_VOLUME_DISMOUNT);
}
EXPRESSION_FUNC_DEFINITION(Debug_Test_Glue) { Debug_Test(); return NewExpValueVoid(); }

// +--------------------------------------------------------------+
// |                         Registration                         |
// +--------------------------------------------------------------+
void GameAddDebugVarsToExpContext(ExpContext_t* context) //pre-declared in game_main.h
{
	const bool read = false;
	const bool write = true;
	
	// Constants
	AddExpConstantDef(context, "version_major", NewExpValueU8(GAME_VERSION_MAJOR));
	AddExpConstantDef(context, "version_minor", NewExpValueU8(GAME_VERSION_MINOR));
	AddExpConstantDef(context, "version_build", NewExpValueU16(GAME_VERSION_BUILD));
}

void GameAddDebugCommandsToExpContext(ExpContext_t* context) //pre-declared in game_main.h
{
	AddDebugCommandDef(context, Debug_Test_Def, Debug_Test_Glue, Debug_Test_Desc);
}
