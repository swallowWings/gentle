#include <iostream>
#include <string>
#include <stdio.h>
#include <intrin.h>
#include <io.h>
#include <stdlib.h>
#include <sstream>
#include <omp.h>
#include<map>
#include <time.h>
#include<ATLComTime.h>
#include <filesystem>
#include <algorithm>

#include "cpuinfodelegate.h"
#include "gpuinfodelegate.h"
#include "bitmap_image.hpp"

#include "gentle.h"


#ifdef _WIN32
	#include <windows.h>
#elif MACOS
	#include <sys/param.h>
	#include <sys/sysctl.h>
#else
	#include <unistd.h>
#endif

using namespace std;
namespace fs = std::filesystem;
extern fs::path fpnLog;
ascRasterFile::ascRasterFile(string fpn_ascRasterFile)
{
	fs::path fpnASC = fpn_ascRasterFile;
	//writeLog(fpnLog,"Read input file : " + fpn_ascRasterFile+'\n',1,1);
	//if (fpn_ascRasterFile == "D:/Github/zTestSet_GRM_SampleGHG_cpp/watershed/GHG_Watershed_c.asc")
	//{
	//	int a = 1;
	//}
	ifstream ascFile(fpnASC);
	if (ascFile.is_open()) {
		string aline;
		int r = 0;
		while (getline(ascFile, aline)) {
			if (r > 7) { break; }//7번 읽는다.. 즉, 7줄을 읽고, 끝낸다. header는 최대 6줄이다.
			linesForHeader[r] = aline;
			r++;
		}
	}
	else {
		string outstr;
		outstr = "ASCII file (" + fpn_ascRasterFile + ") is invalid. It could not be opened.\n";
		cout << outstr;
		return;
	}
	header = getAscRasterHeader(linesForHeader, separator);
	headerStringAll = makeHeaderString(header.nCols, header.nRows,
		header.xllcorner, header.yllcorner, header.cellsize, header.dx, header.dy, header.nodataValue);
	extent = getAscRasterExtent(header);
	valuesFromTL = new double* [header.nCols]; //x를 먼저 할당하고, 아래에서 y를 할당한다.
	for (int i = 0; i < header.nCols; ++i) {
		valuesFromTL[i] = new double[header.nRows];
	}
	bool isBigSize = false;
	if (header.nCols * header.nRows > CONST_BIG_SIZE_ARRAY_THRESHOLD) { isBigSize = true; }
	int headerEndingIndex = header.headerEndingLineIndex;
	int dataStaringIndex = header.dataStartingLineIndex;
	value_sum = 0;
	cellCount_notNull = 0;
	value_max = DBL_MIN;
	value_min = DBL_MAX;	
	if (isBigSize == false) {
		vector<string> allLinesv = readTextFileToStringVector(fpn_ascRasterFile);
		int lyMax = dataStaringIndex + header.nRows - 1; //(int)allLinesv.size();
		// //serial이 parallel 보다 2배 이상 느리다.
		//for (int ly = dataStaringIndex; ly < lyMax; ++ly) {
		//	vector<string> values = splitToStringVector(allLinesv[ly], ' ');
		//	int y = ly - dataStaringIndex;
		//	int nX = header.nCols;// (int)values.size();
		//	for (int x = 0; x < nX; ++x) {
		//		writeLog(fpnLog, "(x, y), value : " + to_string(x) + ", " + to_string(y) + ", " + values[x] + '\n', 1, 1);
		//		if (isNumeric(values[x]) == true) {
		//			valuesFromTL[x][y] = stod(values[x]);
		//		}
		//		else {
		//			valuesFromTL[x][y] = header.nodataValue;
		//		}
		//		if (valuesFromTL[x][y] != header.nodataValue) {
		//			cellCount_notNull++;
		//			value_sum = value_sum + valuesFromTL[x][y];
		//			if (valuesFromTL[x][y] > value_max) {
		//				value_max = valuesFromTL[x][y];
		//			}
		//			if (valuesFromTL[x][y] < value_min) {
		//				value_min = valuesFromTL[x][y];
		//			}
		//		}
		//		writeLog(fpnLog, "(x, y) : " + to_string(x) + ", " + to_string(y) + " completed...\n", 1, 1);
		//	}
		//}

////#pragma omp parallel
////		{
////			double min_local = DBL_MAX;
////			double sum_local = 0;
////			int count_local = 0;
////			double max_local = DBL_MIN;
////#pragma omp parallel for 
////			for (int ly = header.dataStartingLineIndex; ly < lyMax; ++ly) {
////				vector<string> values = splitToStringVector(allLinesv[ly], ' ');
////				int y = ly - dataStaringIndex;
////				int nX = (int)values.size();
////				for (int x = 0; x < nX; ++x) {
////					if (isNumeric(values[x]) == true) {
////						valuesFromTL[x][y] = stod(values[x]);
////					}
////					else {
////						valuesFromTL[x][y] = header.nodataValue;
////					}
////					if (valuesFromTL[x][y] != header.nodataValue) {
////						count_local = count_local + 1;
////						sum_local = sum_local + valuesFromTL[x][y];
////						if (valuesFromTL[x][y] > max_local) {
////							max_local = valuesFromTL[x][y];
////						}
////						if (valuesFromTL[x][y] < min_local) {
////							min_local = valuesFromTL[x][y];
////						}
////					}
////				}
////			}
////#pragma omp critical
////			{
////				if (value_max < max_local) {
////					value_max = max_local;
////				}
////				if (value_min > min_local) {
////					value_min = min_local;
////				}
////				value_sum = value_sum + sum_local;
////				cellCount_notNull = cellCount_notNull + count_local;
////			}
////		}

		// 병렬이 serial 보다 2배 이상 빠르다. // 가장 좋다.
		int numThread = 0;
		numThread = omp_get_max_threads();
		omp_set_num_threads(numThread);
		double* min_local = new double[numThread];
		double* max_local = new double[numThread];
		double* sum_local = new double[numThread];
		int* count_local = new int[numThread];
#pragma omp parallel
		{
			int nth = omp_get_thread_num();
			min_local[nth] = DBL_MAX;
			sum_local[nth] = 0;
			count_local[nth] = 0;
			max_local[nth] = DBL_MIN;
#pragma omp for
			for (int ly = header.dataStartingLineIndex; ly < lyMax; ++ly) {
				vector<string> values = splitToStringVector(allLinesv[ly], ' ');
				int y = ly - dataStaringIndex;
				int nX = header.nCols;// (int)values.size();
				for (int x = 0; x < nX; ++x) {
					if (isNumeric(values[x]) == true) {
						valuesFromTL[x][y] = stod(values[x]);
					}
					else {
						valuesFromTL[x][y] = header.nodataValue;
					}
					if (valuesFromTL[x][y] != header.nodataValue) {
						//cellCount_notNull++;
						count_local[nth] = count_local[nth] + 1;
						sum_local[nth] = sum_local[nth] + valuesFromTL[x][y];
						if (valuesFromTL[x][y] > max_local[nth]) {
							max_local[nth] = valuesFromTL[x][y];
						}
						if (valuesFromTL[x][y] < min_local[nth]) {
							min_local[nth] = valuesFromTL[x][y];
						}
					}
				}
			}
		}
		for (int i = 0; i < numThread; ++i) {
			if (value_max < max_local[i]) {
				value_max = max_local[i];
			}
			if (value_min > min_local[i]) {
				value_min = min_local[i];
			}
			value_sum = value_sum + sum_local[i];
			cellCount_notNull = cellCount_notNull + count_local[i];
		}
		delete[] min_local;
		delete[] max_local;
		delete[] sum_local;
		delete[] count_local;
	}
	else {
		int nl = 0;
		int y = 0;
		string aline;
		while (getline(ascFile, aline)) {
			linesForHeader[nl] = aline;
			if (nl > headerEndingIndex) {
				vector<string> values = values = splitToStringVector(aline, ' ');
				for (int x = 0; x < values.size(); ++x) {
					double v = 0;
					if (isNumeric(values[x]) == true) {
						valuesFromTL[x][y] = stod(values[x]);
					}
					else {
						valuesFromTL[x][y] = header.nodataValue;
					}
					if (valuesFromTL[x][y] != header.nodataValue)
					{
						cellCount_notNull++;
						value_sum = value_sum + valuesFromTL[x][y];
						if (valuesFromTL[x][y] > value_max) {
							value_max = valuesFromTL[x][y];
						}
						if (valuesFromTL[x][y] < value_min) {
							value_min = valuesFromTL[x][y];
						}
					}
				}
				y++;
			}
			nl++;
		}
	}

	if (cellCount_notNull > 0) {
		value_ave = value_sum / cellCount_notNull;
	}
}

