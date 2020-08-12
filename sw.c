#define VERSION_PRIMARY 0
#define VERSION_SECONDARY 1

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

// Error codes
enum {
	ERROR_QUIT = 1,	// User prompted to quit, operation not finished
	ERROR_NO_PAIR,	// Some filename doesn't have a pair
	ERROR_NO_FILE,	// Specified filename does not exist
	ERROR_BUMP		// Bumped into a file that should not exist
};




void PrintHelp(void) {
	printf(
		"sw - swap filenames\n"
		"Usage: sw [FILENAMES A] [FILENAMES B]\n"
		"Examples:\n"
		"    sw file1 file2\n"
		"        will swap file1 and file2\n"
		"    sw file1 file2 file3 file4\n"
		"        will swap file1 and file3\n"
		"        then swap file2 and file4\n"
		"Exceptions are --help and --version arguments.\n"
	);
}




void HandleSignals(int Signal) {
	// It's a switch in case we need more signals later
	switch (Signal) {
		case SIGINT:
			printf(" - Exiting.\n");
			exit(ERROR_QUIT);
	}
}




int main(int ArgN, char *Args[]) {
	// Just in case
	signal(SIGINT, HandleSignals);

	// If there are no arguments - show help
	if (ArgN == 1) PrintHelp();

	// Check for --help and --version
	for (int I = 0; I < ArgN; I++) {
		if (strcmp(Args[I], "--help") == 0) {
			PrintHelp();
			return 0;
		}
		if (strcmp(Args[I], "--version") == 0) {
			printf(
				"sw version %i.%i\n",
				VERSION_PRIMARY, VERSION_SECONDARY
			);
			return 0;
		}
	}

	// The actual program

	// First, we check if every file has a pair
	// Evens fail because the program's name is also an argument
	if ((ArgN & 1) == 0) {
		fprintf(stderr, "Error: every filename requires a pair.\n");
		return ERROR_NO_PAIR;
	}

	// Check that every filename exists
	for (int I = 1; I < ArgN; I++) {
		if (access(Args[I], F_OK) == -1) {
			fprintf(stderr, "Error: filename \"%s\" does not exist.\n", Args[I]);
			return ERROR_NO_FILE;
		}
	}

	// Useful constants
	const int Filenames = ArgN - 1;
	const int Operations = Filenames / 2;

	// Fill the character lists
	char *List[3][Filenames]; // 0=from 1=to 2=temp
	for (int I = 0; I < Operations; I++) {
		List[0][I] = Args[I+1];
	}
	for (int I = Operations; I < Filenames; I++) {
		List[1][I - Operations] = Args[I+1];
	}
	for (int I = 0; I < Operations; I++) {
		List[2][I] = NULL;	// So free() doesn't UB
	}

	// Move to temporary filenames
	for (int I = 0; I < Operations; I++) {
		List[2][I] = calloc(256, sizeof(char));
		strcpy(List[2][I], List[0][I]);
		strcat(List[2][I], "_swtmp");
		if (access(List[2][I], F_OK) != -1) {
			fprintf(stderr, "Error: bumped into file \"%s\" that shouldn't exist.\n", List[2][I]);
			for (int I = 0; I < Operations; I++) {
				free(List[2][I]);
			}
			return ERROR_BUMP;
		}
		rename(List[0][I], List[2][I]);
	}

	// Move to designated filenames
	for (int I = 0; I < Operations; I++) {
		rename(List[2][I], List[1][I]);
	}

	// Freeing the memory before exit
	for (int I = 0; I < Operations; I++) {
		free(List[2][I]);
	}

	return 0;
}
