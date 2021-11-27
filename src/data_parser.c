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
        struct mds_node *schema_child = mds_find_child_schema(schema, data_child->string);
        struct mdd_node *node_child = NULL;
        CHECK_DO_GOTO(!schema_child, 
            LOG_WARN("invalid child data name %s under %s", data_child->string, schema->name),ERR_OUT);
        
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
    CHECK_DO_RTN_VAL(!cJSON_IsArray(data_json), LOG_WARN("invalid list data: %s", schema->name), NULL);

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

static void split_mo_key_value(char *fragment, char **mo, char **key, char **value)
{
    *mo = fragment;
    *key = NULL;
    *value = NULL;

    char *tmp = fragment;
    int len = strlen(fragment);
    for(int i = 0; i < len; i++) {
        if (*tmp == '[') {
            *tmp = '\0';
            *key = tmp + 1; 
        } else if (*tmp == '=') {
            *tmp = '\0';
            *value = tmp + 1;
        } else if (*tmp == ']') {
            *tmp = '\0';
        }
        tmp++;
    }
}

static char *next_word(char **path_head)
{
    int len = strlen(*path_head);
    char *word = *path_head;
    char *tail = *path_head;
    for (int i = 0; i < len; i++) {
        if (*tail == '/') {
            *tail = '\0';
            *path_head = tail + 1;
            return word;
        }
        tail++;
    }
    *path_head = tail;
    return word;
}

static int next_fragment(char **path_head, char **mo, char **key, char **value)
{
    char *fragment = next_word(path_head);
    if (!fragment || strlen(fragment) == 0) {
        return 0;
    }

    split_mo_key_value(fragment, mo, key, value);
    return 1;
}

static int match_node_value(struct mdd_node *node, char *value)
{
    CHECK_RTN_VAL(node->schema->mtype != MDS_MT_LEAF, 0); 

    struct mds_leaf* schema = (struct mds_leaf*)node->schema;
    struct mdd_leaf* leaf = (struct mdd_leaf*)node;

    if(schema->dtype == MDS_DT_STR) {
        return strcmp(leaf->value.strv, value) == 0 ? 1 : 0;
    } else if(schema->dtype == MDS_DT_INT) {
        char *end;
        return strtoll(value, &end, 10) == leaf->value.intv ? 1 : 0;
    }
    return 0;
}

static int match_node(struct mdd_node *node, char *name, char *key, char *value)
{
    int rlt = strcmp(node->schema->name, name);
    CHECK_RTN_VAL(rlt, 0);

    if (key && value) {
        for(struct mdd_node *iter = node->child; iter; iter = iter->next) {
            if(!strcmp(iter->schema->name, key) && match_node_value(iter, value)) {
                return 1;
            }
        }
        return 0;
    }
    return 1;
}

static struct mdd_node *find_child(struct mdd_node *cur, char *name, char *key, char *value)
{
    for(struct mdd_node *iter = cur->child; iter; iter = iter->next) {
        if (match_node(iter, name, key, value)) {
            return iter;
        }
    }
    return NULL;
}

struct mdd_node * mdd_get_data(struct mdd_node *root, const char *path)
{
    int rlt = 0, has_next = 0;
    struct mdd_node *out = NULL;
    char *dup_path = strdup(path);
    CHECK_DO_RTN_VAL(!dup_path, LOG_WARN("Failed to dup string"), NULL);
    
    struct mdd_node *target = NULL;
    char *name = NULL;
    char *key = NULL;
    char *value = NULL;
    char *tmp = dup_path;
    has_next = next_fragment(&tmp, &name, &key, &value);
    CHECK_DO_GOTO(!has_next, LOG_WARN("Failed to parse first fragment:%s", dup_path); rlt = -1, CLEAN);

    target = root;
    CHECK_DO_GOTO(!match_node(target, name, key, value), LOG_WARN("Failed to match root mo:%s-%s", name, target->schema->name); rlt = -1, 
        CLEAN);
    do {
        has_next = next_fragment(&tmp, &name, &key, &value);
        if (!has_next) {
            out = target;
            goto CLEAN;
        }
        target = find_child(target, name, key, value);
    } while(target);
    rlt = -1;

    CLEAN:
    if(dup_path) {
        free(dup_path);
    }
    return out;
}