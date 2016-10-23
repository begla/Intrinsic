call Build_Debug.bat

IF %ERRORLEVEL% EQU 0 (
   cd ../app
   call IntrinsicEdDebug.exe
)

timeout 2
exit 0