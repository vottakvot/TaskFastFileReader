#pragma once

#include <ctime>
#include <atlstr.h>

using namespace std;

class CLogReader {
	
	public:
		// Требование 
		CLogReader();
		~CLogReader();
		bool Open(const char * path);
		void Close();
		bool SetFilter(const char * filter);
		bool GetNextLine(char * buf, const int bufsize);

		// Создаем файл для тестирования
		bool createFileForTest(const char * path, const char * keyWord, const int wordRepeat, const int sizeMB);
		// Время выполнения последнего метода
		inline double getTimeOfReadWriteFunction(){
			return elapsedTime / 1000;
		}

		__int64 getNumberString() {
			return numberStringLast;
		}

		__int64 getCountStringInFile(){
			return countStringsInFile;
		}

		// Соответствует ли строка выражению
		UINT match(const char * pRegex, char * pstr);

		// Для строк
		struct String {
			char * pStr = NULL;
			UINT lenght = NULL;

			// Если строка хотябы 1 символ
			String(UINT lenght) {
				if (lenght > 0) {
					pStr = new char[lenght];
					this->lenght = lenght;
				}
			}

			// Для значений 
			String(const char * pStrInp) {
				if (pStrInp != NULL) {
					lenght = String::getLenght(pStrInp);
					if (lenght > 0) {
						pStr = new char[lenght];
						memcpy(pStr, pStrInp, lenght + 1);
					}
				}
			}

			~String() {
				if (pStr != NULL)
					delete[] pStr;

				pStr = NULL;
				lenght = NULL;
			}

			// Возвращает символ для указанной позиции
			char getAt(const UINT position) const {
				if (position >= 0 && position < lenght) {
					return pStr[position];
				}

				return NULL;
			}

			// Устанавливаем новую строку
			bool setString(const char * newStr) {
				UINT lenght = String::getLenght(newStr);
				if (lenght > 0) {
					this->~String();
					pStr = new char[lenght + 1];
					this->lenght = lenght;
					memcpy(pStr, newStr, lenght + 1);
					return true;
				}

				return false;
			}

			// Копируем из одной строки в другую, с удалением старой и с выделением памяти под новую
			static UINT copyStr(const String * str1, String * str2) {
				if (str1->lenght > 0) {
					str2->~String();
					str2->pStr = new char[str1->lenght + 1];
					str2->lenght = str1->lenght;
					memcpy(str2->pStr, str1->pStr, str1->lenght + 1);
					return str1->lenght;
				}

				return 0;
			}

			// Длина строки
			static UINT getLenght(const char * str) {
				UINT lenght = 0;
				while (str[lenght++] != '\0');
				return lenght - 1;
			}
		};

	private:
		// Символы из которых будет состоять строка
		static const char CLogReader::ALPHA_NUMERIC[];
		// Знаки из которых будет состоять строка
		static const char CLogReader::SYMBOLS[];
		// Время выполнения метода
		double elapsedTime = 0;
		// Минимальное кол-во символов в строке
		const DWORD countCols = 100;
		// Указатель на файл
		HANDLE hFile = NULL;
		// Указатель на карту
		HANDLE pMapHnd = NULL;
		// Указатель на данные 
		LPVOID pFileMap = NULL;
		// Страница(гранулярность), с которой выделяется виртуальная память.
		DWORD granularitySize = NULL;
		// Размер файла старое
		DWORD fileSizeOld = NULL;
		// Размер файла новое
		__int64 fileSizeNew = NULL;
		// Размер выражения
		DWORD size;
		// Предыдущее смещение для поиске в файле
		__int64 fileOffset = NULL;
		// Предыдущее смещение для поиске в строке
		UINT pageOffset = NULL;
		// Начальная позиция строки в странице
		UINT startPos = NULL;
		// Номер строки в котором нашли
		__int64 numberStringLast = NULL;
		// Всего строк в документе, выставляется только после поной проходки по файлу
		__int64 countStringsInFile = NULL;
		// Регулярное выражение
		String * pRegex = NULL;
		// Для буфера конца строки, который находится в другой странице
		char * pLastStringStart = NULL;

	private: 
		// Получить случайное значние из диапазона [0; UINT_MAX], либо указанного при вызове функции
		inline int getRandomNumber(const unsigned int restrict = UINT_MAX) const {
			return rand() % restrict;
		}

		// Шаблон для поиска элемента в массиве
		template <class T>
		bool isArrayContainsValue(T * array, const UINT size, T value) const;
		// Сопоставляем строку с выражением, рекурсивным перебором
		UINT matchRegex(const char * regexp, char * text);
		// Поиск символа с 0, либо бесконечным вхождением в искомую строку
		UINT matchStar(int c, const char * regexp, char * text);
};

