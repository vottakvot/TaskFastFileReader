#pragma once

#include <ctime>
#include <atlstr.h>

using namespace std;

class CLogReader {
	
	public:
		// ���������� 
		CLogReader();
		~CLogReader();
		bool Open(const char * path);
		void Close();
		bool SetFilter(const char * filter);
		bool GetNextLine(char * buf, const int bufsize);

		// ������� ���� ��� ������������
		bool createFileForTest(const char * path, const char * keyWord, const int wordRepeat, const int sizeMB);
		// ����� ���������� ���������� ������
		inline double getTimeOfReadWriteFunction(){
			return elapsedTime / 1000;
		}

		__int64 getNumberString() {
			return numberStringLast;
		}

		__int64 getCountStringInFile(){
			return countStringsInFile;
		}

		// ������������� �� ������ ���������
		UINT match(const char * pRegex, char * pstr);

		// ��� �����
		struct String {
			char * pStr = NULL;
			UINT lenght = NULL;

			// ���� ������ ������ 1 ������
			String(UINT lenght) {
				if (lenght > 0) {
					pStr = new char[lenght];
					this->lenght = lenght;
				}
			}

			// ��� �������� 
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

			// ���������� ������ ��� ��������� �������
			char getAt(const UINT position) const {
				if (position >= 0 && position < lenght) {
					return pStr[position];
				}

				return NULL;
			}

			// ������������� ����� ������
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

			// �������� �� ����� ������ � ������, � ��������� ������ � � ���������� ������ ��� �����
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

			// ����� ������
			static UINT getLenght(const char * str) {
				UINT lenght = 0;
				while (str[lenght++] != '\0');
				return lenght - 1;
			}
		};

	private:
		// ������� �� ������� ����� �������� ������
		static const char CLogReader::ALPHA_NUMERIC[];
		// ����� �� ������� ����� �������� ������
		static const char CLogReader::SYMBOLS[];
		// ����� ���������� ������
		double elapsedTime = 0;
		// ����������� ���-�� �������� � ������
		const DWORD countCols = 100;
		// ��������� �� ����
		HANDLE hFile = NULL;
		// ��������� �� �����
		HANDLE pMapHnd = NULL;
		// ��������� �� ������ 
		LPVOID pFileMap = NULL;
		// ��������(�������������), � ������� ���������� ����������� ������.
		DWORD granularitySize = NULL;
		// ������ ����� ������
		DWORD fileSizeOld = NULL;
		// ������ ����� �����
		__int64 fileSizeNew = NULL;
		// ������ ���������
		DWORD size;
		// ���������� �������� ��� ������ � �����
		__int64 fileOffset = NULL;
		// ���������� �������� ��� ������ � ������
		UINT pageOffset = NULL;
		// ��������� ������� ������ � ��������
		UINT startPos = NULL;
		// ����� ������ � ������� �����
		__int64 numberStringLast = NULL;
		// ����� ����� � ���������, ������������ ������ ����� ����� �������� �� �����
		__int64 countStringsInFile = NULL;
		// ���������� ���������
		String * pRegex = NULL;
		// ��� ������ ����� ������, ������� ��������� � ������ ��������
		char * pLastStringStart = NULL;

	private: 
		// �������� ��������� ������� �� ��������� [0; UINT_MAX], ���� ���������� ��� ������ �������
		inline int getRandomNumber(const unsigned int restrict = UINT_MAX) const {
			return rand() % restrict;
		}

		// ������ ��� ������ �������� � �������
		template <class T>
		bool isArrayContainsValue(T * array, const UINT size, T value) const;
		// ������������ ������ � ����������, ����������� ���������
		UINT matchRegex(const char * regexp, char * text);
		// ����� ������� � 0, ���� ����������� ���������� � ������� ������
		UINT matchStar(int c, const char * regexp, char * text);
};

