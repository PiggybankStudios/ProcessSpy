@echo off

set EngineName=PigEngine
set ProjectName=PigEngineTemplate

set CompilePlatform=1
set CompilePlugins=1

set DebugBuild=1
set DeveloperBuild=1
set DemoBuild=0
set EarlyAccessBuild=1

set CopyToDataDirectory=1
set GenerateWatFile=1

set LibDirectory=..\lib
set EngineSourceDirectory=..\engine
set GameSourceDirectory=..\game
set DataDirectory=..\data
set EmscriptenPath=..\..\..\lib\emsdk\upstream\emscripten
set PlatformCodePath=%EngineSourceDirectory%\platform\web\web_main.cpp
rem set PlatformCodePath=..\test\web_test.cpp
set PlatformWatCodePath=%EngineSourceDirectory%\platform\web\web_assembly_functions.wat
set PlatformHtmlPath=%EngineSourceDirectory%\platform\web\index.html
set PlatformJavascriptPath=%EngineSourceDirectory%\platform\web\pig_engine.js
set PlatformCssPath=%EngineSourceDirectory%\platform\web\main.css
set EngineCodePath=%EngineSourceDirectory%\pig\pig_main.cpp
rem set EngineCodePath=%EngineSourceDirectory%\platform\web\web_pig_main_test.cpp
set IncVersNumScriptPath=..\engine\template\IncrementVersionNumber.py
set PlatformVersionFilePath=%EngineSourceDirectory%\platform\web\web_version.h
set EngineVersionFilePath=%EngineSourceDirectory%\pig\pig_version.h
set TimeString=%date:~-4,4%%date:~-10,2%%date:~-7,2%%time:~0,2%%time:~3,2%%time:~6,2%

echo Running on %ComputerName%

if "%DemoBuild%"=="1" (
	set ProjectName=%ProjectName%_Demo
)

python --version 2>NUL
if errorlevel 1 (
	echo WARNING: Python isn't installed on this computer. The build number will not be incremented
	set PythonInstalled=0
) else (
	set PythonInstalled=1
)

rem -s MIN_WEBGL_VERSION=2 = Require WebGL 2 support in target browser, for smallest generated code size.
rem -s MAX_WEBGL_VERSION=2 = ^
rem -s ENVIRONMENT=web     = The generated build output is only to be expected to be run in a web browser, never in a native CLI shell, or in a web worker.
rem set CompilerFlags=%CompilerFlags% -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s ENVIRONMENT=web
rem -s MIN_FIREFOX_VERSION=70    = ?
rem -s MIN_SAFARI_VERSION=130000 = ?
rem -s MIN_IE_VERSION=0x7FFFFFFF = Do not support Internet Explorer at all. This is the Emscripten default
rem -s MIN_EDGE_VERSION=79       = Require Chromium-based Edge browser
rem -s MIN_CHROME_VERSION=80     = ?
rem set CompilerFlags=%CompilerFlags% -s MIN_FIREFOX_VERSION=70 -s MIN_SAFARI_VERSION=130000 -s MIN_IE_VERSION=0x7FFFFFFF -s MIN_EDGE_VERSION=79 -s MIN_CHROME_VERSION=80
rem -lGL = Link in to WebGL/GLES system library
rem set Libraries=-lGL
rem set IncludeDirectories=/I"%EngineSourceDirectory%" /I"%EngineSourceDirectory%"\platform /I"%GameSourceDirectory%" /I"%LibDirectory%\include" /I"%LibDirectory%\include\my_glfw\include" /I"%LibDirectory%\include\bullet3\src"

