#pragma once
#include <iostream>
#include <stdio.h>
#include <map>
#include<fstream>
#include <filesystem>
#include<ATLComTime.h>
#include <windows.h>
#include <omp.h>

#include "bitmap_image.hpp"

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
	float dx = 0.0;
	float dy = 0.0;
	float cellsize = 0.0;
	int nodataValue = 0;
	int headerEndingLineIndex = 0;
	int dataStartingLineIndex = 0;
} ascRasterHeader;

typedef struct _cellPosition
{
	int xCol = 0;
	int yRow = 0;
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
	int totalNumOfLP = 0;
} CPUsInfo;

typedef struct _version
{
	WORD pmajor = NULL;
	WORD pminor = NULL;
	WORD pbuild = NULL;
	WORD fmajor=NULL	;
	WORD fminor = NULL;
	WORD fbuild = NULL;
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
	E8 = 1, SE8 = 2, S8 = 3, SW8 = 4, W8 = 5, NW8 = 6, N8 = 7, NE8 = 8, None8 = 0
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

enum class dateTimeFormat
{
	yyyymmdd__HHcolMMcolSS, // 20200324 15:30
	yyyy_mm_dd__HHcolMMcolSS, // 2020-03-24 15:30
	yyyymmddHHMMSS //202003241530
};

enum class timeUnitToShow
{
	toS, // 초까지 표시
	toM, //분 까지 표시
	toH // 시간까지 표시
};

void appendTextToTextFile(string fpn, string textToAppend);

bool compareNaturalOrder(const std::string& a, const std::string& b);
int confirmDeleteFile(string filePathNames);
int confirmDeleteFiles(vector<string> filePathNames);

// formatted numeric string
string dtos(double value, int precision); //빠르다
string dtos_L(double value, int length, int precision); //빠르다
//string itos(double value);

CPUsInfo getCPUinfo();
version getCurrentFileVersion();
string getCurrentExeFilePathName();
vector<string> getFileListInNaturalOrder(string path, string extension);
vector<string> getFileList(string path, string extension);
string getGPUinfo();
int getTableStateByXmlLineByLine(string aLine, string tableName);
int getTableStateByXmlLine(string aLine, string tableName);
int getVectorIndex(vector<int> inv, int value);
string getValueStringFromXmlLine(string aLine, string fieldName);

bool isNumeric(string instr);
bool isNumericDbl(string instr);
bool isNumericInt(string instr);

//void makeASCTextFile(string fpn, string allHeader, double** array2D,
//	int arrayLength_x, int arrayLength_y,
//	int precision, int nodataValue);

//void makeBMPFileUsingArrayGTzero_InParallel(string imgFPNtoMake,
//	double** array2D,
//	int colxNum, int rowyNum, rendererType rt,
//    double rendererMaxV = 0, 	double nodataV = -9999);

vector<double> readTextFileToDoubleVector(string fpn);
vector<float> readTextFileToFloatVector(string fpn);
vector<string> readTextFileToStringVector(string fpn);
string readTextFileToString(string fpn);
map <int, vector<string>> readVatFile(string vatFPN, char seperator);
string replaceText(string inText, string textToFind, string textToRepalce);

tm secToHHMMSS(long sec);
double stod_c(string inst);//Comma를 포함하는 string 숫자를 double로 변환
tm stringToDateTime(string yyyymmddHHMM);
tm stringToDateTime2(string yyyy_mm_dd__HHcolonMM);
vector<double> splitToDoubleVector(string strToSplit, 
	const char delimeter, bool removeEmptyEntry = true);
vector<float> splitToFloatVector(string stringToBeSplitted, 
	char delimeter, bool removeEmptyEntry=true);
vector<double> splitToDoubleVector(string strToSplit,
	string delimeter, bool removeEmptyEntry = true);
vector<int> splitToIntVector(string stringToBeSplitted, 
	char delimeter, bool removeEmptyEntry = true);
vector<string> splitToStringVector(string stringToBeSplitted, 
	char delimeter, bool removeEmptyEntry = true);
string* splitToStringArray(string stringToBeSplitted,
	char delimeter, bool removeEmptyEntry = true);
char* stringToCharP(string inString);
char** stringVectorToCharPP(vector<string> inStrV);

// 이 함수의 기준시간 포맷은 yyyymmddHHMM
string timeElaspedToDateTimeFormat(string startTime_yyyymmddHHMM,
	int elaspedTimeSec, timeUnitToShow unitToShow, 
	dateTimeFormat tformat);
// 이 함수의 기준시간 포맷은 yyyy-mm-dd HH:MM
string timeElaspedToDateTimeFormat2(string startTime_yyyy_mm_dd__HHclnMM,
	int elaspedTimeSec, timeUnitToShow unitToShow, 
	dateTimeFormat tformat);
char* timeToString(struct tm* t, bool includeSEC, 
	dateTimeFormat tformat, bool isLocalTime);
string timeToString(struct tm t, bool includeSEC, dateTimeFormat tformat);
string timeToString(COleDateTime t, bool includeSEC, dateTimeFormat tformat);
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
	string linesForHeader[8];
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

// ========== 여기부터  inline 함수 =========
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

inline string getYYYYMMfromYYYYMMddHHmm
(string INPUTyyyyMMddHHmm)
{
	return INPUTyyyyMMddHHmm.substr(0, 6);
}

inline string getYYYYMMddHHfromYYYYMMddHHmm
(string INPUTyyyyMMddHHmm)
{
	return INPUTyyyyMMddHHmm.substr(0, 10);
}
// ==========  여기까지 inline 함수 =========

// ======= 여기부터 template ==========================
template <typename FLT_DBL> 
void makeASCTextFile(string fpn, string allHeader, 
	FLT_DBL array2D,
	int arrayLength_x, int arrayLength_y,
	int precision, int nodataValue){
	fs::path fpn_out = fs::path(fpn);
	std::ofstream outfile;
	outfile.open(fpn_out, ios::out);
	outfile << allHeader << "\n";
	int isBigSize = 0;
	//int BigSizeThreshold = 200000000; //2억개 기준
	if (arrayLength_x * arrayLength_y > CONST_BIG_SIZE_ARRAY_THRESHOLD) { isBigSize = 1; }
	string formatString = "%." + to_string(precision) + "f ";
	const char* formatStr = formatString.c_str();
	//char* aaa= (char*)malloc(sizeof(char) * 10 * arrayLength_x + 1);
	if (isBigSize == 0) {
		string allLInes = "";
		for (int nr = 0; nr < arrayLength_y; nr++) {
			for (int nc = 0; nc < arrayLength_x; nc++) {
				char vchar[20];
				if (array2D[nc][nr] == nodataValue
					|| array2D[nc][nr] == 0) {
					sprintf(vchar, "%d ", (int)array2D[nc][nr]);
				}
				else {
					sprintf(vchar, formatStr, array2D[nc][nr]);
				}
				allLInes += vchar;
			}
			allLInes += '\n';
		}
		outfile << allLInes;
		outfile.close();
	}
	else {
		string lines = "";
		for (int nr = 0; nr < arrayLength_y; nr++) {
			for (int nc = 0; nc < arrayLength_x; nc++) {
				char s1[20];
				if (array2D[nc][nr] == nodataValue
					|| array2D[nc][nr] == 0) {
					sprintf(s1, "%d ", (int)array2D[nc][nr]);
				}
				else {
					sprintf(s1, formatStr, array2D[nc][nr]);
				}
				lines += s1;
			}
			lines += "\n";
			if (nr % 200 == 0) {
				outfile << lines;
				lines = "";
			}
		}
		outfile << lines;
		outfile.close();
	}
}

template <typename FLT_DBL, typename DBL_INT>
void makeBMPFileUsingArrayGTzero_InParallel(string imgFPNtoMake,
	FLT_DBL** array2D,
	int colxNum, int rowyNum, rendererType rt,
	FLT_DBL rendererMaxV = 0, DBL_INT nodataV = -9999) {
	int iw = colxNum;// *100;
	int ih = rowyNum;// *100;
	int numThread = 0;
	numThread = omp_get_max_threads();
	omp_set_num_threads(numThread);
	bitmap_image img(iw, ih);
	img.clear();
	image_drawer draw(img);
	if (rt == rendererType::Depth) {
#pragma omp parallel for 
		for (int y = 0; y < img.height(); ++y) {
			for (int x = 0; x < img.width(); ++x) {
				double av = array2D[x][y];
				if (av == nodataV) {
					av = 0;
				}
				else {
					if (av > rendererMaxV) {
						av = rendererMaxV;
					}
					if (av < 0) {
						av = 0;
					}
				}
				rgb_t col;
				if (av == 0) {
					col = { 255, 217, 170 };
				}
				else {
					int v = 490 + (int)(av / rendererMaxV * 500.0);// hsv_colormap 에서 490부터 사용. 990까지 사용
					col = hsv_colormap[v];
				}
				img.set_pixel(x, y, col.red, col.green, col.blue);
			}
		}
	}
	else if (rt == rendererType::Risk) {
#pragma omp parallel for 
		for (int y = 0; y < img.height(); ++y) {
			for (int x = 0; x < img.width(); ++x) {
				double av = array2D[x][y];
				rgb_t col;
				if (av == nodataV) {
					col = { 255, 255, 255 };
				}
				else {
					if (av > rendererMaxV) {
						av = rendererMaxV;
					}
					if (av < 0) {
						av = 0;
					}
					int v = 380 + (int)(av / rendererMaxV * 610.0);// hsv_colormap 에서 380부터 사용. 990까지 사용
					col = hsv_colormap[v];
				}
				img.set_pixel(x, y, col.red, col.green, col.blue);
			}
		}
	}
	img.save_image(imgFPNtoMake);
}
// ======= 여기까지 template ==========================
