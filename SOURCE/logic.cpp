#include <iostream>
#include <string>
#include <list>
#include <regex>
#include <cmath>

using namespace std;

class SettingsObject 
{
	public:
		bool debug;
		bool backupSite;
		bool backupDb;
		bool updateSite;
		bool processFiles;
		bool removeDeleted;
		bool createWebpages;
		char* templateDir;
		char* outputDir;
		char* websiteDir;
		list<string> sourceDirs;

		SettingsObject () {
			debug = false;
			backupSite = false;
			backupDb = false;
			updateSite = false;
		}
};

class Invoice {
	public:
		char* customerName;
		char* customerDevName;

		char* category;

		string fileDir;
		string fileName;

		int date;
		int year;
		int month;
		int day;
};

static int logicTest();
static SettingsObject getSettings(list<string> fileLines);
static Invoice processName(string fileDir, string fileName);

static int regexExists(const string reString, string line);

static int logicTest() 
{
	std::cout << "test from logic.cpp" << std::endl;

	return 0;
}


// -- REGEX
// private
static int regexExists(const string reString, string line) 
{

	smatch match;
	regex re(reString);
	if (regex_search(line, match, re) && match.size() >= 1) 
		return 1;
	return 0; 
} 

static int regexBool(bool& variable, const string reString, string line) 
{
	
	smatch match;
	regex re(reString);
	if (regex_search(line, match, re) && match.size() >= 1) {
		const char* matchFirst = match.str(1).c_str();

		if (strcmp(matchFirst, "true") == 0) 
			variable = true;
		else
			variable = false;

		return 1;
	}

	return 0;
}

static int regexReplaceString (char*& variable, const string reString, string line, string replace) 
{
	regex re(reString);
	string returnString;

	returnString = regex_replace(line, re, replace);
	variable = new char [returnString.length() + 1];

	strcpy(variable, returnString.c_str());

	return 0;
}

static int regexString(char*& variable, const string reString, string line, const char* default_text, bool default_textCheck) 
{

	smatch match;
	regex re(reString);

	if (regex_search(line, match, re) && match.size() >= 1) {

		string matchFirst = match.str(1);

		variable = new char [matchFirst.length() + 1];
		strcpy(variable, matchFirst.c_str());

		return 1;
	}

	if (default_textCheck) {
		variable = new char [strlen(default_text) + 1];
		strcpy(variable, default_text);
	}

	return 0;
}

static int regexStringAddList(list<string>& variable, const string reString, string line) 
{
	
	smatch match;
	regex re(reString);
	if (regex_search(line, match, re) && match.size() >= 1) 
		variable.push_back(match.str(1));

	return 1;
}


// -- Get Settings
// private
static int settingsLineCheck(string line, SettingsObject& settings, int lastExit) 
{
	if ( line[0] == '#' ) 
		return lastExit;

	if (!lastExit) {
		if (regexBool(settings.debug, "debugMode = (\\w{4})", line) ||
			regexBool(settings.backupSite, "backupSite = (\\w{4})", line) ||
			regexBool(settings.backupDb, "backupDB = (\\w{4})", line) ||
			regexBool(settings.updateSite, "updateSite = (\\w{4})", line)||
			regexBool(settings.processFiles, "processFiles = (\\w{4})", line)||
			regexBool(settings.removeDeleted, "removeDeleted = (\\w{4})", line)||
			regexBool(settings.createWebpages, "createWebpages = (\\w{4})", line)||
			regexString(settings.templateDir, "templates = \"(.*)\"", line, "", false)||
			regexString(settings.websiteDir, "website = \"(.*)\"", line, "", false)||
			regexString(settings.outputDir, "output = \"(.*)\"", line, "", false) ) 
			return 0;
		else if (regexExists("source = \\[", line))
			return 1;
	}
	else if (line[0] == ']')
		return 0;
	else if (regexStringAddList(settings.sourceDirs, "\"(.*)\"", line))
		return lastExit; 

	return lastExit;
}

