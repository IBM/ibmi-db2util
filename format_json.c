#include <stdio.h>

#include "db2util.h"

struct state {
    const char* row_separator;
};

static void* json_format_new(void) {
    struct state* st = malloc(sizeof(struct state));

    st->row_separator = "";

    return st;
}

static void json_format_free(void* st) {
    free(st);
}

static void json_row(FILE* f, void* state, col_info_t* cols, int count) {
    struct state* st = state;

    printf("%s{", st->row_separator);
    st->row_separator = ",\n";

    const char* quote;
    
    const char* comma = "";
    for(int i = 0; i < count; ++i) {
        col_info_t* col = &cols[i];

        switch(col->type) {
        case SQL_BIGINT:
        case SQL_DECFLOAT:
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_REAL:
        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_DECIMAL:
        case SQL_NUMERIC:
            quote = "";
            break;

        default:
            quote = "\"";
            break;
        }
        
        const char* buffer = col->data;
        int length = col->ind;

        if(length == SQL_NULL_DATA) {
            buffer = "null";
            length = 4;
        }

        printf("%s\"%s\":%s%.*s%s", comma, col->name, quote, length, buffer, quote);

        comma = ",";
    }

    printf("}");
}

static void json_start_rows(FILE* f, void* state) {
    struct state* st = state;

    st->row_separator = "";
    fprintf(f, "[\n");
}

static void json_end_rows(FILE* f, void* state) {
    fprintf(f, "\n]\n");
}

format_t json_format = {
    json_format_new,
    json_format_free,
    json_row,
    json_start_rows,
    json_end_rows
};
