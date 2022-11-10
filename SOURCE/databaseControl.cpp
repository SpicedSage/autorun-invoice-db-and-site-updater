#include <iostream>
#include <string>
#include "LIBRARY/sqlite3.h"

using namespace std;

// public
static int databaseControlTest();
static int createDB(const char* s);
static int sqliteExecute(const char* s, string sql);
static int sqliteSelect(const char* s, string sql);

//private
static int callbackPrint(void* NotUsed, int argc, char** argv, char** azColName);

static int databaseControlTest() 
{
	cout << "test from databaseControl.cpp" << endl;

	return 0;
}

static int createDB(const char* s) 
{
	sqlite3* DB;
	int exit = 0;

	exit = sqlite3_open(s, &DB);
	
	sqlite3_close(DB);

	return 0;
}

static int sqliteExecute(const char* s, string sql)
{
	sqlite3* DB;
	char* messageError;

	int exit = sqlite3_open(s, &DB);

	exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messageError);
	if (exit != SQLITE_OK) {
		cerr << "Error executing: \"" << sql << "\"" << endl;
		sqlite3_free(messageError);
	}

	sqlite3_close(DB);

	return 0;
}

static int sqliteSelect(const char* s, string sql) {
	sqlite3* DB;

	int exit = sqlite3_open(s, &DB);

	sqlite3_exec(DB, sql.c_str(), callbackPrint, NULL, NULL);


	sqlite3_close(DB);

	return 0;
}

static int callbackPrint(void* NotUsed, int argc, char** argv, char** azColName) 
{
	for (int i = 0; i < argc; i++) {
		cout << azColName[i] << ": " << argv[i] << endl;
	}

	cout << endl;

	return 0;
}