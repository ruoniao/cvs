//
// Created by zhaogang on 2025-03-01.
//

#include "cmd.h"

int
cmd_parse_option(int argc, char *argv[]){
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "--config") == 0){
            cvs_cmd_config.config = argv[i+1];
        }
        /*
        if(strcmp(argv[i], "--db") == 0){
            cvs_cmd_config.db = argv[i];
        }
        if(strcmp(argv[i], "--log") == 0){
            cvs_cmd_config.log = argv[i];
        }
        if(strcmp(argv[i], "--unix") == 0){
            cvs_cmd_config.unix_path = argv[i];
        }
        if(strcmp(argv[i], "--tcp") == 0){
            cvs_cmd_config.tcp = argv[i];
        }*/
        if(strcmp(argv[i], "--help") == 0){
            printf("Usage: cvs-vswitchd [OPTION]...\n"
                   "  --config  config file path\n"
                   "  --db      db file path\n"
                   "  --log     log file path\n"
                   "  --unix    unix socket path\n"
                   "  --tcp     tcp socket path\n"
                   "  --help    help\n");
            exit(0);
        }
    }
    return 1;
}