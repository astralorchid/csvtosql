#include <stdio.h>
#include <stdlib.h>
#include "csvtosql.h"
#include <string.h>

int main(int argc, char* argv[]) {
	/*if (argc < 4) {
		printf("Invalid amount of arguments.");
		return -1;
	}*/
	csvFile file = openFile(argv[1]);
	if (file.fileSize == 0) {
		printf("Empty file."); printf("\n");
	return -1;};

	if (strcmp(argv[2], TABLE_CREATE_ARG)==0) {
		file.tableType = TABLE_CREATE;
	}
	else if(strcmp(argv[2], TABLE_ADD_ARG)==0){
		file.tableType = TABLE_ADD;
	}
	else {
		printf("Invalid 3rd argument: use 'create' or 'add'"); printf("\n");
		return -1;
	}
	setDatabaseName(&file, argv[3]);



	file.rows = getRows(file);
	file.columns = getColumns(file);
	generateTokenArray(&file);
	getKeyDataTypes(&file);
	generateSQLFile(file);
	printf("OK");
	return 0;
}