rem TODO: Set OPENGL_SUPPORTED back to 1
set CompilerFlags=-DWASM_COMPILATION -DDEBUG_BUILD=%DebugBuild% -DDEVELOPER_BUILD=%DeveloperBuild% -DDEMO_BUILD=%DemoBuild% -DSTEAM_BUILD=0 -DEARLY_ACCESS_BUILD=%EarlyAccessBuild% -DOPENGL_SUPPORTED=0 -DVULKAN_SUPPORTED=0 -DDIRECTX_SUPPORTED=0
rem -Wenum-compare-switch = comparison of different enumeration types in switch statement
set CompilerFlags=%CompilerFlags% -Wno-enum-compare-switch
rem --target=wasm32       = Target the WASM32 instruction set?
rem -O0                   = No optimizations?
rem -c                    = Compile only, no link?
rem --no-standard-include = ?
set CompilerFlags=%CompilerFlags% --target=wasm32 -O0 -c --no-standard-includes
rem --no-entry        = ?
rem --export-all      = ?
rem --allow-undefined = ?
rem --no-gc-sections  = ?
rem --import-memory   = ?
rem --lto-O2          = ?
set LinkerFlags=--no-entry --allow-undefined --import-memory --lto-O2
rem set LinkerFlags=%LinkerFlags% --export=Initialize --export=TestFunc --export=GetStackBase --export=RenderFrame
set IncludeDirectories=-I"%LibDirectory%\include\gylib\std" -I"%EngineSourceDirectory%" -I"%EngineSourceDirectory%\platform" -I"%EngineSourceDirectory%\platform\web" -I"%LibDirectory%\include" -I"%GameSourceDirectory%"

if "%DebugBuild%"=="1" (
	rem -O0 = [compile+link] No optimizations/default. This is the recommended setting for starting to port a project, as it includes various assertions.
	rem set CompilerFlags=%CompilerFlags% -O0 -g
	rem set LibraryDirectories=/LIBPATH:"%LibDirectory%\debug" /LIBPATH:"%LibDirectory%\include\my_glfw\debug"
) else (
	rem -O3 = [compile+link] Like -O2, but with additional optimizations that may take longer to run.
	rem set CompilerFlags=%CompilerFlags% -O3
	rem set LibraryDirectories=/LIBPATH:"%LibDirectory%\release" /LIBPATH:"%LibDirectory%\include\my_glfw\release"
)

del *.pdb > NUL 2> NUL

rem Replace backslashes in this path to forward slashes for WSL calls
set PlatformCodePathLinux=%PlatformCodePath:\=/%

if "%CompilePlatform%"=="1" (
	echo[
	
	if "%PythonInstalled%"=="1" (
		python %IncVersNumScriptPath% %PlatformVersionFilePath%
	)
	
	rem %EmscriptenPath%\emcc %CompilerFlags% %Libraries% "%PlatformCodePath%" -o=%ProjectName%.html
	
	clang %PlatformCodePath% %CompilerFlags% %IncludeDirectories% -o %EngineName%.o
	rem clang %PlatformCodePath% %CompilerFlags% %IncludeDirectories% -dD -E - > prepocessed_output.txt
	wat2wasm %PlatformWatCodePath% -o assembFuncs.wasm
	wasm-ld %EngineName%.o %LinkerFlags% -o %EngineName%.wasm
	rem wsl clang %PlatformCodePathLinux% %CompilerFlags% -o %EngineName%.o
	rem wsl wasm-ld-10 %EngineName%.o %LinkerFlags% -o %EngineName%.wasm
	
	if "%GenerateWatFile%"=="1" (
		echo Generating .wat file...
		wsl wasm2wat %EngineName%.wasm -o %EngineName%.wat
	)
	
	if "%CopyToDataDirectory%"=="1" (
		echo [Copying %EngineName%.wasm to data directory]
		XCOPY ".\%EngineName%.wasm" "%DataDirectory%\" /Y > NUL
		XCOPY ".\assembFuncs.wasm" "%DataDirectory%\" /Y > NUL
		XCOPY "%PlatformHtmlPath%" "%DataDirectory%\" /Y > NUL
		XCOPY "%PlatformJavascriptPath%" "%DataDirectory%\" /Y > NUL
		XCOPY "%PlatformCssPath%" "%DataDirectory%\" /Y > NUL
	) else (
		echo [Platform Build Finished!]
	)
)

echo[
