#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <windows.h>

#include "HEADER/fileControl.h"
#include "HEADER/logic.h"
#include "HEADER/databaseControl.h"
#include "HEADER/webpageControl.h"

using namespace std;

list<websiteCustomer> websiteCustomerCell;
list<websiteCustomer>::iterator g_it;

list<websiteInvoice> websiteInvoiceCell;

list<int> DB_CustomerIDList;
int DB_CustomerID;
int DB_RetSize;

static int backupSite(list<string> websiteFiles);
static int backupDB(SettingsObject settings, char* databaseDir);
static int startupDB (string databaseDir);

static int callbackSize(void* NotUsed, int argc, char** argv, char** azColName);
static bool invoiceExsists(char* s, string fileDir, SettingsObject settings);
static int createInvoice(char* dir, string fileName, string fileDir, int date, int year, int month, int day, int devFound, int customerID);
static int deleteAllNotFound(char* s);

static int callbackCustomerID(void* NotUsed, int argc, char** argv, char** azColName);
static bool customerExsists(char* s, string devName);
static int createNewCustomer(char* dir, string devName, string name, string cat);
static int findCustomer(char* s, string devName, string name, string cat);
static int removeUnusedCustomers(char* s);
static int callbackReadInvoice(void* NotUsed, int argc, char** argv, char** azColName);

static string createWebpageContent(char* s, char letter);

int main () 
{
	// -- general ------------------------
	// return buffer
	int exit;

	// -- settings -----------------------
	// settings name
	string settingsDir = "settings.ini";

	// import settings from settings.ini
	list<string> settingsFileLines = getFileLines(settingsDir);
	SettingsObject settings = getSettings(settingsFileLines);


	// -- database -----------------------
	// database name
	char* databaseDir = new char [sizeof(settings.outputDir) + sizeof("database.db")]; 
	strcpy(databaseDir, settings.outputDir); strcat(databaseDir, "database.db");

	if (settings.debug) {
		cout << "\n" << "- database" << endl;
		cout << "database\t: " << databaseDir << endl;
	}

	//backup database
	if (settings.backupDb) 
		backupDB(settings, databaseDir);

	// -- webpage ------------------------
	const char* websiteDir = "G:/website/billing/";
	list<string> websiteFiles = getWebsiteFiles(websiteDir);

	if (settings.debug) 
		cout << "\n" << "- website" << endl;
		cout << "websiteDir\t: " << websiteDir << endl;
	
	// website backup
	if (settings.backupSite)
		backupSite(websiteFiles);


	// ---- START PROGRAM ----------------

	if (settings.processFiles) {
		// setup database
		if (settings.debug) {
			cout << "\n" << "- start processing" << endl;
		}

		exit = startupDB(databaseDir);

		// loop through source list
		int fileCount = 0;
		list<string>::iterator it;
		for (it = settings.sourceDirs.begin(); it != settings.sourceDirs.end(); ++it) {
			// loop through all files in dirrectory
			for(const auto & entry : fs::directory_iterator(*it)){

				// file info
				fs::path path = entry.path();
				string filedir = path.string();

				cout << "\rfiles completed " << fileCount;

				if (!invoiceExsists(databaseDir, filedir, settings)) {
					//cout << "\n" << "invoice doesnt exsist" << endl;
					string filename = path.filename().string();

					Invoice fileInfo = processName(filedir, filename);

					// get customer ID, if customer doesnt exsist, create it
					int customerID = findCustomer(databaseDir, fileInfo.customerDevName, fileInfo.customerName, fileInfo.category);


					int exit = createInvoice(databaseDir, fileInfo.fileName, fileInfo.fileDir, fileInfo.date, fileInfo.year, fileInfo.month, fileInfo.day, settings.removeDeleted ? 1 : 0, customerID);

					delete fileInfo.customerName;
					delete fileInfo.customerDevName;
					delete fileInfo.category;

				} 

				fileCount++;
			}
		}
		cout << "\n" << endl;

		if (settings.removeDeleted) {
			exit = deleteAllNotFound(databaseDir);
			exit = removeUnusedCustomers(databaseDir);
		}
	}

	if (settings.createWebpages) {
		// template
		string templateWebpageDir = (string)settings.templateDir + (string)"TemplateWebPage.html";
		list<string> templateContent = getFileLines(templateWebpageDir);
		
		// alphabet loop
		string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		char letter;
		for (int alphabet_IT = 0; alphabet_IT < alphabet.length(); alphabet_IT++) {
			letter = alphabet[alphabet_IT];
			cout << "\rformatting section " << (string){letter};

			// new webpage file
			string webpageFileName = (string)"PB_" + (string){letter} + (string)".html";
			string webpageFileDir = (string)settings.outputDir + (string)webpageFileName;
			exit = createFile(webpageFileDir);

			ofstream webpageFile(webpageFileDir.c_str(), ios::app);

			// loop through template file lines
			list<string>::iterator it;
			for (it = templateContent.begin(); it != templateContent.end(); ++it) {
				if (regexExists("\\[letter\\]", *it))
					webpageFile << letter;
				else if (regexExists("\\[content\\]", *it))
					webpageFile << createWebpageContent(databaseDir, letter);
				else 
					webpageFile << *it;

 			} 

			if (settings.updateSite) {
				fileCopy(webpageFileDir, ((string)settings.websiteDir + (string)webpageFileName));
			}

		}

	}


	return 0;
}

