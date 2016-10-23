cd ..

copy %INTR_QTDIR%\\bin\\libEGLd.dll app\\libEGLd.dll
copy %INTR_QTDIR%\\bin\\libGLESv2d.dll app\\libGLESv2d.dll
copy %INTR_QTDIR%\\bin\\Qt5Cored.dll app\\Qt5Cored.dll
copy %INTR_QTDIR%\\bin\\Qt5Guid.dll app\\Qt5Guid.dll
copy %INTR_QTDIR%\\bin\\Qt5Widgetsd.dll app\\Qt5Widgetsd.dll

mkdir app\\platforms
copy %INTR_QTDIR%\\plugins\\platforms\\qwindowsd.dll app\\platforms\\qwindowsd.dll

copy ..\\Intrinsic_Dependencies\\dependencies\\physx\\bin\\PhysX3CharacterKinematicDEBUG_x64.dll app\\PhysX3CharacterKinematicDEBUG_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx\\bin\\PhysX3DEBUG_x64.dll app\\PhysX3DEBUG_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx\\bin\\PhysX3CookingDEBUG_x64.dll app\\PhysX3CookingDEBUG_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx\\bin\\PhysX3CommonDEBUG_x64.dll app\\PhysX3CommonDEBUG_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx\\bin\\nvToolsExt64_1.dll app\\nvToolsExt64_1.dll

copy ..\\Intrinsic_Dependencies\\dependencies\\fbx\\bin\\debug\\libfbxsdk.dll app\\libfbxsdk.dll
copy dependencies\\lua\\lua51.dll app\\lua51.dll
copy dependencies\\sdl\\lib\\x64\\SDL2.dll app\\SDL2.dll

timeout 2