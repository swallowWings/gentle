#pragma once
#include <iostream>
#include <stdio.h>
#include <map>
//#include <list>
#include<fstream>
#include <filesystem>
#include<ATLComTime.h>
#include <windows.h>
//#include <string>

using namespace std;
namespace fs = std::filesystem;

#define ENUM_TO_STR(ENUM) std::string(#ENUM)
const int CONST_BIG_SIZE_ARRAY_THRESHOLD = 200000000;

typedef struct _ascRasterExtent
{
	double bottom=0.0;
	double top = 0.0;
	double left = 0.0;
	double right = 0.0;
	double extentWidth = 0.0;
	double extentHeight = 0.0;
} ascRasterExtent;

typedef struct _ascRasterHeader
{
	int nCols=0;
	int nRows = 0;
	double xllcorner = 0.0;
	double yllcorner = 0.0;
	double dx = 0.0;
	double dy = 0.0;
	double cellsize = 0.0;
	int nodataValue = 0;
	int headerEndingLineIndex = 0;
	int dataStartingLineIndex = 0;
} ascRasterHeader;

typedef struct _cellPosition
{
	int x = 0;
	int y = 0;
} cellPosition;

//typedef struct dateTime
//{
//	int year;
//	int month;
//	int day;
//	int hours;
//	int minutes;
//	int seconds;
//};
typedef struct _CPUsInfo
{
	string infoString = "";
	int numberOfCPUs = 0;
	int totalNumberOfLogicalProcessors = 0;
} CPUsInfo;

typedef struct _version
{
	WORD major=NULL	;
	WORD minor = NULL;
	WORD build = NULL;
	char LastWrittenTime[30] = { ""};
} version;

enum class flowDirection4
{
	//N=1, E = 4, S = 16, W=64, NONE=0
	E4 = 1, S4 = 3, W4 = 5, N4 = 7, NONE4 = 0
};

enum class flowDirection8
{
	//N=1, NE=2, E=4, SE=8, S=16,  SW=32, W=64, NW=128, NONE=0
	E8 = 1, SE8 = 2, S8 = 3, SW8 = 4, W8 = 5, NW8 = 6, N8 = 7, NE8 = 8, NONE8 = 0
};

enum class rainfallDataType
{
	NoneRF, 
	TextFileMAP, 
	TextFileASCgrid, 
	TextFileASCgrid_mmPhr
};

// 1:Discharge, 2:Depth, 3:Height, 4:None
enum class conditionDataType
{
	NoneCD,   //0
	Discharge, // 1
	Depth,      //2
	Height     //3
} ;

enum class fileOrConstant
{
	File,
	Constant,
	None
};

enum class rendererType
{
	Depth,
	Risk
};

void appendTextToTextFile(string fpn, string textToAppend);

bool compareNaturalOrder(const std::string& a, const std::string& b);
int confirmDeleteFile(string filePathNames);
int confirmDeleteFiles(vector<string> filePathNames);

// formatted numeric string
string forString(double value, int precision); 

CPUsInfo getCPUinfo();
version getCurrentFileVersion();
vector<string> getFileListInNaturalOrder(string path, string extension);
vector<string> getFileList(string path, string extension);
string getGPUinfo();
int getVectorIndex(vector<int> inv, int value);
string getValueStringFromXmlLine(string aLine, string fieldName);
//char* getPath(char *fpn);

bool isNumeric(string instr);
bool isNumericDbl(string instr);
bool isNumericInt(string instr);

void makeASCTextFile(string fpn, string allHeader, double** array2D,
	int arrayLength_x, int arrayLength_y,
	int precision, int nodataValue);
void makeBMPFileUsingArrayGTzero_InParallel(string imgFPNtoMake,
	double** array2D,
	int colxNum, int rowyNum, rendererType rt,
    double rendererMaxV = 0, 	double nodataV = -9999);

vector<double> readTextFileToDoubleVector(string fpn);
vector<float> readTextFileToFloatVector(string fpn);
vector<string> readTextFileToStringVector(string fpn);
string readTextFileToString(string fpn);
map <int, vector<string>> readVatFile(string vatFPN, char seperator);
string replaceText(string inText, string textToFind, string textToRepalce);

tm secToHHMMSS(long sec);
tm stringToDateTime(string yyyymmddHHMM);
tm stringToDateTime2(string yyyymmdd_HHcolonMM);
vector<double> splitToDoubleVector(string strToSplit, const char delimeter, bool removeEmptyEntry = true);
vector<float> splitToFloatVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry=true);
vector<double> splitToDoubleVector(string strToSplit, string delimeter, bool removeEmptyEntry = true);
vector<int> splitToIntVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry = true);
vector<string> splitToStringVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry = true);
char* stringToCharP(string c_charP);

string timeElaspedToString_yyyymmddHHMM(string startTime_yyyymmdd_HHcolonMM, int elaspedTimeSec);
char* timeToString_yyyymmdd_HHclnMMclnSS(struct tm* t, int includeSEC = -1);
string timeToString_yyyymmdd_HHclnMMclnSS(struct tm t, int includeSEC = -1);
string timeToString_yyyymmdd_HHclnMMclnSS(COleDateTime t, int includeSEC);
string lower(string instring);
string upper(string instring);

void waitEnterKey();
bool writeLog(const char* fpn, char* printText, int bprintFile, int bprintConsole);
bool writeLog(fs::path fpn, char* printText, int bprintFile, int bprintConsole);
bool writeLog(fs::path fpn, string printText, int bprintFile, int bprintConsole);
bool writeNewLog(const char* fpn, char* printText, int bprintFile, int bprintConsole);
bool writeNewLog(fs::path fpn, char* printText, int bprintFile, int bprintConsole);
bool writeNewLog(fs::path fpn, string printText, int bprintFile, int bprintConsole);
void writeTwoDimData(string fpn, double** array2D, int arrayLength_x, int arrayLength_y,
	int precision, int nodataValue);


class ascRasterFile
{

private:
	//const int BigSizeThreshold = 200000000;//2억개 기준
	char separator = { ' ' };

public:
	bool disposed = false;
	// Instantiate a SafeHandle instance.
//SafeHandle handle = new SafeFileHandle(IntPtr.Zero, true);
	string linesForHeader[8];
	//double dataValueOri;
	ascRasterHeader header;
	string headerStringAll;
	double** valuesFromTL;
	ascRasterExtent extent;
	double value_max = DBL_MIN;
	double value_min = DBL_MAX;
	double value_sum = 0.0;
	int cellCount_notNull = 0;
	double value_ave = 0.0;

	ascRasterFile(string fpn_ascRasterFile);
	ascRasterHeader getAscRasterHeader(string LinesForHeader[], char separator);
	ascRasterExtent getAscRasterExtent(ascRasterHeader header);
	string makeHeaderString(int ncols, int nrows, double xll, double yll, double cellSize, double dx, double dy, int nodataValue);

	~ascRasterFile();
};


inline std::string trim(std::string& str)
{
	str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
	str.erase(str.find_last_not_of(' ') + 1);         //surfixing spaces
	return str;
}

inline std::string trimL(std::string& str)
{
	str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
	return str;
}


inline std::string trimR(std::string& str)
{
	str.erase(str.find_last_not_of(' ') + 1);         //surfixing spaces
	return str;
}

