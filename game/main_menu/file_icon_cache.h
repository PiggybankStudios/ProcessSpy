/*
File:   file_icon_cache.h
Author: Taylor Robbins
Date:   07\20\2024
*/

#ifndef _FILE_ICON_CACHE_H
#define _FILE_ICON_CACHE_H

struct FileIcon_t
{
	u64 index;
	u64 systemId;
	Color_t color; //TODO: Remove me!
	Texture_t texture;
};

struct FileIconCache_t
{
	MemArena_t* allocArena;
	BktArray_t icons; //FileIcon_t
};

#endif //  _FILE_ICON_CACHE_H
