#include <stdio.h>
#include <stdlib.h>

#include "db2util.h"

struct state {
    const char* row_separator;
    const char* start_extra;
    const char* end_extra;
};


static void* json_format_new(void) {
    struct state* st = malloc(sizeof(struct state));

    st->row_separator = "";

    const char* json_format = getenv("DB2UTIL_JSON_CONTAINER");
    if (!json_format || strcmp(json_format, "object") == 0) {
        st->start_extra = "{\"records\":";
        st->end_extra = "}\n";
    }
    else if (strcmp(json_format, "array") == 0) {
        st->start_extra = "";
        st->end_extra = "";
    }
    else {
        fprintf(stderr, "Invalid value for DB2UTIL_JSON_CONTAINER: %s\n", json_format);
        exit(2);
    }

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
        else if (length == SQL_NTS) {
            length = strlen(buffer);
        }

        printf("%s\"%s\":%s", comma, col->name, quote);
        
        for (int y = 0; y < length; y++) {
            switch (buffer[y]) {
                case '\n':
                    putchar('\\');
                    putchar('n');
                    break;
                case '\t':
                    putchar('\\');
                    putchar('t');
                    break;
                case '\r':
                    putchar('\\');
                    putchar('r');
                    break;
                case '\f':
                    putchar('\\');
                    putchar('f');
                    break;
                case '\b':
                    putchar('\\');
                    putchar('b');
                    break;
                    
                case '\\':
                case '"':
                    putchar('\\');
                    // fall through
                default:
                    putchar(buffer[y]);
                    break;
            }
        }
        
        printf("%s", quote);

        comma = ",";
    }

    printf("}");
}

static void json_start_rows(FILE* f, void* state) {
    struct state* st = state;

    fputs(st->start_extra, f);
    fputs("[\n", f);

    st->row_separator = "";
}

static void json_end_rows(FILE* f, void* state) {
    struct state* st = state;
    
    fputs("\n]", f);
    fputs(st->end_extra, f);
    fputs("\n", f);
}

format_t json_format = {
    json_format_new,
    json_format_free,
    json_row,
    json_start_rows,
    json_end_rows
};
