#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "lab1.h"
#include "plugin_api.h"

int main(int argc, char *argv[]) {
    int opt, option_index, file_count;;
    search_dir = NULL;
    plugin_dir = NULL;
    log_path = NULL;
    cond = 0;
    version = "1.0.0";
    plugin_id_array = NULL;
    long_opts_count = 0;
    long_opts = (struct option *)calloc(OPTS_COUNT + 1, sizeof(struct option));
    plugin_count = 0;
    plugin_list = NULL;
    flags.P = flags.l = flags.N = flags.h = flags.v = flags.C = flags.long_opt = flags.wrong_opt = 0;

    if (long_opts == NULL) {
        fprintf(stderr, "Ошибка: не выделено места под опции\n");
        exit(1);
    }
    add_to_long_opts("plugin", required_argument, NULL, 'P');
    add_to_long_opts("log", required_argument, NULL, 'l');
    add_to_long_opts("condition", required_argument, NULL, 'C');
    add_to_long_opts("invert", no_argument, NULL, 'N');
    add_to_long_opts("version", no_argument, NULL, 'v');
    add_to_long_opts("help", no_argument, NULL, 'h');

    while (1) {
        option_index = 0;
        opt = getopt_long(argc, argv, "-:P:l:vh", long_opts, &option_index);
        if (opt == -1)
            break;
        switch (opt) {
            case 1:
                break;
            case 'P':
                if (flags.P) {
                    printf("'-P' given twice\n");
                    flags.wrong_opt++;
                }
                else {
                    flags.P++;
                    plugin_dir = strdup(optarg);
                }
                break;
            case 'l':
                if (flags.l) {
                    printf("'-l' given twice\n");
                    flags.wrong_opt++;
                }
                else {
                    flags.l++;
                    log_path = strdup(optarg);

                }
                break;
            case 'v':
                if (flags.v) {
                    printf("'-v' given twice\n");
                    flags.wrong_opt++;
                }
                else {
                    flags.v++;
                    printf("Version : %s\n", version);
                }
                break;
            case 'h':
                if (flags.h) {
                    printf("'-h' given twice\n");
                    flags.wrong_opt++;
                }
                else {
                    flags.h++;

                }
                break;
            case '?':
                //printf("Incorrect option %s\n", argv[optind]);
                //free(long_opts);
                //printf("free(long_opts)\n");
                //exit(EXIT_FAILURE);
            break;
            /*case 0:
                flags.long_opt++;
                printf("option %s", long_opts[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;*/
            default:
                printf("?? getopt returned character code %c ??\n", opt);
        }
    }

    if (log_path) {
        if (make_logfile(log_path)) {
            printf("Termination...\n");
            free_all();
            exit(EXIT_FAILURE);
        }

    }

    if (!plugin_dir)
        plugin_dir = strdup(".");
    printf("\nPlugin's directory: '%s'\n", plugin_dir);
    if (search_for_plugins(plugin_dir) == -1) {
        fprintf(stderr, "Error: can't open '%s'\n", plugin_dir);
        printf("No plugins found in '%s'\n", plugin_dir);
    }
    else {
        printf("%d plugin(s) found in '%s'\n", plugin_count,  plugin_dir);
    }

    if (flags.h) {
        show_help();
    }
    //reset
    optind = 0;
    while (1) {
        option_index = 0;
        opt = getopt_long(argc, argv, "+P:l:vhCN", long_opts, &option_index);
        if (opt == -1)
            break;
        switch (opt) {
            case 1:
            case 'P':
            case 'l':
            case 'v':
            case 'h':
                break;
            case 'C':
                if (flags.C) {
                    flags.wrong_opt++;
                    printf("'-C' given twice\n");
                }
                else {
                    flags.C++;
                    printf("option -C");
                    if (optarg)
                        printf(" with arg %s", optarg);
                    printf("\n");
                }
            break;
            case 'N':
                if (flags.N) {
                    flags.wrong_opt++;
                    printf("'-N' given twice\n");
                }
                else {
                    flags.N++;
                    printf("option -N\n");
                }
            break;
            case '?':
                flags.wrong_opt++;
                //printf("Incorrect option %s\n", argv[optind]);
                //free(long_opts);
                //printf("free(long_opts)\n");
                //exit(EXIT_FAILURE);
            break;
            case 0:
                if (long_opts[option_index].flag == NULL) {
                    flags.long_opt++;
                    //printf("option %s", long_opts[option_index].name);
                    if (optarg){

                        long_opts[option_index].flag = (int *)strdup(optarg);
                        //printf(" with arg %s", (char *)long_opts[option_index].flag);
                    }
                    //printf("\n");
                }
                else {
                    printf("'--%s' given twice\n", long_opts[option_index].name);
                    flags.wrong_opt++;
                }
                break;
            default:
                printf("?? getopt returned character code %c ??\n", opt);
        }
    }

    if (!flags.wrong_opt) {
        if (optind < argc) {
            search_dir = strdup(argv[optind]);
            printf("\nThe directory to search: '%s'\n", search_dir);
            if (flags.long_opt) {
                printf("Searching for files in '%s' ...\n", search_dir);
                file_count = search_for_files(search_dir);
                if (file_count == -1) {
                    fprintf(stderr, "Error: can't open '%s'\n", search_dir);
                }
                printf("%d file(s) found\n", file_count);
            }
            else {
                printf("No option for searching found\n");
            }
        }
        else {
            printf("\nNo directory to search :(\n");
            if (flags.h == 0) show_help();
        }
    } else {
        printf("Termination...\n");
        free_all();
        exit(EXIT_FAILURE);
    }

    free_all();
    exit(EXIT_SUCCESS);
}














    /*if (argc > 1) {
        // Поиск -P
        for (int i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "-P")) {
                if ((i < argc - 2) && (opendir(argv[i + 1])) != NULL) {
                    dir = argv[i + 1];
                    printf("%s - каталог с плагинам\n", dir);
                }
                else
                    printf("Каталог плагинов введён некорректно или не введён\n");
            }
        }
        // Проверка последнего аргумента
        if (opendir(argv[argc - 1]) != NULL) {
            cat = argv[argc - 1];
            printf("Указан каталог для поиска %s\n", cat);
            return 0;
        }
        else
            printf("Каталог для поиска введён некорректно или не введён\n");
    }
    else
        printf("Программа запущена без аргументов\n");
    printf("Справка:\n");


    return 0;
}*/
