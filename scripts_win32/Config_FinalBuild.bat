cd ..
mkdir build
cd build

cmake -DINTR_FINAL_BUILD:BOOL=ON -G"Visual Studio 14 2015 Win64" ..

timeout 2
