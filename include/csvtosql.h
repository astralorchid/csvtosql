#ifndef CSVTOSQL_H
#define CSVTOSQL_H
#include <stdio.h>
#include<stdint.h>
#include <stdbool.h>

#define TABLE_CREATE 1
#define TABLE_CREATE_ARG "create\0"
#define TABLE_ADD_ARG "add\0"
#define TABLE_ADD 2
#define LINE_FEED 0x0A

#define DATATYPE_INT 1
#define DATATYPE_VARCHAR 2
#define DATATYPE_DECIMAL 3
#define DATATYPE_DATE 4
#define DATATYPE_TIME 5
#define DATATYPE_DATETIME 6

#define FLAG_NON_DIGIT 0
#define FLAG_DIGIT 1
#define FLAG_PERIOD 2
#define FLAG_SPACE 3
#define FLAG_TYPE_DATE 4
#define FLAG_TYPE_TIME 5
#define FLAG_TYPE_DATETIME 6
#define FLAG_MULTIPLE_PERIOD 7

typedef struct {
	char* rawBuffer; //ALLOCATED
	char* databaseName; //ALLOCATED
	uint32_t* tokens, keys;
	uint32_t rows, columns, fileSize, databaseNameSize, tableType;
}csvFile;

typedef struct {
	char* name;
	uint32_t dataType, maxStringLen, maxD, nameLen;
	bool* flags;
} SQLTableKey;

csvFile openFile(char* filename);
uint32_t getFileSize(FILE* fptr);
uint32_t getRows(csvFile _file);
uint32_t getColumns(csvFile _file);
void setDatabaseName(csvFile* _file, char* name);
void generateTokenArray(csvFile* _file);
void populateTokenArray(csvFile* _file);
void createToken(csvFile* _file, uint32_t size, uint32_t row, uint32_t column, uint32_t bufferPos);
char* getCsvFileItem(csvFile _file, uint32_t row, uint32_t column);
void getKeyDataTypes(csvFile* _file);
void setDataTypeFlags(char* token, uint32_t tokenLen, bool* flags);
void applyDataTypeFlags(SQLTableKey* key, bool* flags);
void generateSQLFile(csvFile _file);
void writeDatabaseName(csvFile _file, FILE* sqlFilePointer);
#endif 
