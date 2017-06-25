cd ..

copy %INTR_QTDIR%\\bin\\libEGL.dll app\\libEGL.dll
copy %INTR_QTDIR%\\bin\\libGLESv2.dll app\\libGLESv2.dll
copy %INTR_QTDIR%\\bin\\Qt5Core.dll app\\Qt5Core.dll
copy %INTR_QTDIR%\\bin\\Qt5Gui.dll app\\Qt5Gui.dll
copy %INTR_QTDIR%\\bin\\Qt5Widgets.dll app\\Qt5Widgets.dll

mkdir app\\platforms
copy %INTR_QTDIR%\\plugins\\platforms\\qwindows.dll app\\platforms\\qwindows.dll

copy ..\\Intrinsic_Dependencies\\dependencies\\physx3.4\\bin\\PxFoundation_x64.dll app\\PxFoundation_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx3.4\\bin\\PhysX3CharacterKinematic_x64.dll app\\PhysX3CharacterKinematic_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx3.4\\bin\\PhysX3_x64.dll app\\PhysX3_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx3.4\\bin\\PhysX3Cooking_x64.dll app\\PhysX3Cooking_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx3.4\\bin\\PhysX3Common_x64.dll app\\PhysX3Common_x64.dll

copy ..\\Intrinsic_Dependencies\\dependencies\\physx3.4\\bin\\PxFoundationPROFILE_x64.dll app\\PxFoundationPROFILE_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx3.4\\bin\\PhysX3CharacterKinematicPROFILE_x64.dll app\\PhysX3CharacterKinematicPROFILE_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx3.4\\bin\\PhysX3PROFILE_x64.dll app\\PhysX3PROFILE_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx3.4\\bin\\PhysX3CookingPROFILE_x64.dll app\\PhysX3CookingPROFILE_x64.dll
copy ..\\Intrinsic_Dependencies\\dependencies\\physx3.4\\bin\\PhysX3CommonPROFILE_x64.dll app\\PhysX3CommonPROFILE_x64.dll

copy ..\\Intrinsic_Dependencies\\dependencies\\physx3.4\\bin\\nvToolsExt64_1.dll app\\nvToolsExt64_1.dll

copy ..\\Intrinsic_Dependencies\\dependencies\\fbx\\bin\\release\\libfbxsdk.dll app\\libfbxsdk.dll

copy %VK_SDK_PATH%\\Source\\lib\\vulkan-1.dll app\\vulkan-1.dll
copy dependencies\\lua\\lua51.dll app\\lua51.dll
copy dependencies\\sdl\\lib\\x64\\SDL2.dll app\\SDL2.dll

copy dependencies\\bugsplat\\bin\\BsSndRpt64.exe app\\BsSndRpt64.exe
copy dependencies\\bugsplat\\bin\\BugSplat64.dll app\\BugSplat64.dll
copy dependencies\\bugsplat\\bin\\BugSplatRc64.dll app\\BugSplatRc64.dll

cd scripts_win32