ascRasterFile::~ascRasterFile()
{
	for (int i = 0; i < header.nCols; ++i) {
		if (valuesFromTL[i] != NULL) {
			delete[] valuesFromTL[i];
		}
	}
	delete[] valuesFromTL;
}

ascRasterHeader ascRasterFile::getAscRasterHeader(string inputLInes[], char separator)
{
	ascRasterHeader header;
	header.dataStartingLineIndex = -1;
	for (int ln = 0; ln < 7; ++ln)
	{
		string aline = inputLInes[ln];
		vector<string> LineParts = splitToStringVector(aline, separator);
		int iv = 0;
		double dv = 0;
		switch (ln)
		{
		case 0:
			header.nCols = stoi(LineParts[1]);
			break;
		case 1:
			header.nRows = stoi(LineParts[1]);
			break;
		case 2:
			header.xllcorner = stod(LineParts[1]);
			break;
		case 3:
			header.yllcorner = stod(LineParts[1]);
			break;
		case 4:
			if (lower(LineParts[0]) == "dx") {
				if (isNumeric(LineParts[1]) == true) {
					header.dx = stof(LineParts[1]);
				}
				else {
					header.dx = -1;
				}
			}
			else if (lower(LineParts[0]) == "cellsize") {
				if (isNumeric(LineParts[1]) == true) {
					header.cellsize = stof(LineParts[1]);
				}
				else {
					header.cellsize = -1;
				}
			}
			else {
				header.cellsize = -1;
			}
			break;
		case 5:
			if (lower(LineParts[0]) == "nodata_value") {
				if (isNumeric(LineParts[1]) == false) {
					header.nodataValue = -9999;
				}
				else {
					header.nodataValue = stoi(LineParts[1]);
				}
			}
			else if (lower(LineParts[0]) == "dy") {
				if (isNumeric(LineParts[1]) == true) {
					header.dy = stof(LineParts[1]);
				}
				else {
					header.dy = -1;
				}
			}
			else {
				header.nodataValue = -9999;
			}
			break;
		case 6:
			if (lower(LineParts[0]) == "nodata_value") {
				if (isNumeric(LineParts[1]) == false) {
					header.nodataValue = -9999;
				}
				else {
					header.nodataValue = stoi(LineParts[1]);
				}
			}
			break;
		}
		if (ln > 4) {
			if (LineParts.size() > 0) {
				if (isNumeric(LineParts[0]) == true) {
					header.dataStartingLineIndex = ln;
					header.headerEndingLineIndex = ln - 1;
					return header;
				}
			}
		}
	}
	return header;
}

ascRasterExtent ascRasterFile::getAscRasterExtent(ascRasterHeader header)
{
	ascRasterExtent ext;
	ext.bottom = header.yllcorner;
	ext.top = header.yllcorner +  header.nRows * header.cellsize;
	ext.left = header.xllcorner;
	ext.right = header.xllcorner +  header.nCols * header.cellsize;
	ext.extentWidth = ext.right - ext.left;
	ext.extentHeight = ext.top - ext.bottom;
	return ext;
}

string ascRasterFile::makeHeaderString(int ncols, int nrows, double xll, double yll, double cellSize, double dx, double dy, int nodataValue)
{
	string headerall = "";
	headerall =  "ncols " + to_string(ncols) + "\n";
	headerall = headerall + "nrows " + to_string(nrows) + "\n";
	headerall = headerall + "xllcorner " + to_string(xll) + "\n";
	headerall = headerall + "yllcorner " + to_string(yll) + "\n";

	if (dx != dy && dx > 0 && dy > 0)
	{
		headerall = headerall + "dx" + " " + to_string(dx) + "\n";
		headerall = headerall + "dy" + " " + to_string(dy) + "\n";
	}
	else
	{
		headerall = headerall + "cellsize " + to_string(cellSize) + "\n";
	}
	
	headerall = headerall + "NODATA_value " + to_string(nodataValue);
	return headerall;
}

