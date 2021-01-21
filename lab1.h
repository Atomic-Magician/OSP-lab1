#ifndef LAB1_H
#define LAB1_H

#include "plugin_api.h"


#define OPTS_COUNT 6 //количество опций по умолчанию
#define STR_LEN 512

struct opt_flag {
    unsigned char P;
    unsigned char l;
    unsigned char N;
    unsigned char h;
    unsigned char v;
    unsigned char C;
    unsigned char long_opt;
    unsigned char wrong_opt;
};

struct plugin {
    char * p_path;
    void * handle;
    struct plugin_info info;
    int (*plugin_process_file)(const char *fname,
            struct option *in_opts[],
            size_t in_opts_len,
            char *out_buff,
            size_t out_buff_len);
};

char * version;
char * search_dir;
char * plugin_dir;
char * log_path;
unsigned char cond;
int long_opts_count;
int plugin_count;
int * plugin_id_array;
struct option * long_opts;
struct plugin ** plugin_list;
struct opt_flag flags;

void * quickcpy(char *src);
int add_to_long_opts(const char * name, int has_arg, int * flag, int val);
int search_for_plugins(char * path);
int attach_plugin(char * plugin_path);
int make_logfile(char * log_path);
void show_help(void);
int search_for_files(char * path);
int check_file(char * path);
int free_all(void);

#endif
