#ifndef _DATA_REPO_
#define _DATA_REPO_

#include "data_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

int repo_init(const char *schema_path, const char *data_path);
void repo_free();

int repo_get(const char *path, struct mdd_node **out);
int repo_edit(const char *edit_data);

#ifdef __cplusplus
}
#endif

#endif