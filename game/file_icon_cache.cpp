/*
File:   file_icon_cache.cpp
Author: Taylor Robbins
Date:   07\20\2024
Description: 
	** Icons for files in Windows have an associated index in the icon handle table
	** We use this index as an ID in a caching system so we don't have to make multiple
	** Texture_t objects for the same ID. These icons are stored in a BktArray_t so
	** a pointer can be taken an maintained, even when new items are added
*/

void FreeFileIconCache(FileIconCache_t* cache)
{
	NotNull(cache);
	for (u64 iIndex = 0; iIndex < cache->icons.length; iIndex++)
	{
		FileIcon_t* icon = BktArrayGet(&cache->icons, FileIcon_t, iIndex);
		DestroyTexture(&icon->texture);
	}
	FreeBktArray(&cache->icons);
	ClearPointer(cache);
}

void InitFileIconCache(FileIconCache_t* cache, MemArena_t* memArena)
{
	NotNull(cache);
	ClearPointer(cache);
	cache->allocArena = memArena;
	CreateBktArray(&cache->icons, memArena, sizeof(FileIcon_t));
}

FileIcon_t* FindFileIconById(FileIconCache_t* cache, u64 iconId, MyStr_t filePath)
{
	for (u64 iIndex = 0; iIndex < cache->icons.length; iIndex++)
	{
		FileIcon_t* icon = BktArrayGet(&cache->icons, FileIcon_t, iIndex);
		if (icon->systemId == iconId)
		{
			return icon;
		}
	}
	
	FileIcon_t* newIcon = BktArrayAdd(&cache->icons, FileIcon_t);
	NotNull(newIcon);
	ClearPointer(newIcon);
	newIcon->index = cache->icons.length-1;
	newIcon->systemId = iconId;
	newIcon->color = GetPredefPalColorByIndex(GetRandU64(&pig->random, 0, NUM_PREDEF_PAL_COLORS)); //TODO: Remove me!
	
	MemArena_t* scratch = GetScratchArena(cache->allocArena);
	PlatImageData_t iconImageData = {};
	if (plat->GetFileIconImageData(filePath, scratch, &iconImageData))
	{
		if (!CreateTexture(cache->allocArena, &newIcon->texture, &iconImageData, false, false))
		{
			PrintLine_E("Failed to create texture from PlatImageData for FileIcon Id=%llu", iconId);
		}
	}
	else
	{
		PrintLine_E("Failed to get FileIcon Id=%llu for file \"%.*s\"", iconId, StrPrint(filePath));
	}
	//TODO: Actually get the icon bitmap data from the OS and stuff it into the texture
	
	FreeScratchArena(scratch);
	return newIcon;
}
