#ifndef _COMMON_H_
#define _COMMON_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct mdd_vector
    {
        size_t capacity;
        size_t size;
        void **vec;
    };

    int vector_init(struct mdd_vector *vec, void *ele);
    int vector_add(struct mdd_vector *vec, void *ele);
    void vector_free(struct mdd_vector *vec);

#ifdef __cplusplus
}
#endif

#endif
