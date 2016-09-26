#include "stdafx.h"
#include "CLogReader.h"

const char CLogReader::ALPHA_NUMERIC[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char CLogReader::SYMBOLS[] = " !@#$%^&*()_+|/.,;:\"'";

// Требование
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

	// Для преобразования в родной строковой тип
	CString convertPath(path);

	// Получаем размер страницы(гранулы)
	SYSTEM_INFO sysinfo = {NULL};
	GetSystemInfo(&sysinfo);
	granularitySize = sysinfo.dwAllocationGranularity;

	// Открыть файл
	hFile = CreateFile(convertPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL)
		return false;
	
	// Размер файла первый способ
	fileSizeOld = GetFileSize(hFile, &fileSizeOld);

	// Размер файла новый способ
	LARGE_INTEGER fileSizeEx = {NULL};
	GetFileSizeEx(hFile, &fileSizeEx);
	fileSizeNew = static_cast<__int64>(fileSizeEx.QuadPart);

	// Открыть файл маппинга
	pMapHnd = CreateFileMapping(hFile, NULL, PAGE_READONLY, NULL, NULL, NULL);
	if (pMapHnd == NULL)
		return false;

	elapsedTime = clock() - begin;
	return true;
}

bool CLogReader::GetNextLine(char * buf, const int bufsize){
	clock_t begin = clock();
	elapsedTime = clock() - begin;

	// Проверить что указатель на карту есть
	if (pMapHnd == NULL)
		return false;

	// Флаг, указывающий на необходиость возврата, т.к. найдено совпадение
	bool retFlag = false;
	// Длина строки
	UINT lineLength = NULL;
	// Кол-во символов для копирования
	UINT colForCopy = NULL;
	// Смещения в файле
	DWORD high = 0;
	DWORD low;
	// Копия указателя на начало строки
	char * pCopyLastStringStart = NULL;

	// Читаем файл по страницам
	for (; fileOffset < fileSizeNew; fileOffset += granularitySize) {
		low = static_cast<DWORD>(fileOffset & 0xFFFFFFFF);
		
		// Последняя часть м.б. меньше
		if (fileOffset + granularitySize > fileSizeNew) {
			granularitySize = static_cast<int>(fileSizeNew - fileOffset);
		}

		// Получаем указатель на данные
		char * pView = static_cast<char *>(MapViewOfFile(pMapHnd, FILE_MAP_READ, 0, low, granularitySize));
		// Запоминаем адрес начала строки
		pLastStringStart = &pView[pageOffset];

		// Просматриваем страницу
		for(; pageOffset < granularitySize; pageOffset++){
			// Конец строки, либо текста
			// Т.к. нужно вернуть всю строку с совпадением, то идём до символа конца строки или новой строки
			if(pView[pageOffset] == '\n' || pView[pageOffset] == '\r' || pView[pageOffset] == '\0'){	
				// Очередная строка
				numberStringLast++;

				// Если совпадение только по концу строки
				pCopyLastStringStart = (pRegex->getAt(pRegex->lenght - 1) == '$' && &pView[pageOffset] - pLastStringStart > pRegex->lenght) ?
											&pView[pageOffset] - pRegex->lenght - 3 : pLastStringStart;

				// Пропуск всех управляющих символом
				while (pageOffset < granularitySize && (pView[pageOffset] == '\n' || pView[pageOffset] == '\r' || pView[pageOffset] == '\0')) {
					pageOffset++;
				}

				// Проверка строки на совпадение
				if (pRegex->lenght > 0 && match(pRegex->pStr, pCopyLastStringStart)){
					UINT bufferStrLenght = &pView[pageOffset] - pLastStringStart;
					UINT copyStrLenght = bufferStrLenght < bufsize ? bufferStrLenght : bufsize;
					memcpy(buf, pLastStringStart, copyStrLenght);
					elapsedTime = clock() - begin;
					return true;
				}

				// Запоминаем адрес начала строки
				pLastStringStart = &pView[pageOffset];
			}
		}

		// Сбрасываем для новой страницы
		pageOffset = 0;
	}

	// Если достигли конца, то сбрасываем на начало файла
	if (fileOffset >= fileSizeNew){
		fileOffset = 0;
		countStringsInFile = numberStringLast;
		numberStringLast = 0;
	}
		
	elapsedTime = clock() - begin;
	return false;
}

