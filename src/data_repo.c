#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macro.h"
#include "log.h"
#include "data_repo.h"
#include "data_parser.h"
#include "model_parser.h"

struct repo_ctx
{
    const char *schema_file;
    const char *data_file;
    struct mds_node *schema;
    struct mdd_node *running;
    struct mdd_node *editing;
};

static struct repo_ctx ctx;

static char *load_file(const char *file_path)
{
    long size = 0;
    char *buffer;
    size_t result;
    FILE *fp = fopen (file_path, "r");
    CHECK_DO_RTN_VAL(!fp, LOG_WARN("failed to load file: %s", file_path), NULL);

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    buffer = (char*) calloc(1, sizeof(char)*size);
    CHECK_DO_RTN_VAL(!buffer, fclose(fp), NULL);

    result = fread (buffer, 1, size, fp);
    if (result != size) {
        LOG_WARN("failed to read data from file: %s", file_path);
        fclose(fp);
        free(buffer);
        return NULL;
    }
    fclose(fp);
    return buffer;
}

int repo_init(const char *schema_path, const char *data_path)
{
    memset(&ctx, 0, sizeof(struct repo_ctx));
    ctx.schema_file = strdup(schema_path);
    ctx.data_file = strdup(data_path);

    char *schema_buff = load_file(schema_path);
    CHECK_DO_RTN_VAL(!schema_buff, LOG_WARN("failed to load schema"), -1);
    ctx.schema = mds_load_model(schema_buff);
    free(schema_buff);
    schema_buff = NULL;
    
    char *data_buff = load_file(data_path);
    CHECK_DO_RTN_VAL(!data_buff, LOG_WARN("failed to load data"), -1);
    ctx.running = mdd_parse_data(ctx.schema, data_buff);
    free(data_buff);
    data_buff = NULL;

    return 0;
}

void repo_free()
{
    free(ctx.data_file);
    free(ctx.schema_file);
    mdd_free_data(ctx.running);
    mdd_free_data(ctx.editing);
    mds_free_model(ctx.schema);
    memset(&ctx, 0, sizeof(struct repo_ctx));
}

int repo_get(const char *path, struct mdd_node **out)
{
    CHECK_DO_RTN_VAL(!path || !out, LOG_WARN("NULL Para"), -1);

    *out = mdd_get_data(ctx.running, path);
    return (*out) ? 0 : -1;
}

//TODO: consider file broken 
static int write_file(const char *file_path, char *buffer)
{
    long size = 0;
    size_t result;
    FILE *fp = fopen (file_path, "w");
    CHECK_DO_RTN_VAL(!fp, LOG_WARN("failed to load file: %s", file_path), -1);

    result = fwrite (buffer, strlen(buffer), 1, fp);
    fclose(fp);
    if (result != size) {
        LOG_WARN("failed to write data to file: %s", file_path);
        return -1;
    }
    return 0;
}

int repo_edit(const char *edit_data)
{
    ctx.editing = mdd_parse_data(ctx.schema, edit_data);
    CHECK_DO_RTN_VAL(!ctx.editing, LOG_WARN("Failed to parse edit data"), -1);
    
    mdd_diff *diff = mdd_get_diff(ctx.schema, ctx.running, ctx.editing);
    if(diff) {  //TODO: register diff callback
        mdd_dump_diff(diff);
        mdd_free_diff(diff);
    }

    mdd_free_data(ctx.running);
    ctx.running = ctx.editing;
    ctx.editing = NULL;

    char *buf = NULL;
    int rt = mdd_dump_data(ctx.running, &buf);
    CHECK_DO_RTN_VAL(rt, LOG_WARN("Failed to dump new data"), -1);

    rt = write_file(ctx.data_file, buf);
    free(buf);
    CHECK_DO_RTN_VAL(rt, LOG_WARN("Failed to persist new data"), -1);

    return rt;
}
