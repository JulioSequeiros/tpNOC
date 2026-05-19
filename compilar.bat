@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion

echo ========================================
echo   Compilando Sistema Mini NOC
echo ========================================
echo.

REM ================= CRIAR PASTA OUTPUT SE NAO EXISTIR =================
if not exist output (
    mkdir output
)

echo A compilar...

gcc -Wall -Wextra -g3 main.c module_2.c module_4.c module_6.c -o output\main.exe

if %errorlevel% == 0 (
    echo.
    echo ========================================
    echo   COMPILACAO CONCLUIDA COM SUCESSO!
    echo   Executavel: output\main.exe
    echo ========================================
    echo.
    echo Executando o programa...
    echo.
    output\main.exe
) else (
    echo.
    echo ========================================
    echo   ERRO NA COMPILACAO!
    echo   Verifique:
    echo   - main.c
    echo   - module_2.c
    echo   - module_4.c
    echo   - module_6.c
    echo   - noc.h
    echo ========================================
    echo.
    pause
)