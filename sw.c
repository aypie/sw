/*
	sw - swap filenames
	Copyright (C) 2020 Oleksii Honcharov
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

	Written by Oleksii Honcharov (a.k.a. Alex Potterson)
*/

#define PROGRAM_NAME "sw"
#define VERSION_PRIMARY 1
#define VERSION_SECONDARY 0

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define COLOR_RED "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_WHITE "\x1b[37m"

// Error codes
enum {
	ERROR_QUIT = 1,	// User prompted to quit, operation not finished
	ERROR_NO_ARGS,	// No arguments were passed
	ERROR_NO_PAIR,	// Some filename doesn't have a pair
	ERROR_NO_FILE,	// Specified filename does not exist
	ERROR_BUMP		// Bumped into a file that should not exist
};




void PrintHelp(void) {
	printf(
		COLOR_YELLOW "sw - swap filenames\n" COLOR_WHITE
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
			printf(
				COLOR_YELLOW
				" - Exiting.\n"
				COLOR_WHITE
			);
			exit(ERROR_QUIT);
	}
}




int main(int ArgN, char *Args[]) {
	// Just in case
	signal(SIGINT, HandleSignals);

	// If there are no arguments - show help
	if (ArgN < 2) {
		PrintHelp();
		return ERROR_NO_ARGS;
	}

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


	// First, we check if every file has a pair
	// Evens fail because the program's name is also an argument
	if ((ArgN & 1) == 0) {
		fprintf(
			stderr,
			COLOR_RED
			"sw: error: every filename requires a pair.\n"
			COLOR_WHITE
		);
		return ERROR_NO_PAIR;
	}

	// Check that every filename exists
	for (int I = 1; I < ArgN; I++)
		if (access(Args[I], F_OK) == -1) {
			fprintf(
				stderr,
				COLOR_RED
				"sw: error: filename \"%s\" does not exist.\n"
				COLOR_WHITE,
				Args[I]
			);
			return ERROR_NO_FILE;
		}

	/*
		The Actual Program
	*/

	const int Filenames = ArgN - 1;
	const int Operations = Filenames / 2;
	char *List[3][Filenames]; // Names list, 0=from 1=to 2=temp

	// Fill the character lists

	for (int I = 0; I < Operations; I++)
		List[0][I] = Args[I+1];

	for (int I = Operations; I < Filenames; I++)
		List[1][I - Operations] = Args[I+1];

	for (int I = 0; I < Operations; I++)
		List[2][I] = NULL;	// So free() doesn't UB

	// Create temporary filenames
	for (int I = 0; I < Operations; I++) {
		List[2][I] = calloc(256, sizeof(char));
		strcpy(List[2][I], List[0][I]);
		strcat(List[2][I], "_swtmp");
		if (access(List[2][I], F_OK) != -1) {
			fprintf(
				stderr,
				COLOR_RED
				"sw: error: bumped into an unexpected file \"%s\".\n"
				COLOR_WHITE
				"No changes were made.\n",
				List[2][I]
			);
			for (int I = 0; I < Operations; I++) {
				free(List[2][I]);
			}
			return ERROR_BUMP;
		}
	}

	// Move A to C
	for (int I = 0; I < Operations; I++)
		rename(List[0][I], List[2][I]);

	// Move B to A
	for (int I = 0; I < Operations; I++)
		rename(List[1][I], List[0][I]);

	// Move C to B
	for (int I = 0; I < Operations; I++)
		rename(List[2][I], List[1][I]);

	// Freeing the memory before exit
	for (int I = 0; I < Operations; I++)
		free(List[2][I]);

	return 0;
}
