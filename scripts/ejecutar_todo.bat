@echo off
REM Compila el programa y ejecuta las 3 instancias con la configuracion base (Windows)
g++ -std=c++17 -O2 -Iinclude src\main.cpp src\Instancia.cpp src\PFSP.cpp src\AlgoritmoGenetico.cpp -o ga_pfsp.exe
if errorlevel 1 exit /b 1
for %%f in (instancias\*.txt) do ga_pfsp.exe %%f
