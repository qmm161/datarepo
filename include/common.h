#ifndef __MDM_COMMON_H_
#define __MDM_COMMON_H_

#include <stddef.h>

struct mdd_vector{
    size_t capacity;
    size_t size;
    void **vec;
};

int vector_init(struct mdd_vector *vec, void *ele);
int vector_add(struct mdd_vector *vec, void *ele);
void vector_free(struct mdd_vector *vec);

#endif
