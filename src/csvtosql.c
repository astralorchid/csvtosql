#include "csvtosql.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

uint32_t getFileSize(FILE* fptr) {
	char c;
	uint32_t fileSize = 0;
	c = getc(fptr);
	while (c != EOF) {
		fileSize += 1;
		c = getc(fptr);
	}
	rewind(fptr);
	return fileSize;
}

csvFile openFile(char* filename) {
	csvFile* newCsvFile = malloc(sizeof(csvFile));
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Error opening CSV file\n");
		newCsvFile->fileSize = 0;
		return *newCsvFile;
	}

	uint32_t fs = getFileSize(fp);
	if (fs > 0) { //if file size > 0
		newCsvFile->fileSize = fs;
		newCsvFile->rawBuffer = malloc(sizeof(char) * fs + 1);

		if (newCsvFile->rawBuffer != NULL) {
			newCsvFile->rawBuffer[0] = 0x0A;
			newCsvFile->rawBuffer[fs + 1] = 0;
			for (uint32_t i = 1; i < fs + 1; i++) {
				newCsvFile->rawBuffer[i] = getc(fp);
			}
		}
	}
	fclose(fp);
	return *newCsvFile;
}

uint32_t getRows(csvFile _file) {
	uint32_t fileSize = _file.fileSize;
	uint32_t numRows = 0;
	for (uint32_t  i = 0; i < fileSize + 1; i++) {
		if (_file.rawBuffer[i] == 0x0A) {
			numRows += 1;
		}
	}
	return numRows;
}

uint32_t getColumns(csvFile _file) {
	uint32_t fileSize = _file.fileSize;
	uint32_t numColumns = 1;

	for (uint32_t  i = 1; i < fileSize; i++) {
		if (_file.rawBuffer[i] == (char)0x0A) {
			break;
		}else if (_file.rawBuffer[i] == ',') {
			numColumns += 1;
		}
	}
	return numColumns;
}

void setDatabaseName(csvFile* _file, char* name) {
	uint32_t nameLength = strlen(name);
	_file->databaseNameSize = nameLength;
	_file->databaseName = malloc(sizeof(char) * (nameLength + 1));
	_file->databaseName[nameLength] = '\0';
	for (uint32_t i = 0; i < nameLength; i++) {
		_file->databaseName[i] = name[i];
	}
}

void generateTokenArray(csvFile* _file) {
	_file->tokens = malloc((sizeof(uint32_t) * _file->rows*_file->columns)+1);

	uint32_t currentRow = 0, currentColumn = 0, entrySize = 0;
	for (uint32_t i = 1; i <= _file->fileSize + 1; i++) {

		switch ((char)_file->rawBuffer[i]) {
		case(LINE_FEED):
			if (currentColumn <= _file->columns) {
				createToken(_file, entrySize, currentRow, currentColumn, i - entrySize);
			}
			currentRow += 1;
			currentColumn = 0;
		case(','):
			if (currentColumn <= _file->columns) {
				createToken(_file, entrySize, currentRow, currentColumn, i - entrySize);
			}
			currentColumn += 1;
			entrySize = 0;
		default:
			entrySize += 1;
		}
	}
	createToken(_file, entrySize, currentRow, currentColumn, (_file->fileSize + 2) - entrySize);
}

void populateTokenArray(csvFile* _file) {

}

void createToken(csvFile* _file, uint32_t size, uint32_t row, uint32_t column, uint32_t bufferPos) {
	if (size > 0) {
		char* newToken = malloc(sizeof(char) * (size+1));
		_file->tokens[(row * _file->columns)+column] = newToken;
		if (_file->rawBuffer[bufferPos] == ',' || _file->rawBuffer[bufferPos] == 0x0A) {
			bufferPos += 1;
			size -= 1;
		}
		for (uint32_t i = 0; i < size; i++) {
			newToken[i] = _file->rawBuffer[bufferPos + i];
		}
		newToken[size] = '\0';
	}
	else {
		char* newToken = malloc(sizeof(char));
		_file->tokens[(row * _file->columns) + column] = newToken;
		newToken[0] = '\0';
	}
}

char* getCsvFileItem(csvFile _file, uint32_t row, uint32_t column) {
	if ((row-1 > _file.rows || row <= 0) || (column > _file.columns || column <= 0)) {
		//printf("Item does not exist.");
		return "\0";
	}
	else {
		char* wantedToken = _file.tokens[((row - 1) * _file.columns) + (column)];
		if (wantedToken[0] == 0) {
			//printf("Item does not exist.");
			return "\0";
		}
		if (row == 1) {
			return _file.tokens[((row - 1) * _file.columns) + (column-1)];
		}
		else {
			return _file.tokens[((row - 1) * _file.columns) + (column)];
		}
	}
}
/*
getKeyDataType flags:
0 - contains non-digit characters
1 - contains digit characters
2 - contains .
3 - contains space
4 - DATE format
5 - TIME format
6 - DATETIME format
*/

