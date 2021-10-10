#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "model_parser.h"
#include "macro.h"
#include "cjson/cJSON.h"

static struct mds_node *build_mds_node(cJSON *json_node);


static cJSON *locate_child(cJSON *root, const char *name)
{
    if(root && cJSON_IsObject(root)) {
        for(cJSON * child = root->child; child; child = child->next) {
            if(strcmp(child->string, name) == 0) {
                return child;
            }
        }
    }
    return NULL;
}

static mds_mtype parse_mtype(const char *str)
{
    if(strcmp("container", str) == 0) {
        return MDS_MT_CONTAINER;
    } else if (strcmp("list", str) == 0) {
        return MDS_MT_LIST;
    } else if (strcmp("leaf", str) == 0) {
        return MDS_MT_LEAF;
    }
    return MDS_MT_NULL;
}

static mds_mtype get_mtype(cJSON *node)
{
    cJSON *attr = locate_child(node, "@attr");
    cJSON *mtype = locate_child(attr, "mtype");
    if (mtype) {
        return parse_mtype(mtype->valuestring);
    }
    return MDS_MT_NULL;
}

static mds_dtype parse_dtype(const char *str)
{
    if(strcmp("int", str) == 0) {
        return MDS_DT_INT;
    } else if (strcmp("string", str) == 0) {
        return MDS_DT_STR;
    } 
    return MDS_DT_NULL;
}

static mds_dtype get_dtype(cJSON *node)
{
    cJSON *attr = locate_child(node, "@attr");
    cJSON *dtype = locate_child(attr, "dtype");
    if (dtype) {
        return parse_dtype(dtype->valuestring);
    }
    return MDS_DT_NULL;
}

static struct mds_node * build_self_node(cJSON *json_node)
{
    struct mds_node *node = NULL;

    mds_mtype mtype = get_mtype(json_node);
    if(mtype == MDS_MT_NULL) {
        printf("mds--invalid mtype\n");
        return NULL;
    }

    if(is_mo(mtype)) {
        node = (struct mds_node*)calloc(1, sizeof(struct mds_mo));
        node->name = strdup(json_node->string);
        node->mtype = mtype;
        printf("mds--build self mo-> name:%s, mtype:%d\n", node->name, node->mtype);
    } else {
        struct mds_leaf *leaf = (struct mds_leaf*)calloc(1, sizeof(struct mds_leaf));
        leaf->name = strdup(json_node->string);
        leaf->mtype = mtype;
        leaf->dtype = get_dtype(json_node);
        printf("mds--build self leaf-> name:%s, mtype:%d, dtype=%d\n", leaf->name, leaf->mtype, leaf->dtype);
        node = (struct mds_node*)leaf;
    }
    
    return node;
}

static struct mds_node *build_child_node(struct mds_node * parent , cJSON *json_node)
{
    struct mds_node *node = build_mds_node(json_node);
    CHECK_RTN_VAL(!node, NULL);

    parent->child = node;
    node->parent = parent;
    return node;
}

static struct mds_node *build_next_node(struct mds_node *sib , cJSON *json_node)
{
    struct mds_node *node = build_mds_node(json_node);
    CHECK_RTN_VAL(!node, NULL);

    sib->next = node;
    node->prev = sib;
    node->parent = sib->parent;
    return node;
}

static cJSON *find_child_schema(cJSON *node)
{
    CHECK_RTN_VAL(!node, NULL);

    cJSON *target = node->child;
    while(target) {
        if(strcmp(target->string, "@attr")) {
            printf("mds--find child schema: %s for %s\n", target->string, node->string);
            return target;
        }
        target = target->next;
    }
    return NULL;
}

static cJSON *find_next_schema(cJSON *node)
{
    CHECK_RTN_VAL(!node, NULL);

    cJSON *target = node->next;
    while(target) {
        if(strcmp(target->string, "@attr")) {
            printf("mds--find next schema: %s for %s\n", target->string, node->string);
            return target;
        }
        target = target->next;
    }
    return NULL;
}

static struct mds_node *build_mds_node(cJSON *json_node) 
{
    struct mds_node *node = build_self_node(json_node);
    struct mds_node *child = NULL;
    struct mds_node *next = NULL;

    cJSON *json_child = NULL;
    cJSON *json_next = NULL;

    CHECK_GOTO(!node, ERR_OUT);
    printf("mds--parse %s as %d\n", node->name, node->mtype);

    json_child = find_child_schema(json_node);
    if (json_child) {
        child = build_child_node(node, json_child);
        CHECK_GOTO(!child, ERR_OUT);
    } 

    json_next = find_next_schema(json_node);
    if (json_next) {
        next = build_next_node(node, json_next);
        CHECK_GOTO(!next, ERR_OUT);
    }

    return node;

    ERR_OUT:
    mdm_free_model(node);
    return NULL;
}

struct mds_node *mdm_load_model(const char *model_str)
{
    struct mds_node *model_data = NULL;
    cJSON *root = cJSON_Parse(model_str);
    CHECK_RTN_VAL(!root, NULL);

    cJSON *data = locate_child(root, "Data");

    model_data = build_mds_node(data);
    
    cJSON_Delete(root);
    return model_data;
}

static void mdm_free_self_node(struct mds_node *node)
{
    if(node) {
        free(node->name);
        free(node);
    }
}

void mdm_free_model(struct mds_node *root)
{
    struct mds_node *child = root->child;
    struct mds_node *next = root->next;

    if(child) {
        mdm_free_model(child);
    }

    if(next) {
        mdm_free_model(next);
    }

    mdm_free_self_node(root);
}

struct mds_node *mdm_find_child_schema(struct mds_node*curr, const char *name)
{
    CHECK_RTN_VAL(!curr || !name, NULL);

    struct mds_node* child = curr->child;
    while (child)
    {
        if(!strcmp(child->name, name)) {
            return child;
        }
        child = child->next;
    }
    return NULL;
}

struct mds_node *mdm_find_next_schema(struct mds_node*curr, const char *name)
{
    CHECK_RTN_VAL(!curr || !name, NULL);

    struct mds_node* next = curr->next;
    while(next) {
        if(!strcmp(next->name, name)) {
            return next;
        }
        next = next->next;
    } 

    struct mds_node *prev = curr->prev;
    while(prev) {
        if(!strcmp(prev->name, name)) {
            return prev;
        }
        prev = prev->prev;
    }
    return NULL; 
}