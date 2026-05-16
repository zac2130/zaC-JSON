// an attempt to parse AllPrintings.json from mtgjson.com
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#define PTRSIZE sizeof(int *)

#define JSONSTRING 0
#define JSONNUMBER 1
#define JSONOBJECT 2
#define JSONARRAY 3
#define JSONBOOLEAN 4
#define JSONNULL 5

/* a JSON value has 6 possible data types:
 * 	a string
 * 	a number
 * 	an object
 * 	an array
 * 	a boolean
 * 	NULL
 *
*/

typedef int bool;

typedef struct JSONKeyValue {
	char *key;

	uint8_t type;
	void *value;
}JSONKeyValue;

typedef struct JSONObject {
	char *key;
	struct JSONObject *Parent; /* pointer to the parent of this JSON object */
	JSONKeyValue *ObjectData; /* an array of key/value pairs */
}JSONObject;

/* external placeholder typed variable pointers for typecasting the value void pointer of JSONKeyValue */
char *JSONTypeString;
long *JSONTypeInt;
double *JSONTypeFloat;
JSONObject *JSONTypeObject;
char **JSONTypeArray;
bool *JSONTypeBool;


/* define a publically accessible pointer to the root of the JSON file */
JSONObject *root = malloc(PTRSIZE);

void InitRootObject(){
	extern JSONObject *root;
	root->key = malloc(5);
	root->key = "root"; /* hard coded key assosiated with the root object in given file */
}

static void skipTo(char character, int *ptr, char *array){
	while (array[*ptr] != character && array[*ptr] != '\0') {
		(*ptr)++;
	}
	if (array[*ptr] == '\0') printf("reached end of array (\\0)\n");
}

static void addKeyValuePair(JSONKeyValue *keyValuePairArray) {
	if (keyValuePairArray == NULL) keyValuePairArray = malloc(PTRSIZE);
	else {
	       keyValuePairArray = realloc(keyValuePairArray, sizeof(keyValuePairArray + ));
	       if (keyValuePairArray == NULL) {
		       printf("Failed to reallocate key/value pair array\n\n");
		       return;
	       }
	}
}

JSONObject *spawnJSON(char* key, JSONObject *parent) {
	JSONObject *child = malloc(PTRSIZE);

	if (child == NULL) printf("Failed JSON child allocation\n");
	else{
	       	child->Parent = parent;
		child->key = key;
	}

	return child;
}

JSONObject *parseJSON (FILE *file) {

	// load file to ram

	fseek(file, 0, SEEK_END);
	long int size = ftell(file);
	rewind(file);
	char *jsonFile = malloc(size + 1);
	fread(jsonFile, 1, size, file);
	jsonFile[size + 1] = '\0'; /* nul terminated string */
	printf("file is %ld bytes\nsize of jsonFile pointer: %ld\n", size, sizeof(jsonFile));
	if (jsonFile == NULL) {
		printf("failed to allocate memory for the file\n");
		return NULL; // handling errors TBD
	}

	long int fileptr = 0; /* what byte are we at in the file */
	
	extern JSONObject *root;
	InitRootObject();
	printf("%s\n", root->key);

	JSONObject *CurrentObject = root;
	int index = 0;
	int placeholder = 0;

	// while character is not nul, string end
	while(jsonFile[index] != '\0'){
		if (jsonFile[index] == '"'){
			printf("found start of key\n");
			index++; // move beyond the first "
			placeholder = index; 

			/* going to initiate an array of 5 and resize by +5 each time it would overflow */
			CurrentObject->ObjectData = realloc(CurrentObject->ObjectData, sizeof(CurrentObject->ObjectData) + sizeof(JSONKeyValue));

			skipTo('"', &index, jsonFile); // skip to " character

			printf("found end of key\n");

			JSONKeyValue *keyValue = malloc(sizeof(JSONKeyValue *));
			addKeyValuePair(keyValue);

			keyValue->key = malloc(index - placeholder + 1); /* nul terminated string */

			// fills a char *key with the string of a key to put later in a JSONKeyValue structur
			for (int j = 0; j < index - placeholder; j++)
				// stops at index - 1, index is '"'
				keyValue->key[j] = jsonFile[placeholder + j];

			keyValue->key[index - placeholder] = '\0';

			printf("got key: \"%s\"\n", keyValue->key);

			while (jsonFile[index] != ':') index++; // skip to ':'

			index++;
			printf("found colomn\n");

			while ((jsonFile[index] | 0b11110000) == 0 | jsonFile[index] == ' ') index++; // skip to ASCII printable which is not space

			printf("found start of value: %c (%08b)\n", jsonFile[index], jsonFile[index]);
			
			switch (jsonFile[index]){
				case '{':
					// child JSON object

					keyValue->value = spawnJSON(keyValue->key, CurrentObject); /* need a function to add one object at a time independent of current size */
					printf("Value is a JSON object\n");
					keyValue->type = JSONOBJECT;
					break;
				case '[':
					// array, can be any type
					printf("Value is a JSON array\n");
					keyValue->type = JSONARRAY;
					break;
				case '"':
					// string, standard key/value string
					printf("Value is a string\n");
					keyValue->type = JSONSTRING;

					index++; // move beyond the first "
					placeholder = index;
					while (jsonFile[index] != '"') index++;

					printf("found end of value\n");

					keyValue->value = malloc(index - placeholder + 1); /* size of the character value plus nul */

					// fills a char *Value with the string of a key to put later in a JSONKeyValue structur
					for (int j = 0; j < index - placeholder; j++)
						// stops at index - 1, index is '"'
						*((char *)keyValue->value + j) = jsonFile[placeholder + j];

					*((char *)keyValue->value + index - placeholder) = '\0';

					printf("value is string: \"%s\"\n", keyValue->value);
					break;
				case 'n':
					// null
					printf("Value is null\n");
					keyValue->type = JSONNULL;
					break;
				default:
					// number, need to identify int or float
					printf("Value is a number int, float or bool\n");
					keyValue->type = JSONNUMBER;
					break;
			}
			printf("\n");
		}
		index++;
	}

	return root; // return the root object with all data structured for use
}

void printJSONObject(JSONObject *object); /* to build */
