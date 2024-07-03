@echo off

rem NOTE: We need to change this in the build_installer.nsi as well
rem The "Safe" name doesn't have spaces or invalid characters and also doesn't change for Demo vs non-Demo
set ProjectName=Process Spy
set ProjectNameSafe=ProcessSpy

set CompilePlatform=1
set CompileEngine=1
set CompileInstaller=0
set DumpPreprocessor=0
REM You have to run the game from a terminal that has run VsDevCmd.bat for this to work
set EnableAddressSanitizer=0
set CopyToDataDirectory=1

set LibDirectory=..\lib
set EngineSourceDirectory=..\engine
set GameSourceDirectory=..\game
set DataDirectory=..\data
set GeneratedCodeDirectory=..\build\gen
set PlatformCodePath=%EngineSourceDirectory%\platform\win32\win32_main.cpp
set EngineCodePath=%EngineSourceDirectory%\pig\pig_main.cpp
set IncVersNumScriptPath=%LibDirectory%\include\gylib\IncrementVersionNumber.py
set ExtractDefineScriptPath=%LibDirectory%\include\gylib\ExtractDefine.py
set PlatformVersionFilePath=%EngineSourceDirectory%\platform\win32\win32_version.h
set EngineVersionFilePath=%EngineSourceDirectory%\pig\pig_version.h
set GameVersionFilePath=%GameSourceDirectory%\game_version.h
set TimeString=%date:~-4,4%%date:~-10,2%%date:~-7,2%%time:~0,2%%time:~3,2%%time:~6,2%

echo Running on %ComputerName%

python --version 2>NUL
if errorlevel 1 (
	echo WARNING: Python isn't installed on this computer. Defines cannot be extracted from pig_config.h! And build number won't be incremented
	exit
)

for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h DEBUG_BUILD') do set DEBUG_BUILD=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h RUN_PIG_GEN') do set RUN_PIG_GEN=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h DEMO_BUILD') do set DEMO_BUILD=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h STEAM_BUILD') do set STEAM_BUILD=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h OPENGL_SUPPORTED') do set OPENGL_SUPPORTED=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h DIRECTX_SUPPORTED') do set DIRECTX_SUPPORTED=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h VULKAN_SUPPORTED') do set VULKAN_SUPPORTED=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h PROCMON_SUPPORTED') do set PROCMON_SUPPORTED=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h SOCKETS_SUPPORTED') do set SOCKETS_SUPPORTED=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h BOX2D_SUPPORTED') do set BOX2D_SUPPORTED=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h SLUG_SUPPORTED') do set SLUG_SUPPORTED=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h LUA_SUPPORTED') do set LUA_SUPPORTED=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h BULLET_SUPPORTED') do set BULLET_SUPPORTED=%%i
for /f "delims=" %%i in ('python %ExtractDefineScriptPath% %GameSourceDirectory%\pig_config.h PYTHON_SUPPORTED') do set PYTHON_SUPPORTED=%%i

if "%DEMO_BUILD%"=="1" (
	set ProjectName=%ProjectName% Demo
)

rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
rem call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -no_logo
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64

