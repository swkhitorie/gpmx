@echo off
setlocal enabledelayedexpansion

set proj_dir=%~sdp0

if "%2" neq "" (
    echo build in win
) else (
    echo build.bat subpath -j1/2/... -r 
    exit 1
)

set subpath=%1
set thread=%2
set rebuild=%3
set buildscript=%proj_dir%build.sh
set busybox=%proj_dir%buildtools_win\busybox.exe

@REM Find arm-none-eabi-gcc
for /f "tokens=*" %%i in ('where arm-none-eabi-gcc 2^>nul') do (
    set tc_path_inst_gcc=%%i
    set tc_path_inst_gcc=!tc_path_inst_gcc:\bin\arm-none-eabi-gcc.exe=!
    @REM echo find arm-none-eabi-gcc: !tc_path_inst_gcc!
    goto :break
)
:break

if not defined tc_path_inst_gcc (
    echo cannot find arm-none-eabi-gcc, please check
    exit 1
)

@REM Find armcc
for /f "tokens=*" %%i in ('where armcc 2^>nul') do (
    set tc_path_inst_armcc=%%i
    set tc_path_inst_armcc=!tc_path_inst_armcc:\bin\armcc.exe=!
    @REM echo find armcc: !tc_path_inst_armcc!
    goto :break
)
:break

if not defined tc_path_inst_armcc (
    echo cannot find armcc, please check
    exit 1
)

@REM Find armclang
for /f "tokens=*" %%i in ('where armclang 2^>nul') do (
    set tc_path_inst_armclang=%%i
    set tc_path_inst_armclang=!tc_path_inst_armclang:\bin\armclang.exe=!
    @REM echo find armclang: !tc_path_inst_armclang!
    goto :break
)
:break

if not defined tc_path_inst_armclang (
    echo cannot find armclang, please check
    exit 1
)

%busybox% bash %buildscript% %subpath% %thread% %rebuild% %tc_path_inst_armcc% %tc_path_inst_armclang% %tc_path_inst_gcc%
