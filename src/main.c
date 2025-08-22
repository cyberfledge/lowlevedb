#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

// Print usage message to the user
void print_usage(char *argv[]) {
	printf("Usage: %s -f <filename> -n\n", argv[0]);
	printf("\t-n - create new database file.\n");
	printf("\t-f - (required) path of database file.\n");
	return;
}

// main program logic
int main(int argc, char *argv[]) { 
	char *filepath = NULL;
	bool newfile = false;
	int c;

	struct dbheader_t *dbhdr;
	int dbfd = -1;

	// parse the user supplied command line arguments
	while ((c = getopt(argc, argv, "nf:")) != -1) {
		switch (c) {
			case 'n':
				newfile = true;
				break;
			case 'f':
				filepath = optarg;
				break;
			case '?':
				printf("Unknown option -%c\n", c);
				break;
			default:
				return -1;
		}
	}

	// if filepath is still NULL after parsing the commandline arguments
	// then the user didn't provide one. exit program.
	if(filepath == NULL) {
		printf("Filepath is required.\n");
		print_usage(argv);
		return STATUS_ERROR;
	}

	// user has supplied a filepath and wants a new database file created.
	if(newfile) {
		dbfd = create_db_file(filepath);
		if(dbfd == STATUS_ERROR) {
			printf("Unable to create database file.\n");
			return STATUS_ERROR;
		}

		if(create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
			printf("Unable to create database header.\n");
			return STATUS_ERROR;
		}
	} else { // user wants to open an existing database file.
		dbfd = open_db_file(filepath);
		if(dbfd == STATUS_ERROR) {
			printf("Unable to open database file.\n");
			return STATUS_ERROR;
		}

		if(validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
			printf("Invalid database file header.\n");
			return STATUS_ERROR;
		}
	}

	output_file(dbfd, dbhdr);

	// clouse the database file
	close(dbfd);
	return STATUS_SUCCESS;
}
