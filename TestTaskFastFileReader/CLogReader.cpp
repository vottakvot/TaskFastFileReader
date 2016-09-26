#include "stdafx.h"
#include "CLogReader.h"

const char CLogReader::ALPHA_NUMERIC[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char CLogReader::SYMBOLS[] = " !@#$%^&*()_+|/.,;:\"'";

// ����������
CLogReader::CLogReader(){
	srand(std::time(0));
	pRegex = new String("");
}

CLogReader::~CLogReader(){
	if (pRegex != NULL)
		delete pRegex;
}

bool CLogReader::Open(const char * path){
	clock_t begin = clock();
	elapsedTime = clock() - begin;

	fileOffset = NULL;
	pageOffset = NULL;
	numberStringLast = NULL;
	countStringsInFile = NULL;

	// ��� �������������� � ������ ��������� ���
	CString convertPath(path);

	// �������� ������ ��������(�������)
	SYSTEM_INFO sysinfo = {NULL};
	GetSystemInfo(&sysinfo);
	granularitySize = sysinfo.dwAllocationGranularity;

	// ������� ����
	hFile = CreateFile(convertPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL)
		return false;
	
	// ������ ����� ������ ������
	fileSizeOld = GetFileSize(hFile, &fileSizeOld);

	// ������ ����� ����� ������
	LARGE_INTEGER fileSizeEx = {NULL};
	GetFileSizeEx(hFile, &fileSizeEx);
	fileSizeNew = static_cast<__int64>(fileSizeEx.QuadPart);

	// ������� ���� ��������
	pMapHnd = CreateFileMapping(hFile, NULL, PAGE_READONLY, NULL, NULL, NULL);
	if (pMapHnd == NULL)
		return false;

	elapsedTime = clock() - begin;
	return true;
}

bool CLogReader::GetNextLine(char * buf, const int bufsize){
	clock_t begin = clock();
	elapsedTime = clock() - begin;

	// ��������� ��� ��������� �� ����� ����
	if (pMapHnd == NULL)
		return false;

	// ����, ����������� �� ������������ ��������, �.�. ������� ����������
	bool retFlag = false;
	// ����� ������
	UINT lineLength = NULL;
	// ���-�� �������� ��� �����������
	UINT colForCopy = NULL;
	// �������� � �����
	DWORD high = 0;
	DWORD low;
	// ����� ��������� �� ������ ������
	char * pCopyLastStringStart = NULL;

	// ������ ���� �� ���������
	for (; fileOffset < fileSizeNew; fileOffset += granularitySize) {
		low = static_cast<DWORD>(fileOffset & 0xFFFFFFFF);
		
		// ��������� ����� �.�. ������
		if (fileOffset + granularitySize > fileSizeNew) {
			granularitySize = static_cast<int>(fileSizeNew - fileOffset);
		}

		// �������� ��������� �� ������
		char * pView = static_cast<char *>(MapViewOfFile(pMapHnd, FILE_MAP_READ, 0, low, granularitySize));
		// ���������� ����� ������ ������
		pLastStringStart = &pView[pageOffset];

		// ������������� ��������
		for(; pageOffset < granularitySize; pageOffset++){
			// ����� ������, ���� ������
			// �.�. ����� ������� ��� ������ � �����������, �� ��� �� ������� ����� ������ ��� ����� ������
			if(pView[pageOffset] == '\n' || pView[pageOffset] == '\r' || pView[pageOffset] == '\0'){	
				// ��������� ������
				numberStringLast++;

				// ���� ���������� ������ �� ����� ������
				pCopyLastStringStart = (pRegex->getAt(pRegex->lenght - 1) == '$' && &pView[pageOffset] - pLastStringStart > pRegex->lenght) ?
											&pView[pageOffset] - pRegex->lenght - 3 : pLastStringStart;

				// ������� ���� ����������� ��������
				while (pageOffset < granularitySize && (pView[pageOffset] == '\n' || pView[pageOffset] == '\r' || pView[pageOffset] == '\0')) {
					pageOffset++;
				}

				// �������� ������ �� ����������
				if (pRegex->lenght > 0 && match(pRegex->pStr, pCopyLastStringStart)){
					UINT bufferStrLenght = &pView[pageOffset] - pLastStringStart;
					UINT copyStrLenght = bufferStrLenght < bufsize ? bufferStrLenght : bufsize;
					memcpy(buf, pLastStringStart, copyStrLenght);
					elapsedTime = clock() - begin;
					return true;
				}

				// ���������� ����� ������ ������
				pLastStringStart = &pView[pageOffset];
			}
		}

		// ���������� ��� ����� ��������
		pageOffset = 0;
	}

	// ���� �������� �����, �� ���������� �� ������ �����
	if (fileOffset >= fileSizeNew){
		fileOffset = 0;
		countStringsInFile = numberStringLast;
		numberStringLast = 0;
	}
		
	elapsedTime = clock() - begin;
	return false;
}

UINT CLogReader::match(const char * pRegex, char * pStr) {
	// ����� �� ������ �������� �� ������, ���������� ����� ��������� �� ������� ������� � ��������
	// ���� ���� �������� �� ���������� � ������ ������
	// ��� ������� ����� �������
	if (pRegex[0] == '!')
		return matchRegex(pRegex + 1, pStr);

	// ������� ������� ������� ������, ���� �� ����� ������ ��� �� ����������
	char * pCopyStr = pStr;
	do { 
		if(matchRegex(pRegex, pStr))
			return 1;
		pCopyStr = pStr++;
	} while (*pCopyStr != '\0' && *pCopyStr != '\n' && *pCopyStr != '\r');

	return 0;
}

UINT CLogReader::matchRegex(const char * regexp, char * pStr) {
	// ���� ����� ���������, �� ���������� ����������
	if (regexp[0] == '\0')
		return 1;
	
	// �������� �� ���������� ������� � ���������� �� 0 �� N 
	if (regexp[1] == '*')
		return matchStar(regexp[0], regexp + 2, pStr);
	
	// �������� �� ���������� � ����� ������
	if (regexp[0] == '$' && regexp[1] == '\0')
		return *pStr == '\0' || *pStr == '\n' || *pStr == '\r';
	
	// �������� �� ���������� � ������
	if (*pStr != '\0' && *pStr != '\n' && *pStr != '\r' && (regexp[0] == '?' || regexp[0] == *pStr))
		return matchRegex(regexp + 1, pStr + 1);
	
	return 0;
}

UINT CLogReader::matchStar(int prevRegC, const char * regexp, char * pStr) {
	// ����������� �������, ���� �� ����� ������ � ���������� ������ ����� ��������, ���� ������ �������
	do {
		if (matchRegex(regexp, pStr))
			return 1;
	} while (*pStr != '\0' && *pStr != '\n' && *pStr != '\r' && (*pStr++ == prevRegC || prevRegC == '?'));

	return 0;
}

void CLogReader::Close(){
	clock_t begin = clock();

	if (pMapHnd != NULL)
		CloseHandle(pMapHnd);

	if (hFile != NULL)
		CloseHandle(hFile);

	elapsedTime = clock() - begin;
}

bool CLogReader::SetFilter(const char * filter){	
	if (String::getLenght(filter) == 0)
		return false;

	if (!pRegex->setString(filter))
		return false;

	return true;
}

bool CLogReader::createFileForTest(const char * path, const char * keyWord, const int wordRepeat, const int sizeMB){
	clock_t begin = clock();
	elapsedTime = clock() - begin;

	// �������� ����� ��� ������
	FILE * testDataFile = fopen(path, "w");
	int cWordRepeat = wordRepeat;

	// �������� ���� ����������� ������
	if (testDataFile == NULL || wordRepeat < 1 || sizeMB < 1 || (keyWord != NULL && keyWord[0] == '\0'))
		return false;

	// ������ ����� ������/�������
	UINT countChars = sizeMB * 1024 * 1024 / (sizeof(char) * 2);
	UINT countRow = countChars / countCols;

	// � ����� ������ ������� ������� ��� ������
	UINT * numberStrings = new UINT[wordRepeat];
	for (int i = 0; i < wordRepeat; i++)
		numberStrings[i] = CLogReader::getRandomNumber(countRow);

	// ��������� ����
	for (int j = 0; j < countRow; j++){
		int positionForPaste = CLogReader::getRandomNumber(countCols);
		for (int i = 0; i < countCols + CLogReader::getRandomNumber(10); i++){
			fprintf(testDataFile, "%c", CLogReader::ALPHA_NUMERIC[CLogReader::getRandomNumber(sizeof(CLogReader::ALPHA_NUMERIC) - 1)]);
			fprintf(testDataFile, "%c", CLogReader::SYMBOLS[CLogReader::getRandomNumber(sizeof(CLogReader::SYMBOLS) - 1)]);
			
			// �������� �������� � ������������ ������� � ������
			if(CLogReader::isArrayContainsValue(numberStrings, wordRepeat, (UINT)j) && positionForPaste == i && cWordRepeat-- >= 0)
				fprintf(testDataFile, "%s", keyWord);
		}
			
		fprintf(testDataFile, "\n");

		// ������� �� �������
		if (j % 1000 == 0) {
			printf("\rGenerate file strings: %i/%i", j, countRow);
		}
	}

	// ���� ��
	printf("\rGenerate file strings: %i/%i", countRow, countRow);

	// ��������� ��� ��������
	delete [] numberStrings;
	fclose(testDataFile);
	elapsedTime = clock() - begin;

	return true;
}

template <class T>
bool CLogReader::isArrayContainsValue(T * array, const UINT size, T value) const {
	for (int i = 0; i < size; i++){
		if (array[i] == value)
			return true;
	}

	return false;
}