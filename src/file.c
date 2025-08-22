#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file.h"
#include "common.h"

#define DEFPERMS 0644

// Create database file <filename>
// return file descriptor upon success
// return STATUS_ERROR upon failure
int create_db_file(char *filename) {
	// attempt to open the file read only to determine if it exits
	// if it does exist, close the file descriptor and notify user
	// before exiting.
	int fd = open(filename, O_RDONLY);
	if(fd != -1) {
		close(fd);
		printf("%s already exists.\n", filename);
		return STATUS_ERROR;
	}

	// the file doesn't exist yet
	// create a file of name <filename> and return the read/write file descriptor
	// for further use in the program
	fd = open(filename, O_RDWR | O_CREAT, DEFPERMS);
	if(fd == -1) {
		perror("open");
		return STATUS_ERROR;
	}

	return fd;
}

// Open database file <filename>
// return file descriptor upon success
// return STATUS_ERROR upon failure
int open_db_file(char *filename) {
	int fd = open(filename, O_RDWR);
	if(fd == -1) {
		perror("open");
		return STATUS_ERROR;
	}

	return fd;
}
