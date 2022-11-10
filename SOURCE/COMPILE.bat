@ECHO OFF
rem Compile code into one exe
rem ------------------------------
rem COMPILE FILES

rem File
@ECHO ON
g++ -c fileControl.cpp ^
-o OBJECT/fileControl.o

@ECHO OFF
rem Logic
@ECHO ON
g++ -c logic.cpp ^
-o OBJECT/logic.o

@ECHO OFF
rem Database
@ECHO ON
g++ -c databaseControl.cpp ^
-o OBJECT/databaseControl.o

@ECHO OFF
rem Webpage
@ECHO ON
g++ -c webpageControl.cpp ^
-o OBJECT/webpageControl.o

@ECHO OFF
rem ------------------------------
rem COMPILE MAIN
@ECHO ON
g++ -c main.cpp ^
-o OBJECT/main.o

@ECHO OFF
rem ------------------------------
rem LINK ALL
@ECHO ON
g++ ^
OBJECT/main.o ^
OBJECT/logic.o ^
OBJECT/databaseControl.o ^
OBJECT/webpageControl.o ^
OBJECT/fileControl.o ^
LIBRARY/sqlite3.o ^
-o ../debug