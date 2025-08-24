#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

//void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
//
//}

//int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
//
//}

//int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
//
//}

// write to the specified database file
int output_file(int fd, struct dbheader_t *dbhdr) { //, struct employee_t *employees) {
	if(fd < 0) {
		printf("Bad file descriptor provided.\n");
		return STATUS_ERROR;
	}

	dbhdr->magic = htonl(dbhdr->magic);
	dbhdr->filesize = htonl(dbhdr->filesize);
	dbhdr->count = htons(dbhdr->count);
	dbhdr->version = htons(dbhdr->version);

	lseek(fd, 0, SEEK_SET);
	
	write(fd, dbhdr, sizeof(struct dbheader_t));

	return STATUS_SUCCESS;
}	

// validate the database header to ensure the database file is 
// the correct version and type of database (HEADER_MAGIC).
// also check that the recorded filesize matches the file size on disk.
int validate_db_header(int fd) {
	if(fd < 0) {
		printf("Bad file descriptor given.\n");
		return STATUS_ERROR;
	}

	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if(header == STATUS_ERROR) {
		printf("Calloc failed to create database header.\n");
		return STATUS_ERROR;
	}

	if(read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
		perror("read");
		free(header);
		return STATUS_ERROR;
	}

	header->version = ntohs(header->version);
	header->count = ntohs(header->count);
	header->magic = ntohl(header->magic);
	header->filesize = ntohl(header->filesize);
	
	if(header->version != 1) {
		printf("Wrong header version.\n");
		free(header);
		return STATUS_ERROR;
	}
	if(header->magic != HEADER_MAGIC) {
		printf("Wrong header magic.\n");
		free(header);
		return STATUS_ERROR;
	}

	struct stat dbstat = {0};
	fstat(fd, &dbstat);
	if(header->filesize != dbstat.st_size) {
		printf("Database file corrupted.\n");
		free(header);
		return STATUS_ERROR;
	}

	return STATUS_SUCCESS;
}

// creates a default database header in file associated with
// file descriptor fd
int create_db_header(int fd, struct dbheader_t **headerOut) {
	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if(header == -1) {
		printf("Calloc failed to create database header.\n");
		return STATUS_ERROR;
	}

	header->version = 1;
	header->count = 0;
	header->magic = HEADER_MAGIC;
	header->filesize = sizeof(struct dbheader_t);

	*headerOut = header;

	return STATUS_SUCCESS;
}
