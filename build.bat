@SETLOCAL

if [%SDL_PATH%]==[] @SET SDL_PATH="X:/libs/SDL/2.0.14"
@SET CFLAGS=-W3 -WX -MTd -Zi -D_CRT_SECURE_NO_WARNINGS=1 -DBUILD_DEBUG=1 -I%SDL_PATH%/include
@SET LFLAGS=-subsystem:console -incremental:no -opt:ref -dynamicbase
@SET LLIBS=shell32.lib %SDL_PATH%/lib/x64/SDL2.lib %SDL_PATH%/lib/x64/SDL2main.lib
@SET SRC="../gjk.cc" "../gjk_collision_test.cc" "../main.cc"

pushd %~dp0
del /q .\build\*
mkdir .\build
pushd .\build
cl %1 -Fe:"gjk.exe" %CFLAGS%  %SRC% /link %LFLAGS% %LLIBS%
popd
popd

@ENDLOCAL