// public
static SettingsObject getSettings(list<string> fileLines) 
{
	SettingsObject settings;

	int exit = 0;

	list<string>::iterator it;
	for (it = fileLines.begin(); it != fileLines.end(); ++it)
		exit = settingsLineCheck(*it, settings, exit);

	if (settings.debug) {
		cout << "- settings.ini" << endl;

		cout << "debugMode\t: " << settings.debug << endl;
		cout << "backupDB\t: " << settings.backupDb << endl;
		cout << "backupSite\t: " << settings.backupSite << endl;
		cout << "processFiles\t: " << settings.processFiles << endl;
		cout << "removeDeleted\t: " << settings.removeDeleted << endl;
		cout << "createWebpages\t: " << settings.createWebpages << endl;
		cout << "updateSite\t: " << settings.updateSite << endl; 
		cout << "websiteDir\t: \"" << settings.websiteDir << "\"" << endl;
		cout << "templateDir\t: \"" << settings.templateDir << "\"" << endl;
		cout << "outputDir\t: \"" << settings.outputDir << "\"" << endl;

		cout << "sourceDirs\t: ";
		
		list<string>::iterator it;
		for (it = settings.sourceDirs.begin(); it != settings.sourceDirs.end(); ++it) {
			if (it == settings.sourceDirs.begin())
				cout << "\"" << *it << "\"" << endl;
			else
				cout << "\t\t  \"" << *it << "\"" << endl;
		}

		if (settings.sourceDirs.begin() == settings.sourceDirs.end())
			cout << "\"\"" << endl;
	}

	return settings;
}

// -- Process Information from filename
// private
static int stringToLower(char*& variable, char* source) {

	for (int i = 0; i < strlen(source); i++)
		variable[i] = (char) tolower(source[i]);

	return 0;
}

static bool compare(int compare, int min, int max)
{
	if (compare > min && compare <= max)
		return true;

	return false;
}

static int subInt(int base, int start, int digits) 
{
	int result;
	result = floor(base / pow(10, abs(digits - start)));
	result %= (int)pow(10, digits);

	return result;
}

static int setDate(Invoice& variable, int year, int month, int day)
{
	variable.year  = year;
	variable.month = month;
	variable.day   = day;
	variable.date  = (year * 10000) + (month * 100) + (day);

	return 0;
}

static int getDate(Invoice& variable, int date) 
{
	// return
	int exit;

	// constants
	int foundedyear = 1990;
	int currentyear = 2022;

	// store values
	int year;
	int month;
	int day;

	// LOGIC
	int start = 8;

	// YEAR
	int yearOne = subInt(date, (start - 0), 4);
	int yearTwo = subInt(date, (start - 4), 4);
	if      ( compare(yearOne, foundedyear, currentyear) ) {
		year = yearOne;
		start -= 4;
	}
	else if ( compare(yearTwo, foundedyear, currentyear) ) {
		year = yearTwo;
		start -= 0;
	}
	else {
		exit = setDate(variable, 0, 0, 0);
		return 0;
	}

	// Month
	int monthOne = subInt(date, (start - 0), 2);
	int monthTwo = subInt(date, (start - 2), 2);
	if 		( compare(monthOne, 0, 12) ) {
		month = monthOne;
		start -= 2;
	}
	else if ( compare(monthTwo, 0, 12) ) {
		month = monthTwo;
		start -= 0;
	}
	else {
		exit = setDate(variable, 0, 0, 0);
		return 0;
	}

	// Day
	int dayOne = subInt(date, (start - 0), 2);	// MMDD
	if 		( compare(dayOne, 0, 31) ){
		day = dayOne;
	}
	else {
		exit = setDate(variable, 0, 0, 0);
		return 0;
	}

	exit = setDate(variable, year, month, day);

	return 0;
}

// public
static Invoice processName(string fileDir, string fileName) 
{
	int exit;
	Invoice fileInfo;

	// file Dir
	fileInfo.fileDir = fileDir;
		
	// file Name
	fileInfo.fileName = fileName;
	// customer Name
	exit = regexReplaceString(fileInfo.customerName, "_?(?=\\d{3,}).*", fileName, "");
	exit = regexReplaceString(fileInfo.customerName, "[_-]+", fileInfo.customerName, " ");

	if (strlen(fileInfo.customerName) <= 0) {
		fileInfo.customerName = new char[sizeof("UNKNOWN") + 1];
		strcpy(fileInfo.customerName, "UNKNOWN");
	}

	exit = stringToLower(fileInfo.customerName, fileInfo.customerName);

	// customer devName
	exit = regexReplaceString(fileInfo.customerDevName, "\\W", fileInfo.customerName, "");

	//cout << fileDir << endl;
	fileInfo.category = new char;
	fileInfo.category[0] = fileInfo.customerDevName[0];

	// date
	char* dateRaw;
	// exit = regexString(dateRaw, "(\\d{4}[-_]\\d{4}).*", fileName, "0000-0000", true);
	exit = regexString(dateRaw, "\\D*\\d{0,2}[_\\-\\s\\D]+?\\d{0,2}(\\d{4}[-_\\s]\\d{4}).*", fileName, "0000-0000", true);
	exit = regexReplaceString(dateRaw, "[_-]+", dateRaw, "");

	//cout << dateRaw << endl;
	//cout << subInt(atoi(dateRaw), 4, 4) << endl;

	exit = getDate(fileInfo, atoi(dateRaw));

	delete dateRaw;

	return fileInfo;
}