static int backupSite(list<string> websiteFiles) 
{
	int exit;

	const char* backupDir = "BACKUP/SITE/";
	list<string> backupFiles = getWebsiteFiles(backupDir);

	cout << "Backing up Website Billing to \"" << backupDir << "\"" << endl;
	int i = 0;
	for (i = 26; i >= 0; ++i ) {
		string srcDir = websiteFiles.front();
		string dstDir = backupFiles.front();

		websiteFiles.pop_front();
		backupFiles.pop_front();

		exit = fileCopy(srcDir, dstDir);
	}


	return 0;
}

static int backupDB(SettingsObject settings, char* databaseDir) 
{
	string databaseBackupDir = "BACKUP/database_backup.db";

	if (settings.debug)
		cout << "databaseBackup\t: " << databaseBackupDir << endl;

	cout << "Backing up Database to \"" << databaseBackupDir << "\"" << endl;
	int exit = fileCopy(databaseDir, databaseBackupDir);

	return 0;
}

static int startupDB (string databaseDir)
{
	int exit;
	const char* dir = databaseDir.c_str();

	string sql;

	// create database if not exsists
	exit = createDB(dir);

	// create table "customers" if not exsists
	sql = "CREATE TABLE IF NOT EXISTS customers("
		"customerID INTEGER PRIMARY KEY AUTOINCREMENT, "
		"name 		TEXT NOT NULL, "
		"devName 	TEXT NOT NULL, "
		"category 	CHARACTER(1) NOT NULL );";
	exit = sqliteExecute(dir, sql);

	// create table "invoices" if not exsists
	sql = "CREATE TABLE IF NOT EXISTS invoices("
		"invoiceID 	INTEGER PRIMARY KEY AUTOINCREMENT, "
		"fileName 	TEXT NOT NULL, "
		"fileDir 	TEXT NOT NULL, "
		"date 		INT NOT NULL, "
		"year 		INT NOT NULL, "
		"month 		INT NOT NULL, "
		"day 		INT NOT NULL, "
		"devFound 	INT NOT NULL, "
		"customerID INT NOT NULL );";
	exit = sqliteExecute(dir, sql);

	return 0;
}

// invoice
static int callbackSize(void* NotUsed, int argc, char** argv, char** azColName) 
{
	DB_RetSize = argc;

	return 0;
}

static bool invoiceExsists(char* s, string fileDir, SettingsObject settings) 
{
	sqlite3* DB;
	int exit = sqlite3_open(s, &DB);

	string sql = "SELECT * FROM invoices WHERE fileDir=\"" + fileDir + "\";";

	sqlite3_exec(DB, sql.c_str(), callbackSize, NULL, NULL);

	if (DB_RetSize >= 1) {
		DB_RetSize = 0;

		if (settings.removeDeleted) {
			sql = "UPDATE invoices SET devFound = 1 WHERE fileDir = \"" + fileDir + "\";";
			sqlite3_exec(DB, sql.c_str(), callbackSize, NULL, NULL);
		}

		sqlite3_close(DB);

		return true;
	}

	sqlite3_close(DB);

	return false;
}

static int createInvoice(char* dir, string fileName, string fileDir, int date, int year, int month, int day, int devFound, int customerID)
{
	int exit;

	string sql = "INSERT INTO invoices (fileName, fileDir, date, year, month, day, devFound, customerID) VALUES (\"" + fileName + "\", \"" + fileDir + "\", " + to_string(date) + ", " + to_string(year) + ", " + to_string(month) + ", " + to_string(day) + ", " + to_string(devFound) + ", " + to_string(customerID) + ");";
	exit = sqliteExecute(dir, sql);

	cout << "\rnew Invoice\t\t" << fileName << endl;

	return 0;
}

static int callbackReadInvoice(void* NotUsed, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++) {
		DB_CustomerIDList.push_back(atoi(argv[i]));
	}

	return 0;
}

