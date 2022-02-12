#include <stdio.h>
#include <stdlib.h>
#include "include/data_repo.h"
#include "log.h"
#include "macro.h"

int main()
{
    int rt = repo_init("../model/data_model.json", "../model/data.json");
    LOG_INFO("repo init with rlt: %d", rt);
    CHECK_RTN_VAL(rt, -1);

    struct mdd_node *out = NULL;
    rt = repo_get("Data/Volume", &out);
    LOG_INFO("repo get with rlt: %d", rt);
    if (!rt) {
        LOG_INFO("repo get path %s with val: %d", "Data/Volume", ((struct mdd_leaf* )out)->value.intv);
    }

    rt = repo_edit("{\"Data\": {\"Name\": \"vc1000\", \"Volume\": 200}}");
    LOG_INFO("repo edit with rlt: %d", rt);

    repo_free();
    return 0;
}