void appendTextToTextFile(string fpn, string textToAppend)
{
	std::ofstream outfile;
	outfile.open(fpn, ios::app);
	outfile << textToAppend;
	outfile.close();
	//FILE* fp = fopen(fpn.c_str(), "a");
	//fprintf(fp, textToAppend.c_str());
	//fclose(fp);
}

bool compareNaturalOrder(const std::string& a, const std::string& b) 
{
	if (a.empty())
		return true;
	if (b.empty())
		return false;
	if (std::isdigit(a[0]) && !std::isdigit(b[0]))
		return true;
	if (!std::isdigit(a[0]) && std::isdigit(b[0]))
		return false;
	if (!std::isdigit(a[0]) && !std::isdigit(b[0]))	{
		if (a[0] == b[0]) {
			return compareNaturalOrder(a.substr(1), b.substr(1));
		}
		return (upper(a) < upper(b));
	}

	std::istringstream issa(a);
	std::istringstream issb(b);
	int ia, ib;
	issa >> ia;
	issb >> ib;
	if (ia != ib)	{
		return ia < ib;
	}
		
	std::string anew, bnew;
	std::getline(issa, anew);
	std::getline(issb, bnew);
	return (compareNaturalOrder(anew, bnew));
}

int confirmDeleteFiles(vector<string> filePathNames)
{
	bool bAlldeleted = false;
	int n = 0;
	while (!(bAlldeleted == true)) {
		n += 1;
		for (string fpn : filePathNames) {
			if (fs::exists(fpn) == true) {
				std::remove(fpn.c_str());
			}
		}
		for (string fpn : filePathNames) {
			if (fs::exists(fpn) == false) {
				bAlldeleted = true;
				break;
			}
			else {
				bAlldeleted = false;
				break;
			}
		}
		if (n > 100) { return 0; }
	}
	return 1;
}

int confirmDeleteFile(string filePathNames)
{
	bool bAlldeleted = false;
	int n = 0;
	while (!(bAlldeleted == true))
	{
		n += 1;
		if (fs::exists(filePathNames) == true) {
			std::remove(filePathNames.c_str());
		}
		if (fs::exists(filePathNames) == false) {
			bAlldeleted = true;
			break;
		}
		else {
			bAlldeleted = false;
			break;
		}
		if (n > 100) { return -1; }
	}
	return 1;
}

string dtos(double value, int precision)
{
	char vchar[20];
	char fchar[8];
	sprintf(fchar, "%%.%df", precision);
	sprintf(vchar, fchar, value);
	return vchar;
}

string dtos2(double value, int precision)
{
	stringstream stream;
	stream << std::fixed << std::setprecision(precision) << value;
	string s = stream.str();
	return s;
}

//string itos(double value)
//{
//	char vchar[20];
//	sprintf(vchar, "%d", value);
//	return vchar;
//}


CPUsInfo getCPUinfo()
{
	CPUInfoDelegate *cpuInfo = new CPUInfoDelegate();
	std::vector<CPUInfo> cpuInfoVector = cpuInfo->cpuInfoVector();

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int CPUCount = 1;
	int totalLP = 0;
	string infoStr;
	infoStr ="  " + std::to_string(cpuInfo->numberOfCPUInfoItems()) + " CPU(s) installed.\n";
	for (std::vector<CPUInfo>::iterator iter = cpuInfoVector.begin(); iter != cpuInfoVector.end(); ++iter) 
	{
		//std::cout << "CPU Manufacturer = " << iter->manufacturer() << std::endl;
		//std::cout << "Current CPU Clock Speed = " << iter->currentClockSpeed() << std::endl;
		//std::cout << "CPU Architecture = " << iter->architecture() << std::endl;
		//std::cout << "CPU L2 Cache Size = " << iter->L2CacheSize() << std::endl;
		//std::cout << "CPU L3 Cache Size = " << iter->L3CacheSize() << std::endl;
		//std::cout << "Current CPU Temperature = " << iter->currentTemperature() << std::endl;

		infoStr += "  CPU #" + to_string(CPUCount) + ".\n";
		infoStr += "    CPU name : " + iter->name() + '\n';
		infoStr += "    Number of CPU cores : " + iter->numberOfCores() + '\n';
		infoStr += "    Number of logical processors : " + std::to_string(sysInfo.dwNumberOfProcessors) + '\n';
		CPUCount++;
		totalLP = totalLP + stoi(to_string(sysInfo.dwNumberOfProcessors));
	}
	CPUsInfo cpusi;
	cpusi.infoString = infoStr;
	cpusi.numberOfCPUs = cpuInfo->numberOfCPUInfoItems();
	cpusi.totalNumOfLP = totalLP;
	delete cpuInfo;
	return cpusi;
}


string getCurrentExeFilePathName()
{
	TCHAR fpn_exe[MAX_PATH];
	GetModuleFileName(NULL, fpn_exe, sizeof(fpn_exe));
	return fpn_exe;
}

version getCurrentFileVersion()
{
	TCHAR fpn_exe[MAX_PATH];
	DWORD size = GetModuleFileName(NULL, fpn_exe, sizeof(fpn_exe));
	DWORD infoSize = 0;
	version ver;

	// 파일로부터 버전정보데이터의 크기
	infoSize = GetFileVersionInfoSize(fpn_exe, 0);
	if (infoSize == 0) return ver;

	// 버퍼할당
	char* buffer = NULL;
	buffer = new char[infoSize];

	if (buffer) {
		// 버전정보데이터
		if (GetFileVersionInfo(fpn_exe, 0, infoSize, buffer) != 0) {
			VS_FIXEDFILEINFO* pFineInfo = NULL;
			UINT bufLen = 0;
			// buffer로 부터 VS_FIXEDFILEINFO 정보
			//VerQueryValueA(buffer, "\\", (LPVOID*)&pFineInfo, &bufLen);
			if (VerQueryValue(buffer, "\\", (LPVOID*)&pFineInfo, &bufLen) != 0) {
				ver.major = HIWORD(pFineInfo->dwFileVersionMS);
				ver.minor = LOWORD(pFineInfo->dwFileVersionMS);
				ver.build = HIWORD(pFineInfo->dwFileVersionLS);
				//ver.LastWrittenTime = new char[30];
				struct _stat buf;
				if (_stat(fpn_exe, &buf) != 0) {
					switch (errno) {
					case ENOENT:
						fprintf(stderr, "File %s not found.\n", fpn_exe);
					case EINVAL:
						fprintf(stderr, "Invalid parameter to _stat.\n");
					default:
						fprintf(stderr, "Unexpected error in _stat.\n");
					}
					sprintf_s(ver.LastWrittenTime, "");
				}
				else {
					//printf("%s\n", fpn_exe);
					//printf("\tTime Creation     : %s\n", timeToString(localtime(&buf.st_ctime)));
					//printf("\tTime Last Written : %s\n", timeToString(localtime(&buf.st_mtime)));
					//printf("\tTime Last Access  : %s\n", timeToString(localtime(&buf.st_atime)));
					//tm ltm;
					//localtime_s(&ltm, &buf.st_mtime);
					COleDateTime tnow = COleDateTime::GetCurrentTime();
					sprintf_s(ver.LastWrittenTime, timeToString(tnow,
						false, dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS).c_str());
				}
			}
		}
		delete[] buffer;
	}
	return ver;
}

