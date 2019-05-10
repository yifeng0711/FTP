cls
color 0a
set current_dir=%~dp0
pushd %current_dir%

for /r %current_dir% %%i in (*.vcproj, *.pro) do (
	cd %%~pi
	del *.user
	del Makefile	/a
	del Makefile.Debug	/a
	del Makefile.Release	/a
	rd /s/q temp
	rd /s/q debug
	rd /s/q release
	rd /s/q GeneratedFiles
	rd /s/q Win32
)


rd /s /q %current_dir%Debug
rd /s /q %current_dir%Release
rd /s /q %current_dir%PDB

del %current_dir%*.suo	/a
del %current_dir%*.ncb	/a

popd