UINT CLogReader::match(const char * pRegex, char * pStr) {
	// Чтобы не бегать повторно по строке, используем общий указатель на текущую позицию в странице
	// Если есть указание на совпадение с начала строки
	// Для длинных строк быстрее
	if (pRegex[0] == '!')
		return matchRegex(pRegex + 1, pStr);

	// Перебор каждого символа текста, пока не конец строки или не совпадение
	char * pCopyStr = pStr;
	do { 
		if(matchRegex(pRegex, pStr))
			return 1;
		pCopyStr = pStr++;
	} while (*pCopyStr != '\0' && *pCopyStr != '\n' && *pCopyStr != '\r');

	return 0;
}

UINT CLogReader::matchRegex(const char * regexp, char * pStr) {
	// Если конец выражения, то возвращаем совпадение
	if (regexp[0] == '\0')
		return 1;
	
	// Проверка по совпадению символа с вхождением от 0 до N 
	if (regexp[1] == '*')
		return matchStar(regexp[0], regexp + 2, pStr);
	
	// Проверка на совпадение в конце строки
	if (regexp[0] == '$' && regexp[1] == '\0')
		return *pStr == '\0' || *pStr == '\n' || *pStr == '\r';
	
	// Проверка на совпадение в тексте
	if (*pStr != '\0' && *pStr != '\n' && *pStr != '\r' && (regexp[0] == '?' || regexp[0] == *pStr))
		return matchRegex(regexp + 1, pStr + 1);
	
	return 0;
}

UINT CLogReader::matchStar(int prevRegC, const char * regexp, char * pStr) {
	// Рекурсивный перебор, пока не конец строки и предыдущий символ равен текущему, либо любому символу
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

	// Открытие файла для записи
	FILE * testDataFile = fopen(path, "w");
	int cWordRepeat = wordRepeat;

	// Проверка всех необходимых данных
	if (testDataFile == NULL || wordRepeat < 1 || sizeMB < 1 || (keyWord != NULL && keyWord[0] == '\0'))
		return false;

	// Формат файла строки/столбцы
	UINT countChars = sizeMB * 1024 * 1024 / (sizeof(char) * 2);
	UINT countRow = countChars / countCols;

	// В какие строки вставим символы для поиска
	UINT * numberStrings = new UINT[wordRepeat];
	for (int i = 0; i < wordRepeat; i++)
		numberStrings[i] = CLogReader::getRandomNumber(countRow);

	// Заполняем файл
	for (int j = 0; j < countRow; j++){
		int positionForPaste = CLogReader::getRandomNumber(countCols);
		for (int i = 0; i < countCols + CLogReader::getRandomNumber(10); i++){
			fprintf(testDataFile, "%c", CLogReader::ALPHA_NUMERIC[CLogReader::getRandomNumber(sizeof(CLogReader::ALPHA_NUMERIC) - 1)]);
			fprintf(testDataFile, "%c", CLogReader::SYMBOLS[CLogReader::getRandomNumber(sizeof(CLogReader::SYMBOLS) - 1)]);
			
			// Помещаем значение в произвольную позицию в строке
			if(CLogReader::isArrayContainsValue(numberStrings, wordRepeat, (UINT)j) && positionForPaste == i && cWordRepeat-- >= 0)
				fprintf(testDataFile, "%s", keyWord);
		}
			
		fprintf(testDataFile, "\n");

		// Выводим на консоль
		if (j % 1000 == 0) {
			printf("\rGenerate file strings: %i/%i", j, countRow);
		}
	}

	// Типа всё
	printf("\rGenerate file strings: %i/%i", countRow, countRow);

	// Завершаем все действия
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