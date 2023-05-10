#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void* string_manipulation(void* arg) {
	char* input = (char*)arg;
	size_t input_length = strlen(input);

	//FOUND memory needs to be allocated so its lifetime exceeds the function
    //There also needs to be extra space for the \0
	//char reversed_input[input_length];
	char* reversed_input = malloc((input_length + 1) * sizeof(char));
	if(reversed_input == NULL) {
		fprintf(stderr, "Could not allocate memory.\n");
		return NULL;
	}
	for (size_t char_index = 0; char_index < input_length; ++char_index) {
		reversed_input[input_length - char_index - 1] = input[char_index];
	}
    //adding the zero termination
    reversed_input[input_length] = '\0';

	for (size_t char_index = 0; char_index < input_length; ++char_index) {
		input[char_index] = toupper(input[char_index]);
	}

	return reversed_input;
}

int main(int argc, const char** argv) {
	char* shared_strings[argc];

	for (int arg_index = 0; arg_index < argc; ++arg_index) {
		//FOUND strlen does not count the zero termination -> space for one additional char needed
		//size_t arg_length = strlen(argv[arg_index]);
		size_t arg_length = strlen(argv[arg_index]) + 1;
		shared_strings[arg_index] = malloc(arg_length * sizeof(char));
		if (shared_strings[arg_index] == NULL) {
			fprintf(stderr, "Could not allocate memory.\n");
			exit(EXIT_FAILURE);
		}
		strcpy(shared_strings[arg_index], argv[arg_index]);
	}

	pthread_t t_ids[argc];
	//FOUND out of array bounds when using <=, use < instead
	//for (int arg_index = 0; arg_index <= argc; arg_index++) {
	for (int arg_index = 0; arg_index < argc; arg_index++) {
		pthread_create(&t_ids[arg_index], NULL, string_manipulation, shared_strings[arg_index]);
        //FOUND shared_strings is used later on and must not be freed yet
		//free(shared_strings[arg_index]);
	}

    //FOUND <= will go out of array bounds; use < instead
    //for (int arg_index = 0; arg_index <= argc; arg_index++) {
	for (int arg_index = 0; arg_index < argc; arg_index++) {
		char* reversed_string;
		//FOUND waitpid() is used for child processes, use pthread_join instead
        //FOUND * has higher priority than +, therefore brackets are required
        //waitpid(*t_ids + arg_index, (void*)&reversed_string, 0);
		pthread_join(*(t_ids + arg_index), (void*)&reversed_string);
		//error handling if memory allocation in thread was unsuccessful
		if (reversed_string == NULL) {
			exit(EXIT_FAILURE);
		}
		printf("original argv[%d]: %s\nuppercased: %s\nreversed: %s\n", arg_index, argv[arg_index],
		       shared_strings[arg_index], reversed_string);
        //now shared_strings and reversed_string may be freed
        free(shared_strings[arg_index]);
		free(reversed_string);
	}

	return EXIT_SUCCESS;
}