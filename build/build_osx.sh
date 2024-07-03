#!/bin/bash

ProjectName="PigEngineTest"

CompilePlatform=1
CompileEngine=1
CompilePlugins=0

DebugBuild=1
DeveloperBuild=1
DemoBuild=0
SteamBuild=0
EarlyAccessBuild=1
OpenGlSupport=1
VulkanSupport=0

CopyToDataDirectory=1

LibDirectory="../lib"
SourceDirectory="../code"
DataDirectory="../data"
PlatformCodePath="$SourceDirectory/platform/osx/osx_main.cpp"
EngineCodePath="$SourceDirectory/pig/pig_main.cpp"
IncVersNumScriptPath="../IncrementVersionNumber.py"
PlatformVersionFilePath="$SourceDirectory/platform/osx/osx_version.h"
EngineVersionFilePath="$SourceDirectory/pig/pig_version.h"
TimeString="TODOImplementMe"

# NOTE: $PWD gives us the current working directory as well
echo Running on $HOSTNAME

if [ $DemoBuild -gt 0 ]
then
	ProjectName="${ProjectName}_Demo"
fi

python --version 2> /dev/null
if [ $? -ne 0 ]
then
	PythonInstalled=0
	echo Python isn\'t installed on this computer
else
	PythonInstalled=1
	python --version
fi

# NOTE: For whatever reason -L and -I can't have quotes around their values

CompilerFlags="-DOSX_COMPILATION -DDEBUG_BUILD=$DebugBuild -DDEVELOPER_BUILD=$DeveloperBuild -DDEMO_BUILD=$DemoBuild -DSTEAM_BUILD=$SteamBuild -DEARLY_ACCESS_BUILD=$EarlyAccessBuild -DOPENGL_SUPPORTED=$OpenGlSupport -DVULKAN_SUPPORTED=$VulkanSupport -DDIRECTX_SUPPORTED=0"
# -g = TODO: Describe me
# -std=gnu++0x = TODO: Describe me
CompilerFlags="$CompilerFlags -g -std=gnu++0x"
# -Wno-format-security = TODO: Describe me
CompilerFlags="$CompilerFlags -Wno-format-security"
#  -lglew
Libraries="-lglfw3"
# -framework OpenAL -framework Cocoa -framework IOKit -framework CoreVideo
Frameworks="-framework OpenGL"
LinkerFlags=""
IncludeDirectories="-I$SourceDirectory -I$SourceDirectory/platform -I$LibDirectory/include -I$LibDirectory/include/my_glfw/include"

if [ $DebugBuild -gt 0 ]
then
	GlfwLibPath=$LibDirectory/include/my_glfw/debug
	LibraryDirectories="-L$LibDirectory/debug -L$GlfwLibPath"
else
	GlfwLibPath=$LibDirectory/include/my_glfw/release
	LibraryDirectories="-L$LibDirectory/release -L$GlfwLibPath"
fi

if [ $CompilePlatform -gt 0 ]
then
	if [ $PythonInstalled -gt 0 ]
	then
		python "$IncVersNumScriptPath" "$PlatformVersionFilePath"
	fi
	
	clang $CompilerFlags $PlatformCodePath -o $ProjectName $IncludeDirectories $LibraryDirectories $Libraries $Frameworks $LinkerFlags
	
	if [ $? -ne 0 ]
	then
		echo Platform Build Failed!
	else
		echo Platform Build Succeeded!
	fi
	
	echo 
	
	if [ $CopyToDataDirectory -gt 0 ]
	then
		cp "$ProjectName" "$DataDirectory/$ProjectName"
	fi
fi

cp $GlfwLibPath/libglfw3.dylib $DataDirectory/libglfw3.dylib
