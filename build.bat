@SETLOCAL

pushd %~dp0
if [%SDL_PATH%]==[] @SET SDL_PATH="X:/libs/SDL/2.0.14"
@SET COMPILER_INCLUDES=-I%SDL_PATH%/include
@SET COMPILER_DEFINES=-DARCH_X64=1 -DPLATFORM_WINDOWS=1 -DBUILD_DEBUG=1
@SET COMPILER_FLAGS=-Fe:"gjk.exe" -W3 -WX -MTd -Zi -D_CRT_SECURE_NO_WARNINGS=1 %COMPILER_DEFINES% %COMPILER_INCLUDES%
@SET LINKER_LIBRARIES=shell32.lib %SDL_PATH%/lib/x64/SDL2.lib %SDL_PATH%/lib/x64/SDL2main.lib
@SET LINKER_FLAGS=-subsystem:console -incremental:no -opt:ref -dynamicbase %LINKER_LIBRARIES%
@SET SRC="../gjk.cc" "../gjk_collision_test.cc" "../main.cc"

mkdir .\build
pushd .\build
del .\*.dll .\*.exe .\*.exp .\*.ilk .\*lib .\*.obj .\*.pdb
cl %COMPILER_FLAGS% %SRC% /link %LINKER_FLAGS%
popd
popd

@ENDLOCAL
