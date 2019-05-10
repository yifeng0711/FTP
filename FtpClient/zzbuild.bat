cls
color 0a
set current_dir=%~dp0
pushd %current_dir%

for /r %current_dir% %%i in (*.vcproj, *.pro) do (
	cd %%~pi
	cmd /c "call zbuild.bat"
)

popd  

