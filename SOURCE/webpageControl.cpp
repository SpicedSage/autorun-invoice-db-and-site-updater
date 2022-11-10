#include <windows.h>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;
#include <iostream>
#include <string>
#include <list>

using namespace std;

class websiteInvoice {
public:
	string year;
	string month;
	string day;
	string dir;
};

class websiteCustomer {
public:
	string name;
	string ID;
	list<websiteInvoice> invoices;
};


static int webpageControlTest();

static int webpageControlTest() 
{
	std::cout << "test from webpageControl.cpp" << std::endl;

	return 0;
}