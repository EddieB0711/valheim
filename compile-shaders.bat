@ECHO OFF

SET SLANG_EXE="%VULKAN_SDK%\Bin\slangc.exe"
SET GLSL_EXE="%VULKAN_SDK%\Bin\glslc.exe"
SET CURR_DIR=%~dp0

SET SHADER_FILES="slang"

FOR %%A IN (%SHADER_FILES%) DO (
    FOR /r %CURR_DIR% %%F IN ("*.%%A") DO (
        IF NOT "%%F"=="" (
            CALL :COMPILE_SHADER %%F
        )
    )
)

GOTO :END

:COMPILE_SHADER
SET FILE=%1
SET SPV_FILE=%~n1.spv
REM SET SPV_FILE=%~n1.glsl
SET SPV_FILE_PATH=%~dp1%SPV_FILE%

SET MODULE="module.slang"

ECHO "%FILE%" | FINDSTR /C:"%MODULE%" > NUL

if %ERRORLEVEL% EQU 1  (
    ECHO "COMPILING SHADER: %FILE%"
    ECHO "CREATING BINARY: %SPV_FILE_PATH%"

    %SLANG_EXE% %FILE% -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o %SPV_FILE_PATH%
    REM %SLANG_EXE% %FILE% -target glsl -profile glsl_450 -entry vertMain -o %SPV_FILE_PATH%.vert
    REM %SLANG_EXE% %FILE% -target glsl -profile glsl_450 -entry fragMain -o %SPV_FILE_PATH%.frag

    IF NOT EXIST "%CURR_DIR%cmake-build-debug/assets/shaders" (
        MKDIR "%CURR_DIR%cmake-build-debug/assets/shaders"
    )

    IF NOT EXIST "%CURR_DIR%out/build/x64-debug/assets/shaders" (
        MKDIR "%CURR_DIR%out/build/x64-debug/assets/shaders"
    )

    COPY /Y %SPV_FILE_PATH% "%CURR_DIR%cmake-build-debug/assets/shaders\%SPV_FILE%"

    REM COPY /Y %SPV_FILE_PATH%.vert "%CURR_DIR%cmake-build-debug/assets/shaders\%SPV_FILE%.vert"
    REM COPY /Y %SPV_FILE_PATH%.frag "%CURR_DIR%cmake-build-debug/assets/shaders\%SPV_FILE%.frag"
)

:END
