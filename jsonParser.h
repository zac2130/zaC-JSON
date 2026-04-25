// an attempt to parse AllPrintings.json from mtgjson.com
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define JSONstring 0
#define JSONnumber 1
#define JSONobject 2
#define JSONarray 3
#define JSONboolean 4
#define JSONnull 5

/* a JSON value has 6 possible data types:
 * 	a sting
 * 	a number
 * 	an object
 * 	an array
 * 	a boolean
 * 	NULL
 *
*/

typedef struct JSONKeyValue {
	char *key;

	uint8_t type;
	void *value;
}JSONKeyValue;

typedef struct JSONObject {
	char *key;

	struct JSONObject *Parent;
	JSONKeyValue *ObjectData;
	// if applicable
	struct JSONObject *Child;	
}JSONObject;

JSONObject *parseJSON (FILE *file) {

	// load file to ram

	fseek(file, 0, SEEK_END);
	long int size = ftell(file);
	rewind(file);
	char *jsonFile = malloc(size);
	fread(jsonFile, 1, size, file);
	printf("file is %ld bytes\nsize of jsonFile pointer: %ld\n", size, sizeof(jsonFile));
	if (jsonFile == NULL){
		printf("failed to allocate memory for the file\n");
	}
	long int fileptr = 0; // at what byte are we in the file
	
	JSONObject *root = (JSONObject *)malloc(sizeof(JSONObject)); // root of the json file and return of the function

	root->key = malloc(5);
	root->key = "root";
	printf("%s\n", root->key);

	printf("root->ObjectData = %X\n", root->ObjectData);

	int index = 0;
	int placeholder = 0;
	// while character is not nul, string end
	while(jsonFile[index] != 0){
		if (jsonFile[index] == '"'){
			index++; // +1 to get the character after the "
			placeholder = index; 
			if (root->ObjectData == NULL){
				root->ObjectData = malloc(sizeof(JSONKeyValue));
			}else{
				root->ObjectData = realloc(root->ObjectData, sizeof(root->ObjectData) + sizeof(JSONKeyValue));
			}
			while (jsonFile[index] != '"'){
				index++;
			}

			char *key = malloc(index - placeholder);

			// fills a char *key with the string of a key to put later in a JSONKeyValue structur
			for (int j = 0; j < index - placeholder; j++){
				// stops at index - 1, index is '"'
				key[j] = jsonFile[placeholder + j];
			}
			printf("got key: \"%s\"\n", key);

			// advance to colomn
			while (jsonFile[index] != ':'){
				index++;
			}
			// creat JSONKeyValue * to fill
			JSONKeyValue *keyValue = malloc(sizeof(JSONKeyValue *));
			keyValue->key = key;

			// advance to value of key
			while ((jsonFile[index] | 0b11110000) != 0 && jsonFile[index] != ' ') { // first check if ascii printable character, second check if not space
				switch (jsonFile[index]){
					case '{':
						// child JSON object
						break;
					case '[':
						// array, can be any type
						break;
					case '"':
						// string, standard key/value string
						break;
					case 'n':
						// null
						break;
					default:
						// number, need to identify int or float
						break;
				}
			}
		}
		index++;
	}


	return root;
}
