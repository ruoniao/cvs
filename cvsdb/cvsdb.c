//
// Created by zhaogang on 2025-03-01.
//

#include <stdlib.h>
#include "cvsdb.h"

int init_cvsdb(const char *db_path){
    db = (struct CvsDb *)malloc(sizeof(struct CvsDb));
    if(db == NULL){
        return -1;
    }
    db->root = cJSON_Parse(db_path);
    if(db->root == NULL){
        return -1;
    }
    return 1 ;
}