static int removeUnusedCustomers(char* s) 
{

	cout << "deleting unused customers from database" << endl;

	sqlite3* DB;
	int exit = sqlite3_open(s, &DB);

	string sql = "SELECT customerID FROM customers;";
	exit = sqlite3_exec(DB, sql.c_str(), callbackReadInvoice, NULL, NULL);

	list<int>::iterator it;
	for (it = DB_CustomerIDList.begin(); it != DB_CustomerIDList.end(); ++it) {
		DB_RetSize = 0;

		sql = "SELECT * FROM invoices WHERE customerID = " + to_string(*it) + ";";
		exit = sqlite3_exec(DB, sql.c_str(), callbackSize, NULL, NULL);

		if (DB_RetSize <= 0) {
			sql = "DELETE FROM customers WHERE customerID = " + to_string(*it) + ";";
			exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, NULL);
		} 
	}

	exit = sqlite3_close(DB);

	return 0;
}

static int deleteAllNotFound(char* s) 
{
	cout << "deleting unfound directorys from database" << endl;

	int exit;
	string sql = "DELETE FROM invoices WHERE devFound = 0;";
	exit = sqliteExecute(s, sql);

	sql = "UPDATE invoices SET devFound = 0 WHERE devFound = 1;";
	exit = sqliteExecute(s, sql);

	return 0;
}


// customer
static int callbackCustomerID(void* NotUsed, int argc, char** argv, char** azColName)
{
	DB_RetSize = argc;

	if (argc > 0)
		DB_CustomerID = atoi(argv[0]);


	return 0;
}

static bool customerExsists(char* s, string devName) 
{
	sqlite3* DB;
	int exit = sqlite3_open(s, &DB);

	string sql = "SELECT * FROM customers WHERE devName='" + devName + "';";

	sqlite3_exec(DB, sql.c_str(), callbackCustomerID, NULL, NULL);

	if (DB_RetSize >= 1) {
		DB_RetSize = 0;

		sqlite3_close(DB);
		return true;
	}

	DB_CustomerID = 0;

	sqlite3_close(DB);
	return false;
}

static int createNewCustomer(char* dir, string devName, string name, string cat)
{
	int exit;

	string sql = "INSERT INTO customers (name, devName, category) VALUES (\"" + name + "\", \"" + devName + "\", \"" + cat.c_str()[0] + "\");";
	exit = sqliteExecute(dir, sql);

	cout << "\rnew Customer\t" << name << endl;

	return 0;
}

static int findCustomer(char* s, string devName, string name, string cat) 
{
	int exit;

	if (!customerExsists(s, devName)) {
		exit = createNewCustomer(s, devName, name, cat);
		exit = customerExsists(s, devName);
	}

	return DB_CustomerID;
}

int callbackWebsiteCustomer(void* NotUsed, int argc, char** argv, char** azColName)
{


	websiteCustomer tempCustomer;
	tempCustomer.name = argv[0];
	tempCustomer.ID = argv[1];

	websiteCustomerCell.push_back(tempCustomer);
	return 0;
}

int callbackWebsiteInvoice(void* NotUsed, int argc, char** argv, char** azColName)
{
	websiteInvoice tempInvoice;
	tempInvoice.year = argv[0];
	tempInvoice.month = argv[1];
	tempInvoice.day = argv[2];
	tempInvoice.dir = argv[3];

	(*g_it).invoices.push_back(tempInvoice);
	return 0;
}

static string createWebpageContent(char* s, char letter)
{
	websiteCustomerCell.clear();

	const string months[12] = {
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};

	string returnString;
	int exit;

	sqlite3* DB;
	exit = sqlite3_open(s, &DB);

	// get all customers
	string sql = "SELECT name, customerID FROM customers WHERE upper(category) = '" + (string){letter} + "' ORDER BY devName ASC;";
	exit = sqlite3_exec(DB, sql.c_str(), callbackWebsiteCustomer, NULL, NULL);

	// get all invoices
	for (g_it = websiteCustomerCell.begin(); g_it != websiteCustomerCell.end(); ++g_it) {
		sql = "SELECT year, month, day, fileDir FROM invoices WHERE customerID = " + (*g_it).ID + " ORDER BY date DESC;";
		exit = sqlite3_exec(DB, sql.c_str(), callbackWebsiteInvoice, NULL, NULL);
	}

	exit = sqlite3_close(DB);

	// formating
	list<websiteCustomer>::iterator c_it;
	for (c_it = websiteCustomerCell.begin(); c_it != websiteCustomerCell.end(); ++c_it) {
		returnString.append("<h3>" + (*c_it).name + "</h3>");

		returnString.append("<ul>");
		list<websiteInvoice>::iterator i_it;
		for (i_it = (*c_it).invoices.begin(); i_it != (*c_it).invoices.end(); ++i_it){
			returnString.append("<li><a href=\"" + (*i_it).dir + "\" target=\"_blank\">");

			if (atoi((*i_it).month.c_str()) > 0 && atoi((*i_it).month.c_str()) <= 12)
				returnString.append(months[atoi((*i_it).month.c_str()) - 1] + " " + (*i_it).day + ", " + (*i_it).year );
			else
				returnString.append("error getting date");

			returnString.append("</a></li>");
		}

		returnString.append("</ul>");

		(*c_it).invoices.clear();
	}

	return returnString;
}