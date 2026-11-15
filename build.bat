@echo off
REM =========================================================================
REM  ADAPTIVE TRAFFIC MANAGEMENT SYSTEM - Windows build
REM
REM    build.bat          build the server and all three test suites
REM    build.bat run      build and start the server
REM    build.bat test     build and run all three test suites
REM    build.bat clean    delete build output
REM
REM  Requires g++ (MinGW) on your PATH.
REM  -lws2_32 links Winsock, which the HTTP server needs on Windows.
REM =========================================================================

setlocal

set CXX=g++
set FLAGS=-std=c++17 -Wall -Wextra -Iinclude
set LIBS=-lws2_32

set M1=include/PriorityManager.cpp include/TrafficManager.cpp include/TrafficSimulation.cpp
set M2=database/Database.cpp database/FileStorage.cpp database/RoadRepository.cpp database/TrafficRepository.cpp database/SignalRepository.cpp database/EmergencyRepository.cpp
set M4=integration/traffic.cpp integration/Json.cpp integration/HttpServer.cpp integration/ErrorHandler.cpp integration/Integration.cpp integration/API.cpp integration/RequestHandler.cpp
set ALL=%M1% %M2% %M4%

if "%1"=="clean" goto clean
if "%1"=="run"   goto buildrun
if "%1"=="test"  goto buildtest

:buildall
echo Building traffic_server.exe ...
%CXX% %FLAGS% main.cpp %ALL% -o traffic_server.exe %LIBS%
if errorlevel 1 goto failed

echo Building tests_m1.exe ...
%CXX% %FLAGS% test/test.cpp %M1% -o tests_m1.exe
if errorlevel 1 goto failed

echo Building tests_m2.exe ...
%CXX% %FLAGS% database/database_test.cpp %M2% integration/traffic.cpp -o tests_m2.exe
if errorlevel 1 goto failed

echo Building tests_m4.exe ...
%CXX% %FLAGS% integration/integration_test.cpp %ALL% -o tests_m4.exe %LIBS%
if errorlevel 1 goto failed

echo.
echo Build complete.
echo   build.bat run    start the server
echo   build.bat test   run the test suites
goto end

:buildrun
call "%~f0"
if errorlevel 1 goto end
echo.
echo Starting server. Open http://127.0.0.1:8080/ in a browser.
traffic_server.exe
goto end

:buildtest
call "%~f0"
if errorlevel 1 goto end
echo.
echo --- MEMBER 1: Traffic Intelligence ---
tests_m1.exe
echo.
echo --- MEMBER 2: Database ---
tests_m2.exe
echo.
echo --- MEMBER 4: Integration ---
tests_m4.exe
goto end

:clean
del /q traffic_server.exe tests_m1.exe tests_m2.exe tests_m4.exe 2>nul
rmdir /s /q traffic_data 2>nul
rmdir /s /q test_db_data 2>nul
rmdir /s /q test_integration_data 2>nul
echo Cleaned.
goto end

:failed
echo.
echo BUILD FAILED. Check the compiler errors above.
exit /b 1

:end
endlocal
