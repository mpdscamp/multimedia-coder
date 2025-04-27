@ECHO OFF
SETLOCAL

REM --- Configuration ---
SET EXECUTABLE=build\Release\arithmetic_coder.exe
SET INPUT_DIR=input
SET RESULTS_DIR=results

REM --- Create results directory if it doesn't exist ---
IF NOT EXIST "%RESULTS_DIR%" (
    ECHO Creating results directory: %RESULTS_DIR%
    MKDIR "%RESULTS_DIR%"
)

ECHO ============================================================
ECHO Running ALL Encode/Decode Commands Sequentially
ECHO ============================================================
ECHO.

REM --- Lena ---
ECHO --- Processing lena_ascii ---
ECHO Encoding...
%EXECUTABLE% encode "%INPUT_DIR%\lena_ascii.pgm" "%RESULTS_DIR%\lena_ascii.codestream"
ECHO Decoding...
%EXECUTABLE% decode "%RESULTS_DIR%\lena_ascii.codestream" "%RESULTS_DIR%\lena_ascii-rec.pgm"
ECHO Done with lena_ascii.
ECHO.

REM --- Baboon ---
ECHO --- Processing baboon_ascii ---
ECHO Encoding...
%EXECUTABLE% encode "%INPUT_DIR%\baboon_ascii.pgm" "%RESULTS_DIR%\baboon_ascii.codestream"
ECHO Decoding...
%EXECUTABLE% decode "%RESULTS_DIR%\baboon_ascii.codestream" "%RESULTS_DIR%\baboon_ascii-rec.pgm"
ECHO Done with baboon_ascii.
ECHO.

REM --- Quadrado ---
ECHO --- Processing quadrado_ascii ---
ECHO Encoding...
%EXECUTABLE% encode "%INPUT_DIR%\quadrado_ascii.pgm" "%RESULTS_DIR%\quadrado_ascii.codestream"
ECHO Decoding...
%EXECUTABLE% decode "%RESULTS_DIR%\quadrado_ascii.codestream" "%RESULTS_DIR%\quadrado_ascii-rec.pgm"
ECHO Done with quadrado_ascii.
ECHO.

ECHO Running Python Script for Analysis
py %RESULTS_DIR%\analyze_results.py "%RESULTS_DIR%"
IF ERRORLEVEL 1 (
    ECHO Error: Python script failed.
    EXIT /B 1
)

ECHO ============================================================
ECHO All Commands Executed.
ECHO ============================================================

ENDLOCAL