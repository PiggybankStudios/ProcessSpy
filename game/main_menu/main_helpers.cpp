/*
File:   main_helpers.cpp
Author: Taylor Robbins
Date:   07\19\2024
Description: 
	** Holds a variety of loose functions that help the Main AppState do it's job
*/

v2i GetIconForExtension(MyStr_t extensionStr)
{
	if (StrEqualsIgnoreCase(extensionStr, ".png") ||
		StrEqualsIgnoreCase(extensionStr, ".jpg") ||
		StrEqualsIgnoreCase(extensionStr, ".jpeg") ||
		StrEqualsIgnoreCase(extensionStr, ".gif") ||
		StrEqualsIgnoreCase(extensionStr, ".ico") ||
		StrEqualsIgnoreCase(extensionStr, ".tiff") ||
		StrEqualsIgnoreCase(extensionStr, ".tif") ||
		StrEqualsIgnoreCase(extensionStr, ".webp") ||
		StrEqualsIgnoreCase(extensionStr, ".bmp"))
	{
		return NewVec2i(1, 1); //image
	}
	else if (StrEqualsIgnoreCase(extensionStr, ".exe"))
	{
		return NewVec2i(0, 1); //application
	}
	else if (!IsEmptyStr(extensionStr))
	{
		return NewVec2i(1, 0); //document
	}
	else
	{
		return NewVec2i(0, 0); //empty document
	}
}
