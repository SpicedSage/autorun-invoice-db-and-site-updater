#include "../fileControl.cpp"
using namespace std;

static int fileControlTest();
static int createFile (string fileDir);
static list<string> getFileLines(string file);
static int fileCopy(string src, string dst);
static list<string> getWebsiteFiles(char* websiteDir);