string getGPUinfo()
{
	GPUInfoDelegate *gpuInfo = new GPUInfoDelegate();
	std::vector<GPUInfo> gpuInfoVector = gpuInfo->gpuInfoVector();
	//std::cout << "This computer has " << gpuInfo->numberOfGPUInfoItems() << " GPU(s) installed" << std::endl;
	//int gpuCount = 1;
	//for (std::vector<GPUInfo>::const_iterator iter = gpuInfoVector.begin(); iter != gpuInfoVector.end(); iter++) {
	//	std::cout << "Information for GPU #" << gpuCount << ": " << std::endl;
	//	std::cout << "GPU Name = " << iter->name() << std::endl;
	//	std::cout << "GPU Manufacturer = " << iter->manufacturer() << std::endl;
	//	std::cout << "GPU Adapter RAM = " << iter->adapterRAM() << std::endl;
	//	std::cout << "GPU Refresh Rate = " << iter->refreshRate() << std::endl;
	//	std::cout << "GPU Driver Version = " << iter->driverVersion() << std::endl;
	//	std::cout << "GPU Video Architecture = " << iter->videoArchitecture() << std::endl;
	//	std::cout << "GPU Video Mode Description = " << iter->videoModeDescription() << std::endl;
	//	std::cout << std::endl;
	//	gpuCount++;
	//}

	//SYSTEM_INFO sysInfo;
	//GetSystemInfo(&sysInfo);
	//int CPUCount = 1;
	string infoStr;
	infoStr = "  " + std::to_string(gpuInfo->numberOfGPUInfoItems()) + " GPU(s) installed.\n";
	int gpuCount = 1;
	for (std::vector<GPUInfo>::const_iterator iter = gpuInfoVector.begin(); iter != gpuInfoVector.end(); ++iter)
	{
		infoStr += "  GPU #" + to_string(gpuCount) + ".\n";
		infoStr += "    GPU name : " + iter->name() + '\n';
		infoStr += "    GPU adapter ram : " + iter->adapterRAM() + '\n';
		infoStr += "    GPU driver version : " + iter->driverVersion() + '\n';
		gpuCount++;
	}
	delete gpuInfo;
	return infoStr;
}

vector<string> getFileList(string path, string extension)
{
	vector<string> flist;
	for (const auto& entry : fs::directory_iterator(path)) {
		fs::path filePath = entry.path();
		if (lower(filePath.extension().string())== extension) {
			flist.push_back(filePath.string());
		}
	}
	return flist;
}

vector<string> getFileListInNaturalOrder(string path, string extension)
{
	vector<string> flist = getFileList(path, extension);
	std::sort(flist.begin(), flist.end(), compareNaturalOrder);
	return flist;
}

int getTableStateByXmlLineByLine(string aLine, string tableName)
{
	string sActive = "<" + trim(tableName) + ">";
	string sinActive = "</" + trim(tableName) + ">";

	if (sActive == trim(aLine)) {
		return 1;// activated state
	} 
	if (sinActive == trim(aLine)) {
		return 0;// inactivated state. closed
	}
	return -1; 
}

int getTableStateByXmlLine(string aLine, string tableName)
{
	string sActive = "<" + trim(tableName) + ">";
	string sinActive = "</" + trim(tableName) + ">";
	int posS = (int)aLine.find(sActive, 0);
	int posE = (int)aLine.find(sinActive, 0);
	int state = -1; // this line dose not have table name. it means this table is active.
	if (posS >= 0) {
		state = 1; // activated state
	}
	if (posE >= 0) {
		state = 0; // inactivated state
	}
	if (posS >= 0 && posE >= 0) {
		state = 2; // In this line, table is opened, field info. is, and it is closed.
	}
	return state;
}


string getValueStringFromXmlLine(string aLine, string fieldName)
{
	int len_fiedlName = 0;
	int pos1 = 0;
	string strToFind = "<" + fieldName + ">";
	pos1 = (int)aLine.find(strToFind, 0);
	if (pos1 >= 0) {
		len_fiedlName = (int)strToFind.length();
		int pos2 = 0;
		string strToFind2 = "</" + fieldName + ">";
		pos2 = (int)aLine.find(strToFind2);
		if (pos2 >= 0) {
			string valueString = "";
			int valueSize = 0;
			valueSize = pos2 - pos1 - len_fiedlName;
			valueString = aLine.substr(pos1 + len_fiedlName, valueSize);
			return valueString;
		}
		return "";
	}
	else {
		return "";
	}
}

int getVectorIndex(vector<int> inv, int value)
{
	std::vector<int>::iterator it;
	it = std::find(inv.begin(), inv.end(), value);
	if (it == inv.end()) {
		return -1;
	}
	else {
		int idx = std::distance(inv.begin(), it);
		return idx;
	}
}

bool isNumericDbl(string instr)
{
	return atof(instr.c_str()) != 0 || instr.compare("0") == 0;
}

bool isNumericInt(string instr)
{
	return atoi(instr.c_str()) != 0 || instr.compare("0") == 0;
}

bool isNumeric(string instr)
{
	char* error;
	double num = strtod(instr.c_str(), &error);
	if (*error) {
		return false;
	}
	else {
		return true;
	}
}

