#include "list.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return 1;
    }

    printf("Reading from file: %s\n", argv[1]);

    list_t *l = list_init_from_path(argv[1],
                                    free,
                                    (parse_record_func_t)vec_parse_str);

    printf("Number of lines: %zd\n", list_size(l));

    vector_t *v;
    for (size_t i = 0; i < list_size(l); i++) {
        v = (vector_t *)list_get(l, i);
        printf("(%f, %f)\n", v->x, v->y);
    }

    list_free(l);

    return 0;
}