@echo off
setlocal enabledelayedexpansion

set proj_dir=%~sdp0

if "%2" neq "" (
    echo upload in win
) else (
    echo ym_upload.bat port load_file
    exit 1
)

set port=%1
set loadfile=%2
set buildscript=%proj_dir%ym_upload.sh
set busybox=%proj_dir%busybox.exe

%busybox% bash %buildscript% %port% %loadfile%