// 빠르다. 2020.05.06. 최
void makeASCTextFile(string fpn, string allHeader, double** array2D,
	int arrayLength_x, int arrayLength_y,
	int precision, int nodataValue)
{
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
			allLInes +='\n';
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
					sprintf(s1, "%d ", (int) array2D[nc][nr]);
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

void makeBMPFileUsingArrayGTzero_InParallel(string imgFPNtoMake,
	double** array2D,
	int colxNum, int rowyNum, rendererType rt,
	double rendererMaxV, double nodataV)
{
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
					int v = 490 + (int)(av / rendererMaxV * 510.0);// hsv_colormap 에서 490부터 사용한다.
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
					int v = 380 + (int)(av / rendererMaxV * 610.0);
					col = hsv_colormap[v];
				}
				img.set_pixel(x, y, col.red, col.green, col.blue);
			}
		}
	}
	img.save_image(imgFPNtoMake);
}



// key별 속성을 vector<string>으로 저장해서 반환
map <int, vector<string>> readVatFile(string vatFPN, char seperator)
{
	map <int, vector<string>> values;
	ifstream vatFile(vatFPN);
	if (vatFile.is_open()) {
		string aline;
		int r = 0;
		while (getline(vatFile, aline)) {
			vector<string> parts = splitToStringVector(aline, seperator);
			int attValue = 0;
			if (parts.size() > 1) {
				attValue = stoi(parts[0]);
				if (values.count(attValue) == 0) {
					parts.erase(parts.begin());
					values.insert(make_pair(attValue, parts));
				}
			}
			else {
				string outstr;
				outstr = "Values in VAT file (" + vatFPN + ") are invalid, or have no attributes.\n";
				cout << outstr;
				cout << "Each grid value must have one more attribute. \n";
			}
			r++;
		}
	}
	return values;
}

//bool removeFolder(string filePath)
//{
//	//if (access(filePath.c_str(), 0) == 0) {
//	//	// 파일이 있다.
//		//fs::remove_all(filePath);
//		//fs::remove(filePath);
//	//}
//	//else {
//	//	//파일이 없다.
//		//fs::remove(filePath);
//	//}
//
//	CString fp(filePath.c_str());
//	if (fp.IsEmpty()) {
//		RemoveDirectory(fp);
//	}
//	else {
//		CFileFind   ff;
//		BOOL   bFound = ff.FindFile(fp + _T("\\*"), 0);
//		while (bFound) {
//			bFound = ff.FindNextFile();
//			if (ff.GetFileName() == _T(".") || ff.GetFileName() == _T(".."))
//				continue;
//			SetFileAttributes(ff.GetFilePath(), FILE_ATTRIBUTE_NORMAL);
//			if (ff.IsDirectory()==true) {
//				DeleteDirectory(ff.GetFilePath());
//				RemoveDirectory(ff.GetFilePath());
//			}
//			else {
//				DeleteFile(ff.GetFilePath());
//			}
//		}
//		ff.Close();
//		RemoveDirectory(fp);
//	}
//	return true;
//}

string replaceText(string inText, string textToFind, string textToRepalce)
{
	string s;
	int idxStart = inText.find(textToFind, 0);
	if (idxStart > 0) {
		int length = textToFind.length();
		s = inText.replace(idxStart, length, textToRepalce);
	}
	else {
		s = inText;
	}
	return s;
}

vector<double> readTextFileToDoubleVector(string fpn)
{
	ifstream txtFile(fpn);
	string aline;
	vector<double> linesv;
	while (!txtFile.eof()) {
		getline(txtFile, aline);
		if (aline.size() > 0) {
			if (isNumeric(aline) == true) {
				linesv.push_back(stod(aline));
			}
			else {
				cout << fpn << " contains non-numeric value."<< endl;
				return linesv;
			}
		}
	}
	return linesv;
}

vector<float> readTextFileToFloatVector(string fpn)
{
	ifstream txtFile(fpn);
	string aline;
	vector<float> linesv;
	while (!txtFile.eof()) {
		getline(txtFile, aline);
		if (aline.size() > 0) {
			if (isNumeric(aline) == true) {
				linesv.push_back(stof(aline));
			}
			else {
				cout << fpn << " contains non-numeric value." << endl;
				return linesv;
			}
		}
	}
	return linesv;
}

string readTextFileToString(string fpn)
{
	std::ifstream ifs(fpn);
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	return content;
}

vector<string> readTextFileToStringVector(string fpn)
{
	ifstream txtFile(fpn);
	string aline;
	vector<string> linesv;
	while (!txtFile.eof())	{
		getline(txtFile, aline);
		if (aline.size() > 0)		{
			linesv.push_back(aline);
		}		
	}
	return linesv;
}


tm secToHHMMSS(long sec)
{
	tm t;
	t.tm_hour = sec / 3600;
	long remains;
	remains = sec % 3600;
	t.tm_min = remains / 60;
	remains = remains % 60;
	t.tm_sec = remains;
	return t;
}


/*
std::string split implementation by using delimeter as a character.
*/
vector<double> splitToDoubleVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry)
{
	stringstream ss(stringToBeSplitted);
	double v;
	string item;
	//char * seprator = delimeter.c_str();
	vector<double> splittedValues;
	while (getline(ss, item, delimeter)) {
		string sv = trim(item);
		if (removeEmptyEntry == true) {
			if (sv != "") {
				v = stod(sv);
				splittedValues.push_back(v);
			}
		}
		else {
			v = stod(sv);
			splittedValues.push_back(v);
		}
	}
	return splittedValues;
}

vector<float> splitToFloatVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry)
{
	stringstream ss(stringToBeSplitted);
	float v;
	string item;
	//char * seprator = delimeter.c_str();
	vector<float> splittedValues;
	while (getline(ss, item, delimeter)) {
		string sv = trim(item);
		if (removeEmptyEntry == true) {
			if (sv != "") {
				v = stof(sv);
				splittedValues.push_back(v);
			}
		}
		else {
			v = stof(sv);
			splittedValues.push_back(v);
		}
	}
	return splittedValues;
}