void getKeyDataTypes(csvFile* _file) {
	_file->keys = malloc(sizeof(uint32_t) * _file->columns);
	uint32_t* keys;
	keys = _file->keys;

	for (uint32_t column = 1; column <= _file->columns; column++) {
		SQLTableKey* newKey = malloc(sizeof(SQLTableKey));
		keys[column - 1] = newKey;
		newKey->name = getCsvFileItem(*_file, 1, column);
		newKey->nameLen = strlen(newKey->name);
		newKey->maxStringLen = 0;
		newKey->maxD = 0;

		bool flags[] = { false, false, false, false, false, false, false, false };
		for (uint32_t row = 2; row <= _file->rows; row++) {
			char* token = getCsvFileItem(*_file, row, column);
			uint32_t tokenLen = strlen(token);

			if (tokenLen > newKey->maxStringLen) {
				newKey->maxStringLen = tokenLen;
			}
			setDataTypeFlags(token, tokenLen, flags); //weird return usage but speeds things up a bit
			bool hasPeriod = false;
			uint32_t maxD = 0;
			for (uint32_t tokenChar = 0; tokenChar < tokenLen; tokenChar++) {
				char _tokenChar = token[tokenChar];
				if (hasPeriod == true) {
					maxD += 1;
				}
				if (_tokenChar == '.') {
					hasPeriod = true;
				}
			}
			if (maxD > newKey->maxD) {
				newKey->maxD = maxD;
			}
			printf("MAXD %u", maxD); printf("\n");
		}
		applyDataTypeFlags(newKey, flags);

		printf("%u", column); printf("\n");
		printf(newKey->name); printf("\n");
		printf("DATA TYPE: ");  printf("%u", newKey->dataType); printf("\n");
		printf("%u", newKey->maxD); printf("\n");
	}
}

void setDataTypeFlags(char* token, uint32_t tokenLen, bool* flags) {
	for (uint32_t tokenChar = 0; tokenChar < tokenLen; tokenChar++) {
		char _tokenChar = token[tokenChar];
		if (_tokenChar == ' ') {
			flags[FLAG_SPACE] = true;
		}
		else if (_tokenChar == '.') {
			if (flags[FLAG_PERIOD] == false) {
				flags[FLAG_PERIOD] = true;
			}
		}
		else if (_tokenChar >= 0x30 && _tokenChar <= 0x39) {
			flags[FLAG_DIGIT] = true;
			if (flags[FLAG_PERIOD] == true) {
			}
		}
		else if ((_tokenChar < 0x30 || _tokenChar > 0x39) && (tokenChar != '.')) {
			flags[FLAG_NON_DIGIT] = true;
		}
	}
}

void applyDataTypeFlags(SQLTableKey* key, bool* flags) {
	if (flags[FLAG_NON_DIGIT] == true) {
		key->dataType = DATATYPE_VARCHAR;
	}
	else if (flags[FLAG_DIGIT] == true) {
		if (flags[FLAG_PERIOD] == true) {
			key->dataType = DATATYPE_DECIMAL;
		}
		else {
			key->dataType = DATATYPE_VARCHAR;
		}
		if (flags[FLAG_PERIOD] == false) {
			key->dataType = DATATYPE_INT;
		}
	}

	if (key->dataType == DATATYPE_DECIMAL) {

	}
}

