#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "data_parser.h"
#include "macro.h"
#include "cjson/cJSON.h"
#include "log.h"

static struct mdd_node *build_mdd_node(struct mds_node *schema, cJSON *data_json, struct mdd_node *parent);

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

static struct mdd_node *build_container_node(struct mds_node *schema, cJSON *data_json, struct mdd_node *parent)
{
    CHECK_DO_RTN_VAL(!cJSON_IsObject(data_json), LOG_WARN("invalid container data"), NULL);

    LOG_INFO("mdd--try build container or list: %s", schema->name);
    struct mdd_mo *node = (struct mdd_mo*) calloc(1, sizeof(struct mdd_mo));
    node->schema = schema;
    node->parent = parent;

    cJSON *data_child = data_json->child;
    parent = node;
    struct mdd_node *prev = NULL;
    while(data_child) {
        struct mds_node *schema_child = mdm_find_child_schema(schema, data_child->string);
        struct mdd_node *node_child = NULL;
        CHECK_DO_GOTO(!schema_child, 
            LOG_WARN("invalid child data name %s under %s", data_json->string, schema->name),ERR_OUT);
        
        node_child = build_mdd_node(schema_child, data_child, parent);
        if(!prev) {
            parent->child = node_child;
        } else {
            prev->next = node_child;
            node_child->prev = prev;
        }
        prev = node_child;

        data_child = data_child->next;
    }
    return node;

    ERR_OUT:
    mdd_free_data(node);
    return NULL;
}

static struct mdd_node *build_list_node(struct mds_node *schema, cJSON *data_json, struct mdd_node *parent)
{
    CHECK_DO_RTN_VAL(!cJSON_IsArray(data_json), LOG_WARN("invalid list data"), NULL);

    LOG_INFO("mdd--trye build list: %s-%s", schema->name, data_json->string);
    struct mdd_node *first = NULL;
    struct mdd_node *prev = NULL;
    struct mdd_node *node = NULL;

    cJSON *element = data_json->child;
    while(element) {
        node = build_container_node(schema, element, parent);
        if (!first) {
            first = node;
        }
        if (prev) {
            prev->next = node;
            node->prev = prev;
        }
        prev = node;

        element = element->next;
    }

    return first;
}

static struct mdd_node *build_leaf_node(struct mds_node *schema, cJSON *data_json, struct mdd_node *parent)
{
    LOG_INFO("mdd--try build leaf: %s-%s", schema->name, data_json->string);
    struct mds_leaf *leaf_schema = (struct mds_leaf*)schema;
    struct mdd_leaf *leaf = (struct mdd_leaf*) calloc(1, sizeof(struct mdd_leaf));
    CHECK_DO_RTN_VAL(!leaf, LOG_WARN("no memory!"), NULL);

    leaf->schema = schema;
    leaf->parent = parent;
    if(leaf_schema->dtype == MDS_DT_STR) {
        LOG_DEBUG("mdd--try build str leaf: %s-%s", schema->name, data_json->valuestring);
        CHECK_DO_RTN_VAL(!cJSON_IsString(data_json), LOG_WARN("mdd--data is not string");free(leaf), NULL);
        leaf->value.strv = strdup(data_json->valuestring);
    } else {
        LOG_DEBUG("mdd--try build int leaf: %s-%d", schema->name, data_json->valueint);
        CHECK_DO_RTN_VAL(!cJSON_IsNumber(data_json), LOG_WARN("mdd--data is not number");free(leaf), NULL);
        leaf->value.intv = data_json->valueint;
    }
    return leaf;
}

static struct mdd_node *build_mdd_node(struct mds_node *schema, cJSON *data_json, struct mdd_node *parent)
{
    struct mdd_node *node = NULL;
    
    if(is_cont_node(schema)) {
        node = build_container_node(schema, data_json, parent);
    } else if(is_list_node(schema)) {
        node = build_list_node(schema, data_json, parent);
    } else if(is_leaf_node(schema)) {
        node = build_leaf_node(schema, data_json, parent);
    }

    return node;
}

struct mdd_node *mdd_parse_data(struct mds_node *schema, const char *data_json)
{
    struct mdd_node *data_root = NULL;
    cJSON *json_root = cJSON_Parse(data_json);
    CHECK_RTN_VAL(!json_root || !json_root->child, NULL);

    data_root = build_container_node(schema, json_root->child, NULL);
    
    cJSON_Delete(json_root);
    return data_root;
}
