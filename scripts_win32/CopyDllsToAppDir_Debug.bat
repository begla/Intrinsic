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

copy ..\\Intrinsic_Dependencies\\dependencies\\fbx\\bin\\debug\\libfbxsdk.dll app\\libfbxsdk.dll

cd scripts_win32

