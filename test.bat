@echo OFF
call build_xxl1.bat %*
set XXLFOLDER=C:\Users\Adrien\Desktop\kthings\xxl1_polish
copy d3d9.dll %XXLFOLDER%\
choice /M "Run game to test patch"
if not ERRORLEVEL 2 (
	pushd %XXLFOLDER%
	REM Asterix.exe
	GameModule_MP.exe
	popd
)