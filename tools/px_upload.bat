@echo off
setlocal enabledelayedexpansion

set proj_dir=%~sdp0

set loadscript=%proj_dir%px_upload.sh
set busybox=%proj_dir%busybox.exe

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

%busybox% bash %loadscript% !make_cmd!
