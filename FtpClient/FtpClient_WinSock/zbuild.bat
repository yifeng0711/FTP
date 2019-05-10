@echo off
rem	-------------------------------------------------------------------------
rem	_BATPATH_:	current bat path
rem	_SLNNAME_:	current solution name with relative path
rem	_PRONAME_:	current module name
rem	_PROTYPE_:	current module type
rem	_COMPILE_:	current module compile mode
rem	_3rdPATH_:	third-party dll path
rem	_RUNPATH_:	current run result path
rem _PDBPATH_:	pdb backup path
rem	-------------------------------------------------------------------------
	set _BATPATH_=%~dp0
	
	for /r %_BATPATH_%.. %%i in (*.sln) do set _SLNNAME_=%%~ni
	for /r %_BATPATH_% %%i in (*.vcproj, *.pro) do set _PRONAME_=%%~ni
	
	set _SLNNAME_=..\%_SLNNAME_%
	set _PRONAME_=%_PRONAME_%
	set _PROTYPE_=exe
	set _COMPILE_=Release^|Win32
	set _RUNPATH_=..\release
	set _3rdPATH_=.\3rdparty
	set _PDBPATH_=..\PDB
	
	
	if "%_COMPILE_CMD_%"=="32D"	set _COMPILE_=Debug^|Win32
	if "%_COMPILE_CMD_%"=="32R"	set _COMPILE_=Release^|Win32
	if "%_COMPILE_CMD_%"=="64D"	set _COMPILE_=Debug^|x64
	if "%_COMPILE_CMD_%"=="64R"	set _COMPILE_=Release^|x64

	if "%_COMPILE_CMD_%"=="32D"	set _RUNPATH_=..\debug
	if "%_COMPILE_CMD_%"=="32R"	set _RUNPATH_=..\release
	if "%_COMPILE_CMD_%"=="64D"	set _RUNPATH_=..\debug
	if "%_COMPILE_CMD_%"=="64R"	set _RUNPATH_=..\release

rem	-----------------------------ToDo Other Begin!--------------------------
rem	-----------------------------ToDo Other End!----------------------------

echo	-------------------------------------------------------------------------
echo	BUILD:	%_RUNPATH_%\%_PRONAME_%.%_PROTYPE_%
echo	-------------------------------------------------------------------------

	del %_RUNPATH_%\%_PRONAME_%.%_PROTYPE_%
	del %_RUNPATH_%\%_PRONAME_%.exp
	del %_RUNPATH_%\%_PRONAME_%.pdb
	del %_RUNPATH_%\%_PRONAME_%.lib
	del %_RUNPATH_%\%_PRONAME_%.ilk
	del temp /q
	del debug /q
	del release /q

	call "%MVS2008_ROOT%\VC\bin\vcvars32.bat"
	devenv	/rebuild Release %_BATPATH_%%_SLNNAME_%.sln /project "%_BATPATH_%%_PRONAME_%.vcproj" /projectconfig "%_COMPILE_%"
	IF ERRORLEVEL==1 goto ERROR
	
	rem 备份PDB
	rem xcopy %_RUNPATH_%\*.pdb %_PDBPATH_%\*.pdb /y	
	rem 将依赖库拷贝到运行目录
	rem xcopy %_3rdPATH_%\*.* %_RUNPATH_%	/y
	
	del %_RUNPATH_%\%_PRONAME_%.exp
	del %_RUNPATH_%\%_PRONAME_%.pdb
	del %_RUNPATH_%\%_PRONAME_%.lib
	del %_RUNPATH_%\%_PRONAME_%.ilk
	del temp	/q
	del %_SLNNAME_%.suo	/a
	del %_SLNNAME_%.ncb	/a
	del %_SLNNAME_%.aps	/a
	del %_SLNNAME_%.idc	/a
	del %_PRONAME_%.vcproj.*.user
	del Makefile	/a
	del Makefile.Debug	/a
	del Makefile.Release	/a
	rd /s/q temp
	rd /s/q debug
	rd /s/q release
	rd /s/q GeneratedFiles
	rd /s/q Win32
	
	goto EXIT
rem	-------------------------------------------------------------------------



:ERROR
echo	-------------------------------------------------------------------------
echo	Build Error!  %_PRONAME_%.%_PROTYPE_%
echo	-------------------------------------------------------------------------
	Pause


:EXIT