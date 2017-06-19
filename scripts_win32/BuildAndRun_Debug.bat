cd %~dp0
call Build_Debug.bat

IF %ERRORLEVEL% EQU 0 (
   cd ..\app
   call IntrinsicEdDebug.exe
   cd ..\scripts_win32
)

exit 0
