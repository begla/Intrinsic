call Build_Debug.bat

IF %ERRORLEVEL% EQU 0 (
   cd ..\app
   call IntrinsicEdDebug.exe
   cd ..\scripts_win32
)

timeout 2
exit 0