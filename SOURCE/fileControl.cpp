#include <windows.h>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;
#include <iostream>
#include <string>
#include <list>

using namespace std;

static int fileControlTest();
static int createFile (string fileDir);
static list<string> getFileLines(const char* file);
static int fileCopy(string srcDir, string dstDir);

static int fileControlTest() 
{
	std::cout << "test from fileControl.cpp" << std::endl;

	return 0;
}

static int createFile (string fileDir) 
{
	HANDLE h = CreateFile(fileDir.c_str(),	// name of the file
                          GENERIC_WRITE, 	// open for writing
                          0,             	// sharing mode, none in this case
                          0,             	// use default security descriptor
                          CREATE_ALWAYS, 	// overwrite if exists
                          FILE_ATTRIBUTE_NORMAL,
                          0);

	if (h)
    {
        CloseHandle(h);
    }
    else
    {
        std::cerr << "CreateFile() failed:" << GetLastError() << "\n";
    }
    return 0;

}

static list<string> getFileLines (string fileDir) 
{
	const char* file = fileDir.c_str();
	ifstream myfile; myfile.open(file);

	list<string> mylinelist;

	string myline;

	if ( myfile.is_open() ) {
		while ( myfile ) {
			getline (myfile, myline);
			mylinelist.push_back(myline);
		}
	}

	return mylinelist;
}

static int fileCopy(string srcDir, string dstDir)
{
	ifstream  src(srcDir, ios::binary);
	ofstream  dst(dstDir, ios::binary);
	
	dst << src.rdbuf();
	return 0;
}


static list<string> getWebsiteFiles(const char* websiteDir)
{
	const string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	list<string> fileList;

	for (int i = alphabet.length() - 1; i >= 0; i--) {
		string fileName;
		string prefix = "PB_";
		string extention = ".html";

		string letter = {alphabet[i]}; 

		fileName.append(websiteDir);
		fileName.append(prefix);
		fileName.append(letter);
		fileName.append(extention);

		fileList.push_front(fileName);
	}

	return fileList;
}