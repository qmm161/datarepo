#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "model.h"
#include "cjson/cJSON.h"

#define is_mo(mtype) ((mtype)==MDS_MT_CONTAINER || (mtype)==MDS_MT_LIST)
#define is_leaf(mtype) ((mtype)==MDS_MT_LEAF)

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
        printf("invalid mtype\n");
        return NULL;
    }

    if(is_mo(mtype)) {
        node = (struct mds_node*)calloc(1, sizeof(struct mds_mo));
        node->name = strdup(json_node->string);
        node->mtype = mtype;
        printf("build self mo-> name:%s, mtype:%d\n", node->name, node->mtype);
    } else {
        struct mds_leaf *leaf = (struct mds_leaf*)calloc(1, sizeof(struct mds_leaf));
        leaf->name = strdup(json_node->string);
        leaf->mtype = mtype;
        leaf->dtype = get_dtype(json_node);
        printf("build self leaf-> name:%s, mtype:%d, dtype=%d\n", leaf->name, leaf->mtype, leaf->dtype);
        node = (struct mds_node*)leaf;
    }
    
    return node;
}

static struct mds_node *build_child_node(struct mds_node * parent , cJSON *json_node)
{
    struct mds_node *node = build_self_node(json_node);
    if(!node) {
        return NULL;
    }

    parent->child = node;
    node->parent = parent;
    return node;
}

static struct mds_node *build_next_node(struct mds_node *sib , cJSON *json_node)
{
    struct mds_node *node = build_self_node(json_node);
    if(!node) {
        return NULL;
    }

    sib->next = node;
    node->prev = sib;
    node->parent = sib->parent;
    return node;
}

static cJSON *find_child_schema(cJSON *node)
{
    if(!node) {
        return NULL;
    }

    cJSON *target = node->child;
    while(target) {
        if(strcmp(target->string, "@attr")) {
            printf("find child schema: %s for %s\n", target->string, node->string);
            return target;
        }
        target = target->next;
    }
    return NULL;
}

static cJSON *find_next_schema(cJSON *node)
{
    if(!node) {
        return NULL;
    }

    cJSON *target = node->next;
    while(target) {
        if(strcmp(target->string, "@attr")) {
            printf("find next schema: %s for %s\n", target->string, node->string);
            return target;
        }
        target = target->next;
    }
    return NULL;
}

static struct mds_node *build_mds_node(cJSON *json_node) 
{
    struct mds_node *node = build_self_node(json_node);
    if(is_mo(node->mtype)) {
        printf("parse %s is mo\n", node->name);
        cJSON *json_child = find_child_schema(json_node);
        cJSON *last_json_child = json_child;
        struct mds_node *parent = node;
        while(json_child) {
            struct mds_node *child = build_child_node(parent, json_child);
            if (!child) {
                goto ERR_OUT;
            }
            parent = child;
            last_json_child = json_child;
            json_child = find_child_schema(json_child);
        }

        cJSON *json_next = find_next_schema(last_json_child);
        cJSON *last_json_next = json_next;
        struct mds_node *left_sib = parent;
        while(json_next) {
            struct mds_node *next = build_next_node(left_sib, json_next);
            if(!next) {
                goto ERR_OUT;
            }
            left_sib = next;
            last_json_next = json_next;
            json_next = find_next_schema(json_next);
        }
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
    if (!root) {
        return NULL;
    }

    cJSON *data = locate_child(root, "Data");

    model_data = build_mds_node(data);
    
    cJSON_Delete(root);
    return model_data;
}

// static mdm_free_node(struct mds_node *node)
// {
//     if(node) {}
// }

void mdm_free_model(struct mds_node *root)
{
    struct mds_node *child = root->child;
    // while(child) {

    // }
}