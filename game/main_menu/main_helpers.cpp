/*
File:   main_helpers.cpp
Author: Taylor Robbins
Date:   07\19\2024
Description: 
	** Holds a variety of loose functions that help the Main AppState do it's job
*/

ItemProcessRef_t* FindItemProcessRef(FolderFileItem_t* item, const ProcmonProcess_t* processPntr)
{
	NotNull2(item, processPntr);
	VarArrayLoop(&item->processRefs, pIndex)
	{
		VarArrayLoopGet(ItemProcessRef_t, processRef, &item->processRefs, pIndex);
		if (processRef->process == processPntr) { return processRef; }
	}
	return nullptr;
}
