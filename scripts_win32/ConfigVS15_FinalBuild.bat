cd ..
mkdir build
cd build

cmake -DINTR_FINAL_BUILD:BOOL=ON -G"Visual Studio 15 2017 Win64" ..

cd ..\scripts_win32
