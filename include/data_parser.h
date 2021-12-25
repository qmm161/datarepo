#ifndef __DATA_PARSER_H
#define __DATA_PARSER_H

#include "model_parser.h"
#include "common.h"

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

typedef enum {
    DF_ADD, DF_DELETE, DF_MODIFY
} mdd_diff_type;

struct mdd_leaf_diff
{
    struct mdd_leaf *edit_leaf;
    struct mdd_leaf *run_leaf;
};

struct mdd_mo_diff
{
    mdd_diff_type type;
    struct mdd_mo *edit_data;
    struct mdd_mo *run_data;

    struct mdd_vector diff_leafs;
};

typedef struct mdd_vector mdd_diff;

struct mdd_node *mdd_parse_data(struct mds_node *schema, const char *data_json);
void mdd_free_data(struct mdd_node *root);
struct mdd_node * mdd_get_data(struct mdd_node *root, const char *path);
int mdd_dump_data(struct mdd_node *root, char **json_str);
void mdd_free_diff(mdd_diff *diff);
mdd_diff * mdd_get_diff(struct mds_node *schema, struct mdd_node *root1, struct mdd_node *root2);

#ifdef __cplusplus
}
#endif

#endif