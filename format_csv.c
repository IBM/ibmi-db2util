#include <stdio.h>

#include "db2util.h"


static void* csv_format_new(void) {
    return NULL;
}

static void csv_format_free(void* st) {}

static void csv_row(FILE* f, void* state, col_info_t* cols, int count) {
    const char* separator = "";

    for(int i = 0; i < count; ++i) {
        printf("%s\"%.*s\"", separator, cols[i].ind, cols[i].data);
        separator = ",";
    }

    printf("\n");
}

static void csv_start_rows(FILE* f, void* state) {}

static void csv_end_rows(FILE* f, void* state) {}

format_t csv_format = {
    csv_format_new,
    csv_format_free,
    csv_row,
    csv_start_rows,
    csv_end_rows
};
