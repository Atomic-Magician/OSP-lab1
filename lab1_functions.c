#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "lab1.h"
#include "plugin_api.h"




int add_to_long_opts(const char * name, int has_arg, int * flag, int val) {

    int * id_array;
    struct option * new_ptr;

    if (long_opts_count < OPTS_COUNT) {
        long_opts[long_opts_count].name = name;
        long_opts[long_opts_count].has_arg = has_arg;
        long_opts[long_opts_count].flag = flag;
        long_opts[long_opts_count].val = val;
        long_opts[long_opts_count + 1].name = NULL;
        long_opts[long_opts_count + 1].has_arg = no_argument;
        long_opts[long_opts_count + 1].flag = NULL;
        long_opts[long_opts_count + 1].val = 0;
    }
    else {
        //printf("what\n");
        new_ptr = (struct option *)realloc((void *)long_opts, (long_opts_count + 2) * sizeof(struct option));
        //printf("what\n");
        if (new_ptr != NULL) {
            long_opts = new_ptr;

            long_opts[long_opts_count].name = name;
            long_opts[long_opts_count].has_arg = has_arg;
            long_opts[long_opts_count].flag = flag;
            long_opts[long_opts_count].val = val;
            long_opts[long_opts_count + 1].name = NULL;
            long_opts[long_opts_count + 1].has_arg = no_argument;
            long_opts[long_opts_count + 1].flag = NULL;
            long_opts[long_opts_count + 1].val = 0;

            id_array = (int *)realloc(plugin_id_array ,(long_opts_count - OPTS_COUNT + 1) * sizeof(int));
            if (id_array != NULL) {
                plugin_id_array = id_array;
                plugin_id_array[long_opts_count - OPTS_COUNT] = plugin_count - 1;
            }
            else {
                fprintf(stderr, "Can't allocate memory for plugin_id_array\n");
                return -1;
            }
        }
        else {
            fprintf(stderr, "Ошибка: не выделено места под новую опцию\n");
            return -1;
        }
    }
    //printf("%s    %d\n", long_opts[long_opts_count].name, long_opts_count);
    long_opts_count++;
    return 0;
}

int search_for_plugins (char * pt) {
    //int plugin_count = 0;
    struct dirent *fi;
    struct stat stt;
    char path[STR_LEN];
    char file_path[STR_LEN];
    DIR * plugin_dir;

    strcpy(path, pt);
    if(path[strlen(path) - 1] != '/') strcat(path, "/");
    plugin_dir = opendir(path);
    if(!plugin_dir){
        //printf("Ошибка открытия %s\n", path);
        //printf("Incorrect or missing path to plugins\n");
        return -1;
    }
    printf("Searching for plugins in '%s' ...\n", path);
    while ((fi = readdir(plugin_dir))) {
        strcpy(file_path, path);
        strcat(file_path, fi->d_name);
        lstat(file_path, &stt);
        if(S_ISREG(stt.st_mode)) {
            //printf("File: %s\n", file_path);
            if (strstr(fi->d_name, ".so")) {
                //printf("Looks like plugin: %s\n", fi->d_name);
                !attach_plugin(file_path);
            }
        }
    }
    closedir(plugin_dir);
    return 0;
    //return plugin_count;
}

int attach_plugin (char * plugin_path) {
    void *handle;
    char *error;
    struct option * new_opt_ptr;
    struct plugin ** new_ptr;
    int (*plugin_get_info)(struct plugin_info*);
    int (*plugin_process_file)(const char *fname,
            struct option *in_opts[],
            size_t in_opts_len,
            char *out_buff,
            size_t out_buff_len);

    handle = dlopen(plugin_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    dlerror();

    plugin_get_info = (int (*)(struct plugin_info*))dlsym(handle, "plugin_get_info");
    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        fprintf(stderr, "%s has NOT 'plugin_get_info' function\n", plugin_path);
        dlclose(handle);
        return -1;
        //exit(EXIT_FAILURE);
    }

    plugin_process_file = (int (*)(const char *fname,
            struct option *in_opts[],
            size_t in_opts_len,
            char *out_buff,
            size_t out_buff_len))dlsym(handle, "plugin_process_file");
    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        fprintf(stderr, "%s has NOT 'plugin_process_file' function\n", plugin_path);
        dlclose(handle);
        return -1;
        //exit(EXIT_FAILURE);
    }

    plugin_count++;
    new_ptr = (struct plugin**)realloc(plugin_list ,plugin_count * sizeof(struct plugin*));
    if (new_ptr == NULL) {
        fprintf(stderr, "Can't allocate memory for plugin_list\n");
        dlclose(handle);
        return -1;
    }

    plugin_list = new_ptr;
    plugin_list[plugin_count - 1] = (struct plugin*)malloc(sizeof(struct plugin));
    if (plugin_list[plugin_count - 1] == NULL) {
        fprintf(stderr, "Can't allocate memory for element in plugin_list\n");
        dlclose(handle);
        return -1;
    }

    if (!plugin_get_info(&plugin_list[plugin_count - 1] -> info)) {
        //printf("Got %s info\n", plugin_path);
    }
    else
        fprintf(stderr, "Can't get %s info\n", plugin_path);

    plugin_list[plugin_count - 1] -> p_path = strdup(plugin_path);
    plugin_list[plugin_count - 1] -> handle = handle;
    plugin_list[plugin_count - 1] -> plugin_process_file = plugin_process_file;


    for (int i = 0; i < plugin_list[plugin_count - 1] -> info.sup_opts_len; i++) {
        new_opt_ptr = &plugin_list[plugin_count - 1] -> info.sup_opts[i].opt;
        add_to_long_opts(new_opt_ptr -> name, new_opt_ptr -> has_arg, new_opt_ptr -> flag, new_opt_ptr -> val);
    }

    fprintf(stdout, "Plagin '%s' is useable\n", plugin_list[plugin_count - 1] -> p_path);

    return 0;
}

