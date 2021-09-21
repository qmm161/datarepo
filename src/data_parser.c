#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "data_parser.h"
#include "macro.h"
#include "cjson/cJSON.h"

struct mdd_node *mdm_parse_data(struct mds_node *schema, const char *data_json)
{
    (void)schema;
    (void)data_json;

    return NULL;
}

void mdm_free_data(struct mdd_node *root)
{
    (void)root;
}