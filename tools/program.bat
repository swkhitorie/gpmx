@echo off
setlocal enabledelayedexpansion

set proj_dir=%~sdp0

@REM if "%2" neq "" (
@REM     echo program in win
@REM ) else (
@REM     echo program.bat subpath -j1/2/... -r stlink stm32f1
@REM     exit 1
@REM )

@REM set subpath=%1
@REM set thread=%2
@REM set rebuild=%3
@REM set interface=%4
@REM set target=%5
set buildscript=%proj_dir%program.sh
set busybox=%proj_dir%busybox.exe

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

@REM Find openocd
for /f "tokens=*" %%i in ('where openocd 2^>nul') do (
    set tc_path_openocd=%%i
    set tc_path_openocd=!tc_path_openocd:\bin\openocd.exe=!
    goto :break
)
:break

if not defined tc_path_openocd (
    echo cannot find tc_path_openocd, please check
    exit 1
)

for /f "tokens=*" %%i in ('openocd -v 2^>^&1') do (
    set "tc_ver_openocd=%%i"
    goto :break
)
:break

if not defined tc_ver_openocd (
    echo cannot find tc_ver_openocd, please check
    exit 1
)

set "param_string=%*"
set "make_cmd="
set "current_token="

:: 1. Ensure we process the string character by character.
for /f "delims=" %%C in ('cmd /u /c echo !param_string!^| find /v ""') do (
    set "char=%%C"
    if "!char!"==" " (
        :: 2. A space marks the end of a token.
        if defined current_token (
            @REM echo [ADD] Token: !current_token!
            set "make_cmd=!make_cmd! "!current_token!""
            set "current_token="
        )
    ) else (
        :: 3. Non-space character, append to current token.
        set "current_token=!current_token!!char!"
    )
)

:: 4. Don't forget the last token.
if defined current_token (
    @REM echo [ADD] Token: !current_token!
    set "make_cmd=!make_cmd! "!current_token!""
)

%busybox% bash %buildscript% %tc_path_inst_armcc% %tc_path_inst_armclang% %tc_path_inst_gcc% %tc_path_openocd% "%tc_ver_openocd%" !make_cmd!
