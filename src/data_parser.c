#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "data_parser.h"
#include "macro.h"
#include "cjson/cJSON.h"

static struct mdd_node *build_mdd_node(struct mds_node *schema, cJSON *data_json);

static void mdd_free_self_node(struct mdd_node *node)
{
    CHECK_RTN(!node);

    if (is_leaf(node->schema->mtype)) {
        struct mdd_leaf *leaf = (struct mdd_leaf*)node;
        if(((struct mds_leaf*)leaf->schema)->dtype == MDS_DT_STR) {
            free(leaf->value.strv);
        }
    }
    free(node);
}

void mdd_free_data(struct mdd_node *root)
{
    CHECK_RTN(!root);

    struct mdd_node *child = root->child;
    struct mdd_node *next = root->next;

    if(child) {
        mdd_free_data(child);
    }

    if(next) {
        mdd_free_data(next);
    }

    mdd_free_self_node(root);
}

static struct mdd_node *build_container_node(struct mds_node *schema, cJSON *data_json)
{
    printf("mdd--try build container: %s-%s\n", schema->name, data_json->string);
    struct mdd_mo *mo = (struct mdd_mo*) calloc(1, sizeof(struct mdd_mo));
    mo->schema = schema;
    return mo;
}

static struct mdd_node *build_list_node(struct mds_node *schema, cJSON *data_json)
{
    return NULL;
}

static struct mdd_node *build_leaf_node(struct mds_node *schema, cJSON *data_json)
{
    printf("mdd--try build leaf: %s-%s\n", schema->name, data_json->string);
    struct mds_leaf *leaf_schema = (struct mds_leaf*)schema;
    struct mdd_leaf *leaf = (struct mdd_leaf*) calloc(1, sizeof(struct mdd_leaf));
    CHECK_DO_RTN_VAL(!leaf, printf("no memory!\n"), NULL);

    leaf->schema = schema;
    if(leaf_schema->dtype == MDS_DT_STR) {
        printf("mdd--try build str leaf: %s-%s\n", schema->name, data_json->valuestring);
        CHECK_DO_RTN_VAL(!cJSON_IsString(data_json), printf("mdd--data is not string\n");free(leaf), NULL);
        leaf->value.strv = strdup(data_json->valuestring);
    } else {
        printf("mdd--try build int leaf: %s-%lld\n", schema->name, data_json->valueint);
        CHECK_DO_RTN_VAL(!cJSON_IsNumber(data_json), printf("mdd--data is not number\n");free(leaf), NULL);
        leaf->value.intv = data_json->valueint;
    }
    return leaf;
}

static struct mdd_node *build_self_node(struct mds_node *schema, cJSON *data_json)
{
    struct mdd_node *data = NULL;
    CHECK_DO_RTN_VAL(strcmp(schema->name, data_json->string), 
        printf("mdd--schema not match %s-%s\n", schema->name, data_json->string), NULL);

    printf("mdd--try build self node: %s-%s\n", schema->name, data_json->string);

    if (is_mo(schema->mtype)) {
        if(schema->mtype == MDS_MT_CONTAINER && cJSON_IsObject(data_json)) {
            data = build_container_node(schema, data_json);
        } else if (schema->mtype == MDS_MT_LIST && cJSON_IsArray(data_json)) {
            data = build_list_node(schema, data_json);
        } else {
            printf("mdd--invalid mo\n");
        }
    } else {
        data = (struct mdd_node*)build_leaf_node(schema, data_json);
    }

    printf("mdd--succ build self node: %s-%s\n", schema->name, data_json->string);

    return data;
}

static void find_child_data(struct mds_node *parent_schema, cJSON *parent_json, 
    struct mds_node **child_schema, cJSON **child_json)
{
    CHECK_RTN(!parent_json->child);

    *child_json = parent_json->child;
    const char *name = (*child_json)->string;

    *child_schema = mdm_find_child_schema(parent_schema, name);
    if (!(*child_schema)) {
        printf("mdd--invalid child data node name:%s\n", name);
        *child_json = NULL;
    }
}

static void find_next_data(struct mds_node *curr_schema, cJSON *curr_json, 
    struct mds_node **next_schema, cJSON **next_json)
{
    CHECK_RTN(!curr_json->next);

    *next_json = curr_json->next;
    const char *name = (*next_json)->string;

    *next_schema = mdm_find_next_schema(curr_schema, name);
    if (!(*next_schema)) {
        printf("mdd--invalid next data node name:%s\n", name);
        *next_json = NULL;
    }
}

static struct mdd_node *build_child_data(struct mdd_node *curr, struct mds_node *schema, cJSON *data_json)
{
    struct mdd_node *node = build_mdd_node(schema, data_json);
    CHECK_RTN_VAL(!node, NULL);

    curr->child = node;
    node->parent = curr;
    return node;
}

static struct mdd_node *build_next_data(struct mdd_node *curr, struct mds_node *schema, cJSON *data_json)
{
    struct mdd_node *node = build_mdd_node(schema, data_json);
    CHECK_RTN_VAL(!node, NULL);

    curr->next = node;
    node->prev = curr;
    node->parent = curr->parent;
    printf("mdd--build next %s by %s\n", node->schema->name, curr->schema->name);
    return node;
}

static struct mdd_node *build_mdd_node(struct mds_node *schema, cJSON *data_json)
{
    struct mdd_node *node = build_self_node(schema, data_json);
    struct mdd_node *child = NULL;
    struct mdd_node *next = NULL;

    cJSON *json_child = NULL;
    cJSON *json_next = NULL;

    struct mds_node *schema_child = NULL;
    struct mds_node *schema_next = NULL;

    CHECK_GOTO(!node, ERR_OUT);
    printf("mdd--parse data %s as %d\n", node->schema->name, node->schema->mtype);

    find_child_data(schema, data_json, &schema_child, &json_child);
    if (json_child && schema_child) {
        printf("mdd--try build child %s under %s-%lld-%lld\n", schema_child->name, node->schema->name, node, node->child);
        child = build_child_data(node, schema_child, json_child);
        CHECK_GOTO(!child, ERR_OUT);
    } 

    find_next_data(schema, data_json, &schema_next, &json_next);
    if (json_next && schema_next) {
        printf("mdd--try build next %s by %s\n", schema->name, schema_next->name);
        next = build_next_data(node, schema_next, json_next);
        CHECK_GOTO(!next, ERR_OUT);
    }

    printf("mdd--parse node succ:%s\n", node->schema->name);
    return node;

    ERR_OUT:
    mdd_free_data(node);
    return NULL;
}

struct mdd_node *mdd_parse_data(struct mds_node *schema, const char *data_json)
{
    struct mdd_node *data_root = NULL;
    cJSON *json_root = cJSON_Parse(data_json);
    CHECK_RTN_VAL(!json_root || !json_root->child, NULL);

    data_root = build_mdd_node(schema, json_root->child);
    
    cJSON_Delete(json_root);
    return data_root;
}
