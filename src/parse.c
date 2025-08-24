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

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {

}

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring) {

	if(dbhdr == NULL) {
		printf("Invalid database header.\n");
		return STATUS_ERROR;
	}
	
	if(employees == NULL) {
		printf("Invalid employees list.\n");
		return STATUS_ERROR;
	}

	if(addstring == NULL) {
		printf("Invalid new uesr info.\n");
		return STATUS_ERROR;
	}

	int count = dbhdr->count;
	char *name = strtok(addstring, ",");
	if(name == NULL) {
		printf("Incorrectly formatted addstring.\n");
		return STATUS_ERROR;
	}

	char *address = strtok(NULL, ",");
	if(address == NULL) {
		printf("Incorrectly formatted addstring.\n");
		return STATUS_ERROR;
	}

	char *hours = strtok(NULL, ",");
	if(hours == NULL) {
		printf("Incorrectly formatted addstring.\n");
		return STATUS_ERROR;
	}

	dbhdr->count++;
	struct employee_t *tmpemp = realloc(*employees, (dbhdr->count * sizeof(struct employee_t)));
	if(tmpemp == NULL) {
		perror("realloc()");
		return STATUS_ERROR;
	}

	strncpy(tmpemp[dbhdr->count - 1].name, name, sizeof(tmpemp[dbhdr->count - 1].name));
	strncpy(tmpemp[dbhdr->count - 1].address, address, sizeof(tmpemp[dbhdr->count - 1].address));
	tmpemp[dbhdr->count - 1].hours = atoi(hours);
	
	*employees = tmpemp;

	return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
	if(fd < 0) {
		printf("Invalid file descriptor.\n");
		return STATUS_ERROR;
	}

	if(dbhdr == NULL) {
		printf("Invalid database header.\n");
		return STATUS_ERROR;
	}

	if(employeesOut == NULL) {
		printf("Invalid employees list.\n");
		return STATUS_ERROR;
	}

	int count = dbhdr->count;

	struct employee_t *employees = calloc(count, sizeof(struct employee_t));
	if(employees == STATUS_ERROR) {
		printf("Malloc failed.\n");
		return STATUS_ERROR;
	}

	if(read(fd, employees, count * sizeof(struct employee_t)) == STATUS_ERROR) {
		perror("read_employees");
		return STATUS_ERROR;
	}

	int i = 0;
	for(i = 0; i < count; i++) {
		employees[i].hours = ntohs(employees[i].hours);
	}

	*employeesOut = employees;
	return STATUS_SUCCESS;
}

// write to the specified database file
int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
	if(dbhdr == NULL) {
		printf("Invalid database header.\n");
		return STATUS_ERROR;
	}

	if(employees == NULL) {
		printf("list of employees is invalid.\n");
		return STATUS_ERROR;
	}

	if(fd < 0) {
		printf("Bad file descriptor provided.\n");
		return STATUS_ERROR;
	}

	int realCount = dbhdr->count;
	int filesize = (sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realCount));

	dbhdr->magic = htonl(dbhdr->magic);
	dbhdr->filesize = htonl(filesize);
	dbhdr->count = htons(dbhdr->count);
	dbhdr->version = htons(dbhdr->version);

	lseek(fd, 0, SEEK_SET);
	
	write(fd, dbhdr, sizeof(struct dbheader_t));

	int i = 0;
	for(i = 0; i < realCount; i++) {
		employees[i].hours = htonl(employees[i].hours);
		write(fd, &employees[i], sizeof(struct employee_t));
	}

	return STATUS_SUCCESS;
}	

// validate the database header to ensure the database file is 
// the correct version and type of database (HEADER_MAGIC).
// also check that the recorded filesize matches the file size on disk.
int validate_db_header(int fd, struct dbheader_t **headerOut) {
	if(headerOut == NULL) {
		printf("Invalid database header.\n");
		return STATUS_ERROR;
	}

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

	*headerOut = header;

	return STATUS_SUCCESS;
}

// creates a default database header in file associated with
// file descriptor fd
int create_db_header(struct dbheader_t **headerOut) {
	if(headerOut == NULL) {
		printf("Invalid database header given.\n");
		return STATUS_ERROR;
	}

	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if(header == STATUS_ERROR) {
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
