#ifndef __DATA_PARSER_H
#define __DATA_PARSER_H

#include "model_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union
{
    long long intv;
    char *strv;
} mdd_dvalue;

struct mdd_node
{
    struct mds_node *schema;

    struct mdd_node *parent;
    struct mdd_node *child;
    struct mdd_node *prev;
    struct mdd_node *next;
};

struct mdd_mo
{
    struct mds_node *schema;

    struct mdd_node *parent;
    struct mdd_node *child;
    struct mdd_node *prev;
    struct mdd_node *next;
};

struct mdd_leaf
{
    struct mds_node *schema;

    struct mdd_node *parent;
    struct mdd_node *child;
    struct mdd_node *prev;
    struct mdd_node *next;

    mdd_dvalue value;
};

struct mdd_node *mdm_parse_data(struct mds_node *schema, const char *data_json);
void mdm_free_data(struct mdd_node *root);


#ifdef __cplusplus
}
#endif

#endif