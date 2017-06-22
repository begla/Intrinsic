cd %~dp0
call Build_Release.bat

IF %ERRORLEVEL% EQU 0 (
   cd ..\app
   call IntrinsicEd.exe
   cd ..\scripts_win32
)

exit 0
