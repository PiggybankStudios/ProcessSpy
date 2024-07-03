This folder should contain copies of all the include folders from libraries that the project depends on.

The list of libraries is given below:

# gylib
https://www.github.com/PiggybankStudios/gylib
gylib folder

# my_glfw
https://www.github.com/PiggybankStudios/my_glfw
my_glfw folder
Basically just GLFW 3.2.1 with a few custom changes and using our own build.bat script
Run build.bat twice. Once with DebugBuild=1 and once with DebugBuild=0 (edit the .bat file directly)
There should now be two folders, debug and release, with glfw3.lib in them

# PigGen
https://www.github.com/PiggybankStudios/piggen
piggen folder

# GLAD
https://glad.dav1d.de/
glad folder
Flatten 3 files to glad folder.
Change #include <KHR/khrplatform.h> to #include <glad/khrplatform.h> in glad.h or put khrplatform.h in a separate KHR folder
Options:
	Language: C/C++
	Specification: OpenGL
	Profile: Compatibility
	gl: Version 4.6
	gles1: None
	gles2: None
	glsc2: None
	All Extensions

# STB Libraries
https://www.github.com/nothings/stb
stb folder

# MSF Gif
https://github.com/notnullnotvoid/msf_gif
msf_gif folder
Modifications are required to get it compiling with /W4

# Tiny Regex C
https://github.com/kokke/tiny-regex-c
regex folder
This is needed to compile PigGen.exe
+FIXUP: Rename CHAR enum value to _CHAR so it doesn't conflict with typedef in windows header

# Dear ImGui
https://github.com/ocornut/imgui
imgui folder
