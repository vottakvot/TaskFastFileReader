// TestTaskFastFileReader.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

using namespace std;

const char * pFileArg = "--filePath";
const char * pRegexArg = "--regex";
const char * pHelpArg = "--help";
const char * pCreateTest = "--test";

// Номер аргумента с путем
int numArgFilePath = 0;
// Номер аргумента с выражением
int numArgRegex = 0;

// Вывод подсказки
void outputHelp();

int main(int argc, char *argv[])
{
	CLogReader * cLogReader = new CLogReader();
	const int bufferSize = 256;
	char buffer[bufferSize];

	// Проверяем аргументы
	for (int i = 0; i < argc; i++){
		// Путь к файлу
		if(cLogReader->match(pFileArg, argv[i])){
			numArgFilePath = i + 1;
		}

		// Выражение для поиска
		if (cLogReader->match(pRegexArg, argv[i])) {
			numArgRegex = i + 1;
		}

		// Выражение для поиска
		if (cLogReader->match(pHelpArg, argv[i])) {
			outputHelp();
			cin.get();
			return 0;
		}

		// Создать файл для тестов
		if (cLogReader->match(pCreateTest, argv[i]) && argc >= 6) {
			if (cLogReader->createFileForTest(argv[i + 1], argv[i + 2], atoi(argv[i + 3]), atoi(argv[i + 4]))){
				cout << endl << "File create! " << argv[i + 1] << endl;
			} else
				cout << endl << "Cant't create file! " << argv[i + 1] << endl;
			
			cin.get();
			return 0;
		}
	}

	// Информацинное сообщение
	if (numArgFilePath == 0 || numArgRegex == 0){
		outputHelp();
		return 0;
	}

	clock_t begin = clock();
	if (cLogReader->Open(argv[numArgFilePath])){
		cout << "Time for open: " << cLogReader->getTimeOfReadWriteFunction() << endl;
		if(cLogReader->SetFilter(argv[numArgRegex])){
			while (cLogReader->GetNextLine(buffer, bufferSize)) {
				cout << "Row number: " << cLogReader->getNumberString() << endl;
				for (int i = 0; i < sizeof(buffer); i++)
					if (buffer[i] != '\0')
						cout << buffer[i];

				cout << endl << "Time: " << cLogReader->getTimeOfReadWriteFunction() << endl << endl;
			}
		}

		cLogReader->Close();
		cout << "Search for regex: " << argv[numArgRegex] << endl;
		cout << "Count lines: " << cLogReader->getCountStringInFile() << endl;
		cout << "Total time: " << (float)(clock() - begin) / 1000.0f << endl;
	} else {
			cout << "Cant't open file: " << argv[numArgFilePath] << endl;
		}
	
	cin.get();
	return 0;
}

void outputHelp(){
	cout << "Arguments for use: " << endl;
	cout << "    " << "--filePath <Path_to_file>" << endl;
	cout << "    " << "--regex <Regular expression>" << endl;
	cout << "Full example: TestReadFile.exe --filePath C:\\FILE.txt --regex SEARCH_PHRASE" << "" << endl << endl;
	cout << "Regex format:" << endl;
	cout << "    " << "Match in beginning of string: !PHRASE" << endl;
	cout << "    " << "Match in ending of string: PHRASE$" << endl;
	cout << "    " << "Any symbole: PHR?SE" << endl;
	cout << "    " << "Exist infinity or zero match of previous of *: PHRA*SE" << endl << endl;
	cout << "For create test file: " << "TestReadFile.exe --test FILE_PATH SEARCH_PHRASE REPEAT_PHRASE FILE_SIZE" << endl;
}