void show_help(void) {
    //int plug_id;
    //char * opt_name;
    //char * plugin_name;
    //char * opt_descr;
    char * help =   " -P dir            - каталог с плагинами. Значение по умолчанию -  «.» (текущий каталог)\n"
                    " -l /path/to/log   - путь к лог-файлу\n"
                    " -C cond           - условие объединения опций. возможные значения AND, OR (без учёта регистра). значение по умолчанию - AND\n"
                    " -N                - инвертирование условий поиска\n"
                    " -v                - вывод версии программы\n"
                    " -h                - вывод справки по опциям\n";


    printf("\n===== HELP =====\n");
    printf("%s", help);
    for (int i = 0; i < plugin_count; i++) {
        printf(" Плагин: %s ------------\n", plugin_list[i] -> info.plugin_name);
        for (int j = 0; j < plugin_list[i] -> info.sup_opts_len; j++) {
            printf(" --%s\t- %s\n", plugin_list[i] -> info.sup_opts[j].opt.name, plugin_list[i] -> info.sup_opts[j].opt_descr);
        }
    }
    printf("================\n\n");

}

int search_for_files(char * pt) {
    int count, res;
    struct dirent *fi;
    struct stat stt;
    char path[STR_LEN];
    char file_path[STR_LEN];
    DIR * file_dir;

    count = 0;
    strcpy(path, pt);
    if(path[strlen(path) - 1] != '/') strcat(path, "/");
    file_dir = opendir(path);
    if(!file_dir){
        return -1;
    }
    //printf("Searching for files in '%s' ...\n", path);
    while ((fi = readdir(file_dir))) {
        if(!strcmp(fi->d_name, ".") || !strcmp(fi->d_name, "..")) continue;
        strcpy(file_path, path);
        strcat(file_path, fi->d_name);
        lstat(file_path, &stt);
        if (S_ISDIR(stt.st_mode)) {
            count += search_for_files(file_path);
        }
        else {
            if (S_ISREG(stt.st_mode)) {
                //printf("%s - %d\n", file_path, check_file(file_path));
                res = check_file(file_path);
                if (!res && !flags.N || res && flags.N) {
                    printf("File: %s\n", file_path);
                    count++;
                }
            }
        }
    }
    closedir(file_dir);
    return count;
}

int check_file(char * pt) {
    int old_plug_id, plug_id, in_opts_count, rez, func_count, rez_count;
    char path[STR_LEN];
    char out_buff[STR_LEN];
    struct option * in_opts;
    int (*process_file)(const char *fname,
            struct option *in_opts[],
            size_t in_opts_len,
            char *out_buff,
            size_t out_buff_len);

    strcpy(path, pt);
    old_plug_id = -1;
    in_opts_count = 1;
    in_opts = NULL;
    rez = -1;
    rez_count = func_count = 0;;
    for (int i = OPTS_COUNT; i < long_opts_count; i++)  {
        if (long_opts[i].flag != NULL || long_opts[i].flag == NULL && long_opts[i].has_arg == no_argument){
            plug_id = plugin_id_array[i - OPTS_COUNT];
            if (old_plug_id != plug_id) {
                if (old_plug_id != -1) {
                    func_count++;
                    rez = process_file(path, &in_opts, (size_t)in_opts_count, out_buff, STR_LEN);
                    if(rez < 0)
                        fprintf(stderr, "Error: process_file\n");
                    else
                        if (rez == 0)
                            rez_count++;

                }
                in_opts_count = 1;
                //printf("before process_file\n");
                process_file = plugin_list[plug_id] -> plugin_process_file;
                //printf("after process_file\n");
            }
            else
                in_opts_count++;
            in_opts = (struct option *)realloc(in_opts ,in_opts_count * sizeof(struct option));
            if (in_opts == NULL) {
                fprintf(stderr, "Error: realloc in_opts\n");
                return -1;
            }

            in_opts[in_opts_count - 1].name = long_opts[i].name;
            in_opts[in_opts_count - 1].has_arg = long_opts[i].has_arg;
            in_opts[in_opts_count - 1].flag = long_opts[i].flag;
            in_opts[in_opts_count - 1].val = long_opts[i].val;

            old_plug_id = plug_id;
        }
    }

    free(in_opts);

    func_count++;
    rez = process_file(path, &in_opts, (size_t)in_opts_count, out_buff, STR_LEN);
    if(rez < 0)
        fprintf(stderr, "Error: process_file\n");
    else
        if (rez == 0)
            rez_count++;

    if ((rez_count <= func_count && flags.C == 0 && rez_count > 0) || (rez_count == func_count && flags.C == 1))
        return 0;
    else
        return -1;
}

int make_logfile(char * pt) {
    FILE * log_fp;
    printf("Making log file '%s'...\n", pt);
    log_fp = freopen(pt, "w", stderr);
    if (log_fp) {
        printf("Log file created\n");
        return 0;
    }
    else {
        fprintf(stderr, "Error: creating log file");
        return -1;
    }
}

int free_all() {
    if (plugin_count) {
        for (int i = 0; i < plugin_count; i++) {
            free(plugin_list[i] -> p_path);
            free(plugin_list[i] -> info.sup_opts);
            dlclose(plugin_list[i] -> handle);
            free(plugin_list[i]);
        }
        free(plugin_list);
    }
    for (int i = 0; i < long_opts_count; i++) {
        free(long_opts[i].flag);
    }
    free(long_opts);
    free(plugin_id_array);
    free(search_dir);
    free(plugin_dir);
    free(log_path);
    printf("\nHave a nice day!\n");
    return 0;
}
