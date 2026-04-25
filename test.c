#include "jsonParser.h"
#include <stdio.h>

int main () {
	FILE *file = fopen("example.json", "r");

	if (file == NULL){
		printf("failed to open file\n");
	}else{
		printf("file opened successfully\n");
	}

	parseJSON(file);
	printf("\n");
	return 0;
}
