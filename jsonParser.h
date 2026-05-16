// an attempt to parse AllPrintings.json from mtgjson.com
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PTR_SIZE sizeof(int *)

#define JSON_STRING 0
#define JSON_NUMBER 1
#define JSON_OBJECT 2
#define JSON_ARRAY 3
#define JSON_BOOLEAN 4
#define JSON_NULL 5

/* a JSON value has 6 possible data types:
 * 	a string
 * 	a number
 * 	an object
 * 	an array
 * 	a boolean
 * 	NULL
 *
*/

typedef struct JSONObject JSONObject;

typedef struct JSONKeyValue {
	char *key;

	uint8_t valueType;
	union{
		char *JSONString;
		long JSONInt;
		double JSONFloat;
		JSONObject *JSONObject;
		char **JSONArray;
		bool *JSONBool;
	}value;
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
JSONObject *root;

void InitRootObject(){
	extern JSONObject *root;
	root = malloc(PTR_SIZE);
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
	if (keyValuePairArray == NULL) keyValuePairArray = malloc(PTR_SIZE);
	else {
	       keyValuePairArray = realloc(keyValuePairArray, sizeof(keyValuePairArray + PTR_SIZE));
	       if (keyValuePairArray == NULL) {
		       printf("Failed to reallocate key/value pair array\n\n");
		       return;
	       }
	}
}

JSONObject *spawnJSON(char* key, JSONObject *parent) {
	JSONObject *child = malloc(PTR_SIZE);

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
			CurrentObject->ObjectData = realloc(CurrentObject->ObjectData, sizeof(CurrentObject->ObjectData) + PTR_SIZE);

			skipTo('"', &index, jsonFile); // skip to " character

			printf("found end of key\n");

			JSONKeyValue *keyValue = malloc(PTR_SIZE);
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

					keyValue->value.JSONObject = spawnJSON(keyValue->key, CurrentObject); /* need a function to add one object at a time independent of current size */
					printf("Value is a JSON object\n");
					keyValue->valueType = JSON_OBJECT;
					break;
				case '[':
					// array, can be any type
					printf("Value is a JSON array\n");
					keyValue->valueType = JSON_ARRAY;
					break;
				case '"':
					// string, standard key/value string
					printf("Value is a string\n");
					keyValue->valueType = JSON_STRING;

					index++; // move beyond the first "
					placeholder = index;
					while (jsonFile[index] != '"') index++;

					printf("found end of value\n");

					keyValue->value.JSONString = malloc(index - placeholder + 1); /* size of the character value plus nul */

					// fills a char *Value with the string of a key to put later in a JSONKeyValue structur
					for (int j = 0; j < index - placeholder; j++)
						// stops at index - 1, index is '"'
						keyValue->value.JSONString[j] = jsonFile[placeholder + j];

					keyValue->value.JSONString[index - placeholder] = '\0';

					printf("value is string: \"%s\"\n", keyValue->value.JSONString);
					break;
				case 'n':
					// null
					printf("Value is null\n");
					keyValue->valueType = JSON_NULL;
					break;
				default:
					// number, need to identify int or float
					printf("Value is a number int, float or bool\n");
					keyValue->valueType = JSON_NUMBER;
					break;
			}
			printf("\n");
		}
		index++;
	}

	return root; // return the root object with all data structured for use
}

void printJSONObject(JSONObject *object); /* to build */
