cd ../dependencies/glslang
mkdir build
cd build

cmake .. -G"Visual Studio 14 2015 Win64"
call cmake --build . --config Release
call cmake --build . --config Debug

cd ..\..\..\scripts_win32

timeout 2
