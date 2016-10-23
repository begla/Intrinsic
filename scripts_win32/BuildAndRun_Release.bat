call Build_Release.bat

IF %ERRORLEVEL% EQU 0 (
   cd ../app
   call IntrinsicEd.exe
)

timeout 2
exit 0