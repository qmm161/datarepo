#include <stdio.h>
#include <stdlib.h>
#include "include/model.h"

int main()
{
    FILE * fd = fopen("../model/data_model.json", "r" );
    char *buf = calloc(1, 1024*1024*2);
    fread(buf, sizeof(char), 1024*1024*2, fd);
    fclose(fd);

    struct mds_node * model = mdm_load_model(buf);
    mdm_free_model(model);

    free(buf);

    return 0;
}