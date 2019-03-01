#include <stdio.h>

#include "db2util.h"


static void* space_format_new(void) {
    return NULL;
}

static void space_format_free(void* st) {}

static void space_row(FILE* f, void* state, col_info_t* cols, int count) {
    const char* separator = "";

    for(int i = 0; i < count; ++i) {
        printf("%s\"%.*s\"", separator, cols[i].ind, cols[i].data);
        separator = " ";
    }

    printf("\n");
}

static void space_start_rows(FILE* f, void* state) {}

static void space_end_rows(FILE* f, void* state) {}

format_t space_format = {
    space_format_new,
    space_format_free,
    space_row,
    space_start_rows,
    space_end_rows
};
