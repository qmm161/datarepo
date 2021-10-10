#ifndef _DATA_REPO_
#define _DATA_REPO_

#include "data_parser.h"

int repo_init(const char *schema_path, const char *data_path);
void repo_free();
int repo_edit(const char *edit_data);

#endif