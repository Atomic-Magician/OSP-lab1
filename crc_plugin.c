// compile with:
// gcc -fPIC -shared crc_plugin.c -o crc_plugin.so

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

int strtocrc (const char * str, char * out_buff, size_t out_buff_len);
int get_crc (const char * file_path, char * out_buff, size_t out_buff_len);

int plugin_get_info(struct plugin_info* ppi) {
    if (ppi == NULL) {
        return -1;
    }

    ppi->plugin_name = "crc16-Modbus";
    ppi->sup_opts_len = 1;

    struct plugin_option *sup_opts = calloc(1, sizeof(struct plugin_option));

    sup_opts[0].opt.name = "crc16";
    sup_opts[0].opt.has_arg = required_argument;
    sup_opts[0].opt_descr = "Выполняет поиск файлов, контрольная сумма которых,"
    " рассчитанная с помощью CRC-16-MODBUS, равна заданному значению."
    " Значение суммы задается строкой, содержащей запись числа либо в двоичной (0b...)"
    ", либо в десятичной, либо шестнадцатеричной (0x...) системах.";

    ppi->sup_opts = sup_opts;

    return 0;
}

int plugin_process_file(const char *fname,
                struct option *in_opts[],
                size_t in_opts_len,
                char *out_buff,
                size_t out_buff_len) {

    int file_crc, arg_crc;

    if (in_opts_len != 1) {
        if (snprintf(out_buff, out_buff_len, "To many options\n") < 0) return -20;
        return -1;
    }

    arg_crc = strtocrc((const char *)in_opts[0] -> flag, out_buff, out_buff_len);
    if (arg_crc < 0) {
        //if (snprintf(out_buff, out_buff_len, "Above is the error code(%d): from strtocrc\n", arg_crc) < 0) return -20;
        return arg_crc;
    }

    file_crc =  get_crc(fname, out_buff, out_buff_len);
    if (file_crc < 0) {
        //if (snprintf(out_buff, out_buff_len, "Above is the error code(%d): from get_crc\n", file_crc) < 0) return -20;
        return file_crc;
    }

    //printf("%d\n", (file_crc == arg_crc) ? 0 : 1);
    return (file_crc == arg_crc) ? 0 : 1;

    return 0;
}
