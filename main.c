#include <stdio.h>
#include <stdlib.h>
#include "include/data_repo.h"
#include "log.h"

int main()
{
    int rlt = repo_init("../model/data_model.json", "../model/data.json");

    repo_free();

    LOG_INFO("repo init with rlt: %d", rlt);

    return rlt;
}