/*
std::string split implementation by using delimeter as an another string
*/
vector<double> splitToDoubleVector(string stringToBeSplitted, string delimeter, bool removeEmptyEntry)
{
	vector<double> splittedValues;
	int startIndex = 0;
	int  endIndex = 0;
	while ((endIndex = (int)stringToBeSplitted.find(delimeter, startIndex)) < (int)stringToBeSplitted.size()) {
		string item = stringToBeSplitted.substr(startIndex, endIndex - startIndex);
		string sv = trim(item);
		double val;
		if (removeEmptyEntry == true) {
			if (sv != "") {
				val = stod(sv);
				splittedValues.push_back(val);
			}
		}
		else {
			val = stod(sv);
			splittedValues.push_back(val);
		}
		startIndex = endIndex + (int)delimeter.size();
	}
	if (startIndex < stringToBeSplitted.size()) {
		string item = stringToBeSplitted.substr(startIndex);
		string sv = trim(item);
		double val;
		if (removeEmptyEntry == true) {
			if (sv != "") {
				val = stod(sv);
				splittedValues.push_back(val);
			}
		}
		else {
			val = stod(sv);
			splittedValues.push_back(val);
		}
	}
	return splittedValues;
}


vector<int> splitToIntVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry)
{
	stringstream ss(stringToBeSplitted);
	int v;
	string item;
	//char * seprator = delimeter.c_str();
	vector<int> splittedValues;
	while (getline(ss, item, delimeter)) {
		string sv = trim(item);
		if (removeEmptyEntry == true) {
			if (sv != "") {
				v = stoi(sv);
				splittedValues.push_back(v);
			}
		}
		else {
			v = stoi(sv);
			splittedValues.push_back(v);
		}
	}
	return splittedValues;
}

vector<string> splitToStringVector(string stringToBeSplitted, char delimeter, bool removeEmptyEntry)
{
	stringstream ss(stringToBeSplitted);
	string v;
	string item;
	//char * seprator = delimeter.c_str();
	vector<string> splittedValues;
	while (getline(ss, item, delimeter)) {
		v = trim(item);
		if (removeEmptyEntry == true) {
			if (v != "") {
				splittedValues.push_back(v);
			}
		}
		else {
			splittedValues.push_back(v);
		}
	}
	return splittedValues;
}

string * splitToStringArray(string stringToBeSplitted, char delimeter, bool removeEmptyEntry)
{
	stringstream ss(stringToBeSplitted);
	string v;
	string item;
	//char * seprator = delimeter.c_str();
	vector<string> splittedValues;
	while (getline(ss, item, delimeter)) {
		v = trim(item);
		if (removeEmptyEntry == true) {
			if (v != "") {
				splittedValues.push_back(v);
			}
		}
		else {
			splittedValues.push_back(v);
		}
	}
	string* values = new string[splittedValues.size()];
	copy(splittedValues.begin(), splittedValues.end(), values);
	return values;
}

char* stringToCharP(string inString)
{
	std::vector<char> cpv(inString.begin(), inString.end());
	cpv.push_back('\0');
	return &cpv[0];
}


char** stringVectorToCharPP(vector<string> inStrV)
{
	vector <char*> cpv(inStrV.size());
	int vs = inStrV.size();
	char** r = new char* [vs];
	for (int i = 0; i < vs; ++i) { //unsigned i 
		int len = strlen(inStrV[i].c_str())+1;
		r[i] = (char*)malloc(sizeof(char) * len);
		strcpy(r[i], inStrV[i].c_str());
	}
	return r;
}

tm stringToDateTime(string yyyymmddHHMM) // 201711282310
{
	tm t;
	t.tm_year = stoi(yyyymmddHHMM.substr(0, 4));
	t.tm_mon = stoi(yyyymmddHHMM.substr(4, 2));
	t.tm_mday = stoi(yyyymmddHHMM.substr(6, 2));
	t.tm_hour = stoi(yyyymmddHHMM.substr(8, 2));
	t.tm_min = stoi(yyyymmddHHMM.substr(10, 2));
	return t;
}

tm stringToDateTime2(string yyyy_mm_dd__HHcolonMM) // 2017-11-28 23:10, 0123-56-89 12:45
{
	tm t;
	t.tm_year = stoi(yyyy_mm_dd__HHcolonMM.substr(0, 4));
	t.tm_mon = stoi(yyyy_mm_dd__HHcolonMM.substr(5, 2));
	t.tm_mday = stoi(yyyy_mm_dd__HHcolonMM.substr(8, 2));
	t.tm_hour = stoi(yyyy_mm_dd__HHcolonMM.substr(11, 2));
	t.tm_min = stoi(yyyy_mm_dd__HHcolonMM.substr(14, 2));
	return t;
}

