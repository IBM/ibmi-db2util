#include <stdio.h>

#include "db2util.h"


static void* csv_format_new(void) {
    return NULL;
}

static void csv_format_free(void* st) {}

static void csv_row(FILE* f, void* state, col_info_t* cols, int count) {
    const char* separator = "";

    for(int i = 0; i < count; ++i) {
        col_info_t* col = &cols[i];
        
        const char* buffer = col->data;
        int length = col->ind;

        if(length == SQL_NULL_DATA) {
            buffer = "null";
            length = 4;
        }
        
        printf("%s\"", separator);
        
        for (int y = 0; y < length; y++) {
            if (buffer[y] == '"') {
                putchar('"');
            }
            putchar(buffer[y]);
        }
        
        putchar('"');
        
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
