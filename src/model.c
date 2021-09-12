#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "model.h"
#include "cjson/cJSON.h"

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

static struct mds_node *build_mds_node(cJSON *json_node) 
{
    struct mds_node *node = NULL;

    mds_mtype mtype = get_mtype(json_node);
    if(mtype == MDS_MT_NULL) {
        printf("invalid mtype\n");
        return NULL;
    }

    if(mtype == MDS_MT_CONTAINER || mtype == MDS_MT_LIST) {
        node = (struct mds_node*)calloc(1, sizeof(struct mds_mo));
        node->name = strdup(json_node->string);
    } else {
        struct mds_leaf *leaf = (struct mds_leaf*)calloc(1, sizeof(struct mds_leaf));
        leaf->name = strdup(json_node->string);
        leaf->dtype = get_dtype(json_node);
        node = (struct mds_node*)leaf;
    }
    return node;
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

void mdm_free_model(struct mds_node *root)
{
    if (root) {
        free(root->name);
        free(root);
    }
}