// yyyymmddHHMMSS
string timeElaspedToDateTimeFormat(string startTime_yyyymmddHHMM,
	int elaspedTimeSec, timeUnitToShow unitToShow, dateTimeFormat tformat)
{
	tm tms = stringToDateTime(startTime_yyyymmddHHMM);
	string startTime=timeToString(tms, false, 
		dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
	string t_elasped;
	t_elasped = timeElaspedToDateTimeFormat2(startTime,
		elaspedTimeSec, unitToShow, tformat);
	return t_elasped;
}

string timeElaspedToDateTimeFormat2(string startTime_yyyy_mm_dd__HHclnMM,
	int elaspedTimeSec, timeUnitToShow unitToShow, dateTimeFormat tformat)
{
	string startTime = startTime_yyyy_mm_dd__HHclnMM;
	tm tms = stringToDateTime2(startTime);
	COleDateTime pt(tms.tm_year, tms.tm_mon, tms.tm_mday, tms.tm_hour, tms.tm_min, 0);
	COleDateTimeSpan SpendTime;
	int et = abs(elaspedTimeSec);
	SpendTime.SetDateTimeSpan(0, 0, 0, et);
	if (elaspedTimeSec > 0) {
		pt = pt + SpendTime;
	}
	else {
		pt= pt - SpendTime;
	}	
	string time_elasped;
	switch (unitToShow) {
	case timeUnitToShow::toS: {
		switch (tformat) {
		case dateTimeFormat::yyyymmddHHMMSS: {
			time_elasped = CT2CA(pt.Format(_T("%Y%m%d%H%M%S")));
			break;
		}
		case dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS: {
			time_elasped = CT2CA(pt.Format(_T("%Y-%m-%d %H:%M:%S")));
			break;
		}
		case dateTimeFormat::yyyymmdd__HHcolMMcolSS: {
			time_elasped = CT2CA(pt.Format(_T("%Y%m%d %H:%M:%S")));
			break;
		}
		}
		break;
	}
	case timeUnitToShow::toM: {
		switch (tformat) {
		case dateTimeFormat::yyyymmddHHMMSS: {
			time_elasped = CT2CA(pt.Format(_T("%Y%m%d%H%M")));
			break;
		}
		case dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS: {
			time_elasped = CT2CA(pt.Format(_T("%Y-%m-%d %H:%M")));
			break;
		}
		case dateTimeFormat::yyyymmdd__HHcolMMcolSS: {
			time_elasped = CT2CA(pt.Format(_T("%Y%m%d %H:%M")));
			break;
		}
		}
		break;
	}
	case timeUnitToShow::toH: {
		switch (tformat) {
		case dateTimeFormat::yyyymmddHHMMSS: {
			time_elasped = CT2CA(pt.Format(_T("%Y%m%d%H")));
			break;
		}
		case dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS: {
			time_elasped = CT2CA(pt.Format(_T("%Y-%m-%d %H")));
			break;
		}
		case dateTimeFormat::yyyymmdd__HHcolMMcolSS: {
			time_elasped = CT2CA(pt.Format(_T("%Y%m%d %H")));
			break;
		}
		}
	}
	}
	return time_elasped;
}

// yyyymmdd HH:MM:SS
char* timeToString(struct tm* t, bool includeSEC, dateTimeFormat tformat)
{
	static char s[20];
	if (includeSEC ==false) {
		switch (tformat) {
		case dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS: {
			sprintf_s(s, "%04d-%02d-%02d %02d:%02d",
				t->tm_year, t->tm_mon + 1, t->tm_mday,
				t->tm_hour, t->tm_min);
			// t로 local time이 들어오면 t.tm_year+1900 해야 한다.
			break;
		}
		}
	}
	else {
		switch (tformat) {
		case dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS: {
			sprintf_s(s, "%04d-%02d-%02d %02d:%02d:%02d",
				t->tm_year, t->tm_mon + 1, t->tm_mday,
				t->tm_hour, t->tm_min, t->tm_sec);
			break;
		}
		}
	}
	return s;
}

string timeToString(struct tm t, bool includeSEC, dateTimeFormat tformat)
{
	static char s[20];
	if (includeSEC ==false) {
		switch (tformat) {
		case dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS: {
			sprintf_s(s, "%04d-%02d-%02d %02d:%02d",
				t.tm_year, t.tm_mon, t.tm_mday,
				t.tm_hour, t.tm_min);
			// t로 local time이 들어오면 t.tm_year+1900 해야 한다.
			break;
		}
		case dateTimeFormat::yyyymmddHHMMSS: {
			sprintf_s(s, "%04d%02d%02d%02d%02d",
				t.tm_year, t.tm_mon, t.tm_mday,
				t.tm_hour, t.tm_min);
			break;
		}
		}
	}
	else {
		switch (tformat) {
		case dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS: {
			sprintf_s(s, "%04d-%02d-%02d %02d:%02d:%02d",
				t.tm_year, t.tm_mon, t.tm_mday,
				t.tm_hour, t.tm_min, t.tm_sec);
			break;
		}
		case dateTimeFormat::yyyymmddHHMMSS: {
			sprintf_s(s, "%04d%02d%02d%02d%02d%02d",
				t.tm_year, t.tm_mon, t.tm_mday,
				t.tm_hour, t.tm_min, t.tm_sec);
			break;
		}
		}
	}
	return s;
}


string timeToString(COleDateTime t, bool includeSEC, dateTimeFormat tformat)
{
	string s;
	if (includeSEC ==false) {
		switch (tformat) {
		case dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS: {
			s = CT2CA(t.Format(_T("%Y-%m-%d %H:%M")));
			break;
		}
		case dateTimeFormat::yyyymmddHHMMSS: {
			s = CT2CA(t.Format(_T("%Y%m%d%H%M")));
			break;
		}
		}
	}
	else {
		switch (tformat) {
		case dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS: {
			s = CT2CA(t.Format(_T("%Y-%m-%d %H:%M:%S")));
			break;
		}
		case dateTimeFormat::yyyymmddHHMMSS: {
			s = CT2CA(t.Format(_T("%Y%m%d%H:%M:%S")));
			break;
		}
		}
	}
	return s;
}

string lower(string instring)
{
	std::transform(instring.begin(), instring.end(), instring.begin(), tolower);
	return instring;
}

string upper(string instring)
{
	std::transform(instring.begin(), instring.end(), instring.begin(), toupper);
	return instring;
}

void waitEnterKey()
{
	std::cout << "Press [Enter] to continue . . .";
	std::cin.get();
}

bool writeNewLog(const char* fpn, char* printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0) {
		printf(printText);
	}
	if (bprintFile > 0) {
		//time_t now = time(0);
		////tm *ltm = localtime(&now);
		//tm ltm;
		//localtime_s(&ltm, &now);
		COleDateTime tnow = COleDateTime::GetCurrentTime();
		string nows = timeToString(tnow, 
			false, dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
		std::ofstream outfile;
		outfile.open(fpn, ios::out);
		outfile << nows + " " + printText;
		outfile.close();
		//FILE* outFile;
		//outFile = fopen(fpn, "w");
		//fprintf(outFile, printText);
		//fclose(outFile);
	}
	return true;
}

bool writeNewLog(fs::path fpn, char* printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0)	{
		printf(printText);
	}
	if (bprintFile > 0)	{
		//time_t now = time(0);
		//tm ltm;
		//localtime_s(&ltm, &now);
		COleDateTime tnow = COleDateTime::GetCurrentTime();
		string nows = timeToString(tnow,
			false, dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
		std::ofstream outfile;
		outfile.open(fpn, ios::out);
		outfile << nows + " " + printText;
		outfile.close();
		//FILE* outFile;
		//string pstr = fpn.string();
		//const char* fpn_cchar = pstr.c_str();
		//outFile = fopen(fpn_cchar, "w");
		//fprintf(outFile, printText);
		//fclose(outFile);
	}
	return true;
}

bool writeNewLog(fs::path fpn, string printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0) {
		cout << printText;
	}
	if (bprintFile > 0) {
		//time_t now = time(0);
		//tm ltm;
		//localtime_s(&ltm, &now);
		COleDateTime tnow = COleDateTime::GetCurrentTime();
		string nows = timeToString(tnow,
			false, dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
		std::ofstream outfile;
		outfile.open(fpn, ios::out);
		outfile << nows + " " + printText;
		outfile.close();
	}
	return true;
}

bool writeLog(const char* fpn, char* printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0) {
		printf(printText);
	}
	if (bprintFile > 0) {
		std::ofstream outfile;
		if (fs::exists(fpn) == false) {
			outfile.open(fpn, ios::out);
		}
		else if (fs::exists(fpn) == true) {
			outfile.open(fpn, ios::app);
		}
		//time_t now = time(0);
		//tm ltm;
		//localtime_s(&ltm, &now);
		COleDateTime tnow = COleDateTime::GetCurrentTime();
		string nows = timeToString(tnow,
			false, dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
		outfile << nows + " " + printText;
		outfile.close();
		//FILE* outFile;
		//int nResult = access(fpn, 0);
		//if (nResult == -1)
		//{
		//	outFile = fopen(fpn, "w");
		//}
		//else if (nResult == 0)
		//{
		//	outFile = fopen(fpn, "a");
		//}
		//fprintf(outFile, printText);
		//fclose(outFile);
	}
	return true;
}

bool writeLog(fs::path fpn, char* printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0) {
		printf(printText);
	}

	if (bprintFile > 0) {
		std::ofstream outfile;
		if (fs::exists(fpn) == false) {
			outfile.open(fpn, ios::out);
		}
		else if (fs::exists(fpn) == true) {
			outfile.open(fpn, ios::app);
		}
		//time_t now = time(0);
		//tm ltm;
		//localtime_s(&ltm, &now);
		COleDateTime tnow = COleDateTime::GetCurrentTime();
		string nows = timeToString(tnow,
			false, dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
		outfile << nows + " " + printText;
		outfile.close();
	}
	return true;
}

bool writeLog(fs::path fpn, string printText, int bprintFile, int bprintConsole)
{
	if (bprintConsole > 0) {
		cout << printText;// << endl;
	}
	if (bprintFile > 0) {
		std::ofstream outfile;
		if (fs::exists(fpn) == false) {
			outfile.open(fpn, ios::out);
		}
		else if (fs::exists(fpn) == true) {
			outfile.open(fpn, ios::app);
		}
		//time_t now = time(0);
		//tm ltm;
		//localtime_s(&ltm, &now);
		COleDateTime tnow = COleDateTime::GetCurrentTime();
		string nows = timeToString(tnow, 
			false, dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
		outfile << nows + " " + printText;
		outfile.close();
	}
	return true;
}


void writeTwoDimData(string fpn, double** array2D, int arrayLength_x, int arrayLength_y,
	int precision, int nodataValue)
{
	int nx = arrayLength_x;
	int ny = arrayLength_y;
	int isBigSize = 0;
	//int BigSizeThreshold = 200000000; //2억개 기준
	if (nx * ny > CONST_BIG_SIZE_ARRAY_THRESHOLD) { isBigSize = 1; }
	fs::path fpn_out = fs::path(fpn);
	std::ofstream outfile;
	outfile.open(fpn_out, ios::app);
	string formatString = "%." + to_string(precision) + "f ";
	const char* formatStr = formatString.c_str();
	if (isBigSize == 0) {
		string strALL = "";
		for (int nr = 0; nr < ny; nr++) {
			string aLineEle = "";
			for (int nc = 0; nc < nx; nc++) {
				char s1[10];
				if (array2D[nc][nr] == 0 || array2D[nc][nr] == nodataValue) {
					sprintf(s1, "%d ", (int) array2D[nc][nr]);
				}
				else {
					sprintf(s1, formatStr, array2D[nc][nr]);
				}
				aLineEle += s1;
			}
			strALL += aLineEle+ '\n';
		}
		outfile << strALL;
	}
	else {
		for (int nr = 0; nr < ny; nr++) {
			string strALine = "";
			string aLineEle = "";
			for (int nc = 0; nc < nx; nc++) {				
				char s1[20];
				if (array2D[nc][nr] == 0 || array2D[nc][nr] == nodataValue) {
					sprintf(s1, "%d ", (int) array2D[nc][nr]);
				}
				else {
					sprintf(s1, formatStr, array2D[nc][nr]);
				}
				aLineEle += s1;
			}
			strALine = aLineEle + "\n";
			outfile << strALine;
		}
	}
	outfile.close();
}

// 느리다. 2020.05.06. 최
//void writeTwoDimData2(string fpn, double** array2D, int arrayLength_x, int arrayLength_y,
//	int precision, int nodataValue)
//{
//	int nx = arrayLength_x;
//	int ny = arrayLength_y;
//	int isBigSize = 0;
//	//int BigSizeThreshold = 200000000; //2억개 기준
//	if (nx * ny > CONST_BIG_SIZE_ARRAY_THRESHOLD) { isBigSize = 1; }
//	fs::path fpn_out = fs::path(fpn);
//	std::ofstream outfile;
//	outfile.open(fpn_out, ios::app);
//	if (isBigSize == 0) {
//		string strALL = "";
//		for (int nr = 0; nr < ny; nr++) {
//			for (int nc = 0; nc < nx; nc++) {
//				string aStr = "";
//				if (array2D[nc][nr] == 0 || array2D[nc][nr] == nodataValue) {
//					aStr = dtos2(array2D[nc][nr], 0);
//					aStr += " ";
//				}
//				else {
//					aStr = dtos2(array2D[nc][nr], precision);
//					aStr += " ";
//				}
//				strALL += aStr;
//			}
//			strALL += "\n";
//		}
//		outfile << strALL;
//		outfile.close();
//	}
//	else {
//		string strALine = "";
//		for (int nr = 0; nr < ny; nr++) {
//			for (int nc = 0; nc < nx; nc++) {
//				if (array2D[nc][nr] == 0 || array2D[nc][nr] == nodataValue) {
//					string aStr = dtos(array2D[nc][nr], 0);
//					aStr += " ";
//					strALine += aStr;
//				}
//				else {
//					string aStr = dtos(array2D[nc][nr], precision);
//					aStr += " ";
//					strALine += aStr;
//				}
//			}
//			strALine += "\n";
//			outfile << strALine;
//		}
//		outfile.close();
//	}
//}






