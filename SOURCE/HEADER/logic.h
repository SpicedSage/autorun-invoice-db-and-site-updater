#include "../logic.cpp"
using namespace std;

static int logicTest();
static SettingsObject getSettings(list<string> fileLines);
static Invoice processName(string fileDir, string fileName);

static int regexExists(const string reString, string line);