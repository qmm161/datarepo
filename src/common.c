#include <string.h>
#include <stdlib.h>
#include "macro.h"
#include "log.h"
#include "common.h"

#define DEFAULT_CAP 10

int vector_init(struct mdd_vector *dvec, void *ele)
{
    CHECK_NULL_RTN(dvec, -1);

    memset(dvec, 0, sizeof(struct mdd_vector));
    dvec->vec = calloc(DEFAULT_CAP, sizeof(void*));
    CHECK_DO_RTN_VAL(!dvec->vec, LOG_WARN("No memory."), -1);

    dvec->capacity = DEFAULT_CAP;
    dvec->size = 0;
    if(ele) {
        dvec->vec[0] = ele;
        dvec->size = 1;
    }
    return 0;
}

int vector_add(struct mdd_vector *dvec, void *ele)
{
    CHECK_NULL_RTN(dvec, -1);

    if(dvec->capacity == dvec->size) {
        size_t newcap = dvec->capacity * 2;
        void *newp = realloc(dvec->vec, newcap);
        CHECK_DO_RTN_VAL(!newp, LOG_WARN("No memory."), -1);

        dvec->vec = newp;
        dvec->capacity = newcap;
    }

    dvec->vec[dvec->size] = ele;
    dvec->size++;
}

void vector_free(struct mdd_vector *dvec)
{
    CHECK_NULL(dvec);

    free(dvec->vec);
    dvec->size = 0;
    dvec->capacity = 0;
}