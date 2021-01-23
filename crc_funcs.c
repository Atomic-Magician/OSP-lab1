
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "plugin_api.h"

#define POL 0xA001


int strtocrc (const char * str, char * out_buff, size_t out_buff_len) {
    int base, offset;
    char *endptr;
    long val;

    offset = 0;
    base = 10;

    if (str == NULL) {
        if (snprintf(out_buff, out_buff_len, "Option argument is NULL\n") < 0) return -20;
        return -10;
    }

    if (strlen(str) > 2) {
        if (str[0] == '0' && str[1] == 'b') {
            base = 2;
            offset = 2;
        }
        else {
            if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
                base = 16;
                offset = 2;
            }
        }
    }
    else {
        if (snprintf(out_buff, out_buff_len, "Incorrect argument. No digits were found\n") < 0) return -20;
        return -12;
    }
    //printf("%d\n", base);
    //printf("%s\n", str + offset);

    errno = 0;    /* To distinguish success/failure after call */
    val = strtol(str + offset, &endptr, base);
    /* Check for various possible errors */

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
            || (errno != 0 && val == 0)) {
        perror("strtol");
        return -11;
    }

    if (endptr == str) {
        if (snprintf(out_buff, out_buff_len, "Incorrect argument. No digits were found\n") < 0) return -20;
        return -12;
    }

    /* If we got here, strtol() successfully parsed a number */

    //printf("strtol() returned %ld\n", val);

    if (*endptr != '\0') {
        if (snprintf(out_buff, out_buff_len, "Incorrect argument.\n") < 0) return -20;
        return -13;
    }

    return val;
}

int get_crc (const char * file_path, char * out_buff, size_t out_buff_len){
    int fd, page_size;
    unsigned short res;
    unsigned char *ptr;
    struct stat file_stat;
    off_t file_size;

    if (file_path == NULL) {
        if (snprintf(out_buff, out_buff_len, "Path to file is NULL\n") < 0) return -20;
        return -1;
    }

    fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        if (snprintf(out_buff, out_buff_len, "Error: can't open %s\n", file_path) < 0) return -20;
        return -2;
    }
    if (fstat(fd, &file_stat) != 0)
	{
		if (snprintf(out_buff, out_buff_len, "Error: can't get stat\n") < 0) return -20;
		return -3;
	}

    file_size = file_stat.st_size;
    //printf("file size %ld\n", file_size);
    page_size = getpagesize();
    //printf("page size %d\n", page_size);

    res = 0xFFFF;

    for (int i = 0; i < file_size / page_size; i++)
	{
		ptr = mmap(NULL, page_size, PROT_READ, MAP_PRIVATE, fd, i * page_size);
		if (ptr == MAP_FAILED)
		{
			if (snprintf(out_buff, out_buff_len, "mmap-1 failed at i:%d\n", i) < 0) return -20;
			return -4;
		}
		for (int j = 0; j < page_size; j++)
		{
            res ^= (unsigned int)ptr[j];

            for (int k = 8; k != 0; k--) {
                if ((res & 0x0001) != 0) {
                    res >>= 1;
                    res ^= POL;
                }
                else
                res >>= 1;
            }
		}
		if (munmap(ptr, page_size) != 0)
		{
			if (snprintf(out_buff, out_buff_len, "munmap-1 failed at i:%d\n", i) < 0) return -20;
			return -5;
		}
	}
    if (file_size % page_size != 0)
    {
		ptr = mmap(NULL, page_size, PROT_READ, MAP_PRIVATE, fd, file_size / page_size * page_size);
		if (ptr == MAP_FAILED)
		{
			if (snprintf(out_buff, out_buff_len, "mmap-2 failed\n") < 0) return -20;
			return -6;
		}
		for (int j = 0; j < file_size % page_size; j++)
		{
            res ^= (unsigned int)ptr[j];

            for (int k = 8; k != 0; k--) {
                if ((res & 0x0001) != 0) {
                    res >>= 1;
                    res ^= POL;
                }
                else
                res >>= 1;
            }
		}
		if (munmap(ptr, page_size) != 0)
		{
			if (snprintf(out_buff, out_buff_len, "munmap-2 failed\n") < 0) return -20;
			return -7;
		}
	}
    //printf("result = %X\n", res);
    close(fd);
    return res;
}
