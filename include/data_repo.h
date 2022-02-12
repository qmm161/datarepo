#ifndef _DATA_REPO_
#define _DATA_REPO_

#include "cjson/cJSON.h"
#include "data_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

    int repo_init(const char *schema_path, const char *data_path);
    void repo_free();

    int repo_get(const char *path, struct mdd_node **out);
    int repo_edit(const char *edit_data);
    int repo_edit_json(const cJSON *edit_data);

#define int_leaf_val(node) ((struct mdd_leaf*)node)->value.intv
#define str_leaf_val(node) ((struct mdd_leaf*)node)->value.strv

#ifdef __cplusplus
}
#endif

#endif