void writeDatabaseName(csvFile _file, FILE* sqlFilePtr) {
	for (uint32_t dbNameIndex = 0; dbNameIndex < _file.databaseNameSize; dbNameIndex++) {
		fputc(_file.databaseName[dbNameIndex], sqlFilePtr);
	}
}
void generateSQLFile(csvFile _file) {
	char* sqlFileName = malloc(sizeof(char) * _file.databaseNameSize + 6);
	char* fileExtension = ".sql\0";
	for (uint32_t dbNameIndex = 0; dbNameIndex < _file.databaseNameSize; dbNameIndex++) {
		sqlFileName[dbNameIndex] = _file.databaseName[dbNameIndex];
	}
	sqlFileName[_file.databaseNameSize] = '\0';
	strcat(sqlFileName, fileExtension);
	printf(sqlFileName);
	FILE* sqlFilePtr;
	sqlFilePtr = fopen(sqlFileName, "w");

	if (_file.tableType == TABLE_CREATE) {
		uint32_t* keys;
		keys = _file.keys;
		SQLTableKey* key = keys[0];

		fputs("CREATE TABLE ", sqlFilePtr);

		writeDatabaseName(_file, sqlFilePtr);

		fputs("(", sqlFilePtr);

		for (uint32_t nameIndex = 0; nameIndex < key->nameLen; nameIndex++) {
			fputc(key->name[nameIndex], sqlFilePtr);
		}
		fputc(' ', sqlFilePtr);

		if (key->dataType == DATATYPE_INT) {
			fputs("INT", sqlFilePtr);
		}

		else if (key->dataType == DATATYPE_VARCHAR) {
			char strLen[6];
			sprintf(strLen, "%u", key->maxStringLen);
			fputs("VARCHAR(", sqlFilePtr);
			for (uint32_t strIndex = 0; strIndex < 6; strIndex++) {
				if (strLen[strIndex] == '\0') { break; }
				else {
					fputc(strLen[strIndex], sqlFilePtr);
				}
			}
			fputc(')', sqlFilePtr);
		}

		else if (key->dataType == DATATYPE_DECIMAL) {
			char strLen[3];
			sprintf(strLen, "%u", key->maxStringLen - 1);
			//fputs("DECIMAL(", sqlFilePtr);
			for (uint32_t strIndex = 0; strIndex < 6; strIndex++) {
				if (strLen[strIndex] == '\0') { break; }
				else {
					fputc(strLen[strIndex], sqlFilePtr);
				}
			}
			fputs(", ", sqlFilePtr);

			sprintf(strLen, "%u", key->maxD);
			for (uint32_t strIndex = 0; strIndex < 6; strIndex++) {
				if (strLen[strIndex] == '\0') { break; }
				else {
					fputc(strLen[strIndex], sqlFilePtr);
				}
			}

			fputc(')', sqlFilePtr);
		}

	fputs(");\n", sqlFilePtr);

		for (uint32_t column = 1; column < _file.columns; column++) {
			uint32_t* keys;
			keys = _file.keys;
			SQLTableKey* key = keys[column];
			fputs("ALTER TABLE ", sqlFilePtr);

			writeDatabaseName(_file, sqlFilePtr);

			fputs("\n", sqlFilePtr);
			fputs("ADD COLUMN ", sqlFilePtr);

			for (uint32_t nameIndex = 0; nameIndex < key->nameLen; nameIndex++) {
				fputc(key->name[nameIndex], sqlFilePtr);
			}

			fputc(' ', sqlFilePtr);

			if (key->dataType == DATATYPE_INT) {
				fputs("INT", sqlFilePtr);
			}

			else if (key->dataType == DATATYPE_VARCHAR) {
				char strLen[6];
				sprintf(strLen, "%u", key->maxStringLen);
				fputs("VARCHAR(", sqlFilePtr);
				for (uint32_t strIndex = 0; strIndex < 6; strIndex++) {
					if (strLen[strIndex] == '\0') { break; }
					else {
						fputc(strLen[strIndex], sqlFilePtr);
					}
				}
				fputc(')', sqlFilePtr);
			}

			else if (key->dataType == DATATYPE_DECIMAL) {
				char strLen[3];
				sprintf(strLen, "%u", key->maxStringLen - 1);
				fputs("DECIMAL(", sqlFilePtr);
				for (uint32_t strIndex = 0; strIndex < 6; strIndex++) {
					if (strLen[strIndex] == '\0') { break; }
					else {
						fputc(strLen[strIndex], sqlFilePtr);
					}
				}
				fputs(", ", sqlFilePtr);

				sprintf(strLen, "%u", key->maxD);
				for (uint32_t strIndex = 0; strIndex < 6; strIndex++) {
					if (strLen[strIndex] == '\0') { break; }
					else {
						fputc(strLen[strIndex], sqlFilePtr);
					}
				}
				fputc(')', sqlFilePtr);
			}
			fputc(';', sqlFilePtr);
			fputs("\n", sqlFilePtr);
		}
	}

	for (uint32_t row = 2; row <= _file.rows; row++) {
		fputs("INSERT INTO ", sqlFilePtr);
		writeDatabaseName(_file, sqlFilePtr);
		fputs(" VALUE(", sqlFilePtr);

		for (uint32_t column = 1; column <= _file.columns; column++) {
			uint32_t* keys;
			keys = _file.keys;
			SQLTableKey* key = keys[column-1];
			char* valString = getCsvFileItem(_file, row, column);
			if (key->dataType == DATATYPE_VARCHAR) {
				fputc('\'', sqlFilePtr);
				for (uint32_t len = 0; len < key->maxStringLen; len++) {
					if (valString[len] == '\0') { break; }
					else {
						fputc(valString[len], sqlFilePtr);
					}
				}
				fputc('\'', sqlFilePtr);
			}
			else {
				for (uint32_t len = 0; len < key->maxStringLen; len++) {
					if (valString[len] == '\0') { break; }
					else {
						fputc(valString[len], sqlFilePtr);
					}
				}
			}
			if (column != _file.columns) {
				fputs(", ", sqlFilePtr);
			}
		}
		fputs(");\n", sqlFilePtr);
	}

	fclose(sqlFilePtr);
}

