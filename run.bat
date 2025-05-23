@ECHO OFF
SETLOCAL

SET EXECUTABLE=build\arithmetic_coder.exe
SET INPUT_DIR=input
SET RESULTS_DIR=results

IF NOT EXIST "%RESULTS_DIR%" (
    ECHO Creating results directory: %RESULTS_DIR%
    MKDIR "%RESULTS_DIR%"
)

ECHO ============================================================
ECHO Running Encode/Decode Commands
ECHO ============================================================
ECHO.

ECHO --- Processing lena_ascii ---
ECHO Encoding...
%EXECUTABLE% encode "%INPUT_DIR%\lena_ascii.pgm" "%RESULTS_DIR%\lena_ascii.codestream"
ECHO Decoding...
%EXECUTABLE% decode "%RESULTS_DIR%\lena_ascii.codestream" "%RESULTS_DIR%\lena_ascii-rec.pgm"
ECHO Done with lena_ascii.
ECHO.

ECHO --- Processing baboon_ascii ---
ECHO Encoding...
%EXECUTABLE% encode "%INPUT_DIR%\baboon_ascii.pgm" "%RESULTS_DIR%\baboon_ascii.codestream"
ECHO Decoding...
%EXECUTABLE% decode "%RESULTS_DIR%\baboon_ascii.codestream" "%RESULTS_DIR%\baboon_ascii-rec.pgm"
ECHO Done with baboon_ascii.
ECHO.

ECHO --- Processing quadrado_ascii ---
ECHO Encoding...
%EXECUTABLE% encode "%INPUT_DIR%\quadrado_ascii.pgm" "%RESULTS_DIR%\quadrado_ascii.codestream"
ECHO Decoding...
%EXECUTABLE% decode "%RESULTS_DIR%\quadrado_ascii.codestream" "%RESULTS_DIR%\quadrado_ascii-rec.pgm"
ECHO Done with quadrado_ascii.
ECHO.

ECHO ============================================================
ECHO All Commands Executed.
ECHO ============================================================

ENDLOCAL