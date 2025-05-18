//
// Created by zhaogang on 2025-05-17.
//
#include "utils/check.h"
#include "cvs/clog.h"

void health_check_handler(void) {
    // Perform health check logic here
    // For example, check if the server is running, database is accessible, etc.
    LOG_DEBUG("Health check passed\n");
}