cd %~dp0
call Build_Release.bat

cd ..\app
call IntrinsicEd.exe
cd ..\scripts_win32