set CompilerFlags=-DWINDOWS_COMPILATION -DPROJECT_NAME="\"%ProjectName%\"" -DPROJECT_NAME_SAFE=\"%ProjectNameSafe%\"
rem /FC = Full path for error messages
rem /EHsc = Exception Handling Model: Standard C++ Stack Unwinding. Functions declared as extern "C" can't throw exceptions
rem /EHa- = TODO: Do we really need this?? It seems like this option should be off if we specify s and c earlier
rem /nologo = Suppress the startup banner
rem /GS- = Buffer overrun protection is turned off
rem /Gm- = Minimal rebuild is enabled [deprecated]
rem /GR- = Run-Time Type Information is Disabled [_CPPRTTI macro doesn't work]
rem /Fm = Enables map file output [with default name, same as /MAP linker option]
rem /Oi = Enable intrinsic generation
rem /WX = Treat warnings as errors
rem /W4 = Warning level 4 [just below /Wall]
set CompilerFlags=%CompilerFlags% /FC /EHsc /EHa- /nologo /GS- /Gm- /GR- /Fm /Oi /WX /W4
rem /wd4130 = Logical operation on address of string constant [W4] TODO: Should we re-enable this one? Don't know any scenarios where I want to do this
rem /wd4201 = Nonstandard extension used: nameless struct/union [W4] TODO: Should we re-enable this restriction for ANSI compatibility?
rem /wd4324 = Structure was padded due to __declspec[align[]] [W4]
rem /wd4458 = Declaration of 'identifier' hides class member [W4]
rem /wd4505 = Unreferenced local function has been removed [W4]
rem /wd4996 = Usage of deprecated function, class member, variable, or typedef [W3]
rem /wd4127 = Conditional expression is constant [W4]
rem /wd4706 = assignment within conditional expression [W?]
set CompilerFlags=%CompilerFlags% /wd4130 /wd4201 /wd4324 /wd4458 /wd4505 /wd4996 /wd4127 /wd4706
set Libraries=
set EngineLibraries=
set LinkerFlags=-incremental:no
rem TODO: Do we really need all of these? Maybe go through and document what functions we use from them?
rem glfw3.lib    = GLFW, platform independent window creation + device context + input
rem gdi32.lib    = ?
rem User32.lib   = ?
rem Shell32.lib  = Shlobj.h ? 
rem kernel32.lib = ?
rem winmm.lib    = ?
rem Winhttp.lib  = ?
rem Shlwapi.lib  = ?
rem Ole32.lib    = Combaseapi.h, CoCreateInstance
set PlatformLibraries=glfw3.lib gdi32.lib User32.lib Shell32.lib kernel32.lib winmm.lib Winhttp.lib Shlwapi.lib Ole32.lib
set IncludeDirectories=/I"%EngineSourceDirectory%" /I"%EngineSourceDirectory%"\platform /I"%GameSourceDirectory%" /I"%GeneratedCodeDirectory%" /I"%LibDirectory%\include" /I"%LibDirectory%\include\my_glfw\include" /I"%LibDirectory%\include\imgui"
set EngineDllExports=/EXPORT:Pig_GetVersion /EXPORT:Pig_GetStartupOptions /EXPORT:Pig_Initialize /EXPORT:Pig_Update /EXPORT:Pig_AudioService /EXPORT:Pig_ShouldWindowClose /EXPORT:Pig_Closing /EXPORT:Pig_PreReload /EXPORT:Pig_PostReload /EXPORT:Pig_PerformTask
set EngineDllDirective=/DLL /PDB:"%ProjectNameSafe%_%TimeString%.pdb"
set LibraryDirectories=

if "%DEBUG_BUILD%"=="1" (
	rem /Od = Optimization level: Debug
	rem /Zi = Generate complete debugging information
	rem /MTd = Statically link the standard library [not as a DLL, Debug version]
	rem /wd4065 = Switch statement contains 'default' but no 'case' labels
	rem /wd4100 = Unreferenced formal parameter [W4]
	rem /wd4101 = Unreferenced local variable [W3]
	rem /wd4127 = Conditional expression is constant [W4]
	rem /wd4189 = Local variable is initialized but not referenced [W4]
	rem /wd4702 = Unreachable code [W4]
	set CompilerFlags=%CompilerFlags% /Od /Zi /MTd /wd4065 /wd4100 /wd4101 /wd4127 /wd4189 /wd4702
	set LibraryDirectories=%LibraryDirectories% /LIBPATH:"%LibDirectory%\debug" /LIBPATH:"%LibDirectory%\include\my_glfw\debug" /LIBPATH:"%LibDirectory%\include\lua\build\debug"
	rem Dbghelp.lib = ?
	set Libraries=%Libraries% Dbghelp.lib
	rem set EngineLibraries=%EngineLibraries%
) else (
	rem /Ot = Favors fast code over small code
	rem /Oy = Omit frame pointer [x86 only]
	rem /O2 = Optimization level 2: Creates fast code
	rem /MT = Statically link the standard library [not as a DLL]
	rem /Zi = Generate complete debugging information [optional]
	set CompilerFlags=%CompilerFlags% /Ot /Oy /O2 /MT /Zi
	set LibraryDirectories=%LibraryDirectories% /LIBPATH:"%LibDirectory%\release" /LIBPATH:"%LibDirectory%\include\my_glfw\release" /LIBPATH:"%LibDirectory%\include\lua\build\release"
	rem set Libraries=%Libraries%
	rem set EngineLibraries=%EngineLibraries%
)

if "%EnableAddressSanitizer%"=="1" (
	rem /fsanitize=address = Enable Address Sanitizer
	set CompilerFlags=%CompilerFlags% /fsanitize=address
)

if "%STEAM_BUILD%"=="1" (
	set Libraries=%Libraries% steam_api64.lib
)

if "%BOX2D_SUPPORTED%"=="1" (
	set IncludeDirectories=%IncludeDirectories% /I"%LibDirectory%\include\Box2D\include"
)
if "%SLUG_SUPPORTED%"=="1" (
	set IncludeDirectories=%IncludeDirectories% /I"%LibDirectory%\include\slug\SlugCode" /I"%LibDirectory%\include\slug\TerathonCode"
)
if "%SOCKETS_SUPPORTED%"=="1" (
	set Libraries=%Libraries% Ws2_32.lib
)
if "%BULLET_SUPPORTED%"=="1" (
	set IncludeDirectories=%IncludeDirectories% /I"%LibDirectory%\include\bullet3\src"
)
if "%LUA_SUPPORTED%"=="1" (
	set IncludeDirectories=%IncludeDirectories% /I"%LibDirectory%\include\lua\src"
	set EngineLibraries=%EngineLibraries% lua.lib
)
if "%PYTHON_SUPPORTED%"=="1" (
	set IncludeDirectories=%IncludeDirectories% /I"%LibDirectory%\include\python\Include"
)
if "%PROCMON_SUPPORTED%"=="1" (
	set CompilerFlags=%CompilerFlags% -DUNICODE -D_UNICODE
	set IncludeDirectories=%IncludeDirectories% /I"%LibDirectory%\include\procmonsdk\sdk\procmonsdk"
	set PlatformLibraries=%PlatformLibraries% procmonsdk.lib winspool.lib
)
if "%OPENGL_SUPPORTED%"=="1" (
	set Libraries=%Libraries% opengl32.lib
)
if "%VULKAN_SUPPORTED%"=="1" (
	set IncludeDirectories=%IncludeDirectories% /I"%VULKAN_SDK%\Include"
	set Libraries=%Libraries% vulkan-1.lib
)

rem Compile the resources file to generate resources.res which defines our program icon
rem TODO: Make the .res file show up in the build folder. Also maybe make this file path another define above
rc /nologo resources.rc

del *.pdb > NUL 2> NUL

rem +--------------------------------------------------------------+
rem |                       Compile Platform                       |
rem +--------------------------------------------------------------+
if "%CompilePlatform%"=="1" (
	echo[
	
	python %IncVersNumScriptPath% %PlatformVersionFilePath%
	
	if "%RUN_PIG_GEN%"=="1" (
		PigGen.exe ../engine -output="gen"
	)
	
	if "%DumpPreprocessor%"=="1" (
		rem /P = Output preprocessed file to [filename].i
		rem /d1PP = Undocumented option that keeps #defines in preprocessed output (useful with /P)
		rem /PD /Zc:preprocessor = Output all #defines to the debug output
		cl /Fe%ProjectNameSafe%.exe /PD /Zc:preprocessor %CompilerFlags% %IncludeDirectories% "%PlatformCodePath%" /link %LibraryDirectories% %LinkerFlags% %Libraries% %PlatformLibraries% resources.res > defines_win32_main.txt
		cl /Fe%ProjectNameSafe%.exe /P %CompilerFlags% %IncludeDirectories% "%PlatformCodePath%" /link %LibraryDirectories% %LinkerFlags% %Libraries% %PlatformLibraries% resources.res
	) else (
		cl /Fe%ProjectNameSafe%.exe %CompilerFlags% %IncludeDirectories% "%PlatformCodePath%" /link %LibraryDirectories% %LinkerFlags% %Libraries% %PlatformLibraries% resources.res
	)
	
	if "%CopyToDataDirectory%"=="1" (
		echo [Copying %ProjectNameSafe%.exe to data directory]
		XCOPY ".\%ProjectNameSafe%.exe" "%DataDirectory%\" /Y > NUL
	) else (
		echo [Platform Build Finished!]
	)
)

rem +--------------------------------------------------------------+
rem |                        Compile Engine                        |
rem +--------------------------------------------------------------+
if "%CompileEngine%"=="1" (
	echo[
	
	python %IncVersNumScriptPath% %EngineVersionFilePath%
	python %IncVersNumScriptPath% %GameVersionFilePath%
	
	if "%RUN_PIG_GEN%"=="1" (
		PigGen.exe ../game -output="gen"
	)
	
	if "%DumpPreprocessor%"=="1" (
		rem /P = Output preprocessed file to [filename].i
		rem /d1PP = Undocumented option that keeps #defines in preprocessed output (useful with /P)
		rem /PD /Zc:preprocessor = Output all #defines to the debug output
		cl /Fe%ProjectNameSafe%.dll /PD /Zc:preprocessor %CompilerFlags% %IncludeDirectories% "%EngineCodePath%" /link %LibraryDirectories% %LinkerFlags% %Libraries% %EngineLibraries% %EngineDllExports% %EngineDllDirective% > defines_pig_main.txt
		cl /Fe%ProjectNameSafe%.dll /P %CompilerFlags% %IncludeDirectories% "%EngineCodePath%" /link %LibraryDirectories% %LinkerFlags% %Libraries% %EngineLibraries% %EngineDllExports% %EngineDllDirective%
	) else (
		cl /Fe%ProjectNameSafe%.dll %CompilerFlags% %IncludeDirectories% "%EngineCodePath%" /link %LibraryDirectories% %LinkerFlags% %Libraries% %EngineLibraries% %EngineDllExports% %EngineDllDirective%
	)
	
	if "%CopyToDataDirectory%"=="1" (
		echo [Copying %ProjectNameSafe%.dll to data directory]
		XCOPY ".\%ProjectNameSafe%.dll" "%DataDirectory%\" /Y > NUL
	) else (
		echo [Engine Build Finished!]
	)
)

rem +--------------------------------------------------------------+
rem |                      Compile Installer                       |
rem +--------------------------------------------------------------+
if "%CompileInstaller%"=="1" (
	echo[
	
	makensis build_installer.nsi
)
