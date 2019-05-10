cls
color 0a
set current_dir=%~dp0
pushd %current_dir%

set _COMPILE_CMD_=32R
cmd /c	"call zzbuild.bat"

popd



