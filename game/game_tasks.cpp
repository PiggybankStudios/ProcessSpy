/*
File:   game_tasks.cpp
Author: Taylor Robbins
Date:   02\26\2022
Description: 
	** Holds the implementation details for all game task types that can be
	** performed on the separate threads using the engine provided thread pool
*/

//NOTE: This function runs on a thread pool thread
void GameHandleTask(const PlatformInfo_t* info, const PlatformApi_t* api, PlatThreadPoolThread_t* thread, PlatTask_t* task)
{
	NotNull(info);
	NotNull(api);
	NotNull(thread);
	NotNull(task);
	switch (task->input.type)
	{
		// +--------------------------------------------------------------+
		// |                        GameTask_Test                         |
		// +--------------------------------------------------------------+
		case GameTask_Test:
		{
			PrintLine_I("Test task %llu is starting...", task->input.id);
			u64 numIterations = 25;
			for (u64 lIndex = 0; lIndex < numIterations; lIndex++)
			{
				PrintLine_I("Test task %llu is running on %llu (%llu/%llu)", task->input.id, thread->id, lIndex+1, numIterations);
				plat->SleepForMs(200);
			}
			PrintLine_I("Test task %llu is done!", task->input.id);
			task->result.resultCode = (u8)ProgramTime;
		} break;
		
		// +--------------------------------------------------------------+
		// |                        GameTask_None                         |
		// +--------------------------------------------------------------+
		// case GameTask_None:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +--------------------------------------------------------------+
		// |                        Unimplemented                         |
		// +--------------------------------------------------------------+
		default:
		{
			AssertMsg(false, "Unimplemented task type in GameHandleTask");
		}
	}
}

//NOTE: This function runs on the main thread
void GameHandleCompletedTask(PlatTask_t* task)
{
	NotNull(task);
	switch (task->input.type)
	{
		// +--------------------------------------------------------------+
		// |                   GameTask_Test Completed                    |
		// +--------------------------------------------------------------+
		case GameTask_Test:
		{
			if (task->input.callbackFunc != nullptr)
			{
				TaskCallbackFunc_f* callback = (TaskCallbackFunc_f*)task->input.callbackFunc;
				callback(task, task->input.callbackContext);
			}
			WriteLine_I("Test task finished!");
		} break;
		
		// +--------------------------------------------------------------+
		// |                   GameTask_None Completed                    |
		// +--------------------------------------------------------------+
		// case GameTask_None:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +--------------------------------------------------------------+
		// |                        Unimplemented                         |
		// +--------------------------------------------------------------+
		default:
		{
			AssertMsg(false, "Unimplemented task type in GameHandleCompletedTask");
		}
	}
}

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
