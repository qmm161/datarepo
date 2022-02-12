#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "data_parser.h"
#include "macro.h"
#include "cjson/cJSON.h"
#include "log.h"

static struct mdd_node* build_mdd_node(struct mds_node *schema, cJSON *data_json, struct mdd_node *parent);
static int dump_mdd_node(struct mdd_node *node, char **buf, size_t *size, size_t *posi, struct mdd_node **next);
static int compare_list(struct mds_node *lists, struct mdd_node *mo_run_parent, struct mdd_node *mo_edit_parent,
        mdd_diff *diff);
static int compare_container(struct mds_node *mos, struct mdd_node *mo_run, struct mdd_node *mo_edit, mdd_diff *diff);

static void mdd_free_self_node(struct mdd_node *node)
{
    CHECK_RTN(!node);

    if (is_leaf(node->schema->mtype)) {
        struct mdd_leaf *leaf = (struct mdd_leaf*) node;
        if (((struct mds_leaf*) leaf->schema)->dtype == MDS_DT_STR) {
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

    if (child) {
        mdd_free_data(child);
    }

    if (next) {
        mdd_free_data(next);
    }

    mdd_free_self_node(root);
}

static struct mdd_node* get_last_child(struct mdd_node *node)
{
    struct mdd_node *n = node;
    while (n->next) {
        n = n->next;
    }
    return n;
}

static struct mdd_node* build_container_node(struct mds_node *schema, cJSON *data_json, struct mdd_node *parent)
{
    CHECK_DO_RTN_VAL(!cJSON_IsObject(data_json), LOG_WARN("invalid container data"), NULL);

    LOG_INFO("mdd--try build container or list: %s", schema->name);
    struct mdd_mo *node = (struct mdd_mo*) calloc(1, sizeof(struct mdd_mo));
    node->schema = schema;
    node->parent = parent;

    cJSON *data_child = data_json->child;
    parent = node;
    struct mdd_node *prev = NULL;
    while (data_child) {
        struct mds_node *schema_child = mds_find_child_schema(schema, data_child->string);
        struct mdd_node *node_child = NULL;
        CHECK_DO_GOTO(!schema_child, LOG_WARN("invalid child data name %s under %s", data_child->string, schema->name),
                ERR_OUT);

        node_child = build_mdd_node(schema_child, data_child, parent);
        if (!prev) {
            parent->child = node_child;
        } else {
            prev->next = node_child;
            node_child->prev = prev;
        }
        prev = get_last_child(node_child);

        data_child = data_child->next;
    }
    return node;

ERR_OUT:
    mdd_free_data(node);
    return NULL;
}

static struct mdd_node* build_list_node(struct mds_node *schema, cJSON *data_json, struct mdd_node *parent)
{
    CHECK_DO_RTN_VAL(!cJSON_IsArray(data_json), LOG_WARN("invalid list data: %s", schema->name), NULL);

    LOG_INFO("mdd--try build list: %s-%s", schema->name, data_json->string);
    struct mdd_node *first = NULL;
    struct mdd_node *prev = NULL;
    struct mdd_node *node = NULL;

    cJSON *element = data_json->child;
    while (element) {
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

static struct mdd_node* build_leaf_node(struct mds_node *schema, cJSON *data_json, struct mdd_node *parent)
{
    LOG_INFO("mdd--try build leaf: %s-%s", schema->name, data_json->string);
    struct mds_leaf *leaf_schema = (struct mds_leaf*) schema;
    struct mdd_leaf *leaf = (struct mdd_leaf*) calloc(1, sizeof(struct mdd_leaf));
    CHECK_DO_RTN_VAL(!leaf, LOG_WARN("no memory!"), NULL);

    leaf->schema = schema;
    leaf->parent = parent;
    if (leaf_schema->dtype == MDS_DT_STR) {
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

static struct mdd_node* build_mdd_node(struct mds_node *schema, cJSON *data_json, struct mdd_node *parent)
{
    struct mdd_node *node = NULL;

    if (is_cont_node(schema)) {
        node = build_container_node(schema, data_json, parent);
    } else if (is_list_node(schema)) {
        node = build_list_node(schema, data_json, parent);
    } else if (is_leaf_node(schema)) {
        node = build_leaf_node(schema, data_json, parent);
    }

    return node;
}

struct mdd_node* mdd_parse_json(struct mds_node *schema, const cJSON *data_root)
{
    CHECK_RTN_VAL(!data_root && !data_root->child, NULL);

    return build_container_node(schema, data_root->child, NULL);
}

struct mdd_node* mdd_parse_data(struct mds_node *schema, const char *data_json)
{
    const char *end = NULL;
    struct mdd_node *data_root = NULL;
    cJSON *json_root = cJSON_ParseWithOpts(data_json, &end, cJSON_True);
    CHECK_DO_RTN_VAL(!json_root, LOG_WARN("Failed to parse json %s, error occured at %s", data_json, end), NULL);

    data_root = mdd_parse_json(schema, json_root);

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
    for (int i = 0; i < len; i++) {
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

static char* next_word(char **path_head)
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

    struct mds_leaf *schema = (struct mds_leaf*) node->schema;
    struct mdd_leaf *leaf = (struct mdd_leaf*) node;

    if (schema->dtype == MDS_DT_STR) {
        return strcmp(leaf->value.strv, value) == 0 ? 1 : 0;
    } else if (schema->dtype == MDS_DT_INT) {
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
        for (struct mdd_node *iter = node->child; iter; iter = iter->next) {
            if (!strcmp(iter->schema->name, key) && match_node_value(iter, value)) {
                return 1;
            }
        }
        return 0;
    }
    return 1;
}

static struct mdd_node* find_child(struct mdd_node *cur, char *name, char *key, char *value)
{
    for (struct mdd_node *iter = cur->child; iter; iter = iter->next) {
        if (match_node(iter, name, key, value)) {
            return iter;
        }
    }
    return NULL;
}

struct mdd_node* mdd_get_data(struct mdd_node *root, const char *path)
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
    CHECK_DO_GOTO(!match_node(target, name, key, value),
            LOG_WARN("Failed to match root mo:%s-%s", name, target->schema->name); rlt = -1, CLEAN);
    do {
        has_next = next_fragment(&tmp, &name, &key, &value);
        if (!has_next) {
            out = target;
            goto CLEAN;
        }
        target = find_child(target, name, key, value);
    } while (target);
    rlt = -1;

CLEAN:
    if (dup_path) {
        free(dup_path);
    }
    return out;
}

static int dump_write_str(char **buf, size_t *size, size_t *posi, const char *str)
{
    size_t write_len = strlen(str);
    size_t need_len = *posi + write_len + 1;
    if (need_len > *size) {
        size_t new_size = (*size) * 2;
        new_size = new_size > need_len ? new_size : need_len;
        char *new_buf = realloc(*buf, new_size);
        CHECK_DO_RTN_VAL(!new_buf, LOG_WARN("Failed to realloc memory!"), -1);

        *buf = new_buf;
        *size = new_size;
    }

    strcat(*buf + *posi, str);
    *posi = *posi + write_len;
    return 0;
}

static int dump_node_name(const char *name, char **buf, size_t *size, size_t *posi)
{
    int rlt = dump_write_str(buf, size, posi, "\"");
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump '\"'!"), -1);

    rlt = dump_write_str(buf, size, posi, name);
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump node name!"), -1);

    rlt = dump_write_str(buf, size, posi, "\"");
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump '\"'!"), -1);

    return 0;
}

static int dump_container_body(struct mdd_node *node, char **buf, size_t *size, size_t *posi)
{
    int rlt = dump_write_str(buf, size, posi, "{");
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump '{'"), -1);

    struct mdd_node *n = node->child;
    struct mdd_node *next = NULL;
    while (n) {
        if (n != node->child) {
            rlt = dump_write_str(buf, size, posi, ",");
            CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump ','"), -1);
        }

        rlt = dump_mdd_node(n, buf, size, posi, &next);
        CHECK_RTN_VAL(rlt, -1);

        n = next;
    }

    rlt = dump_write_str(buf, size, posi, "}");
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump '}'"), -1);

    return rlt;
}

static int dump_container_node(struct mdd_node *node, char **buf, size_t *size, size_t *posi, struct mdd_node **next)
{
    int rlt = dump_node_name(node->schema->name, buf, size, posi);
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump node name!"), -1);

    rlt = dump_write_str(buf, size, posi, ":");
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump ':'"), -1);

    rlt = dump_container_body(node, buf, size, posi);
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump container body"), -1);

    *next = node->next;

    return 0;
}

static int dump_list_node(struct mdd_node *node, char **buf, size_t *size, size_t *posi, struct mdd_node **next)
{
    int rlt = dump_node_name(node->schema->name, buf, size, posi);
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump list name!"), -1);

    rlt = dump_write_str(buf, size, posi, ":[");
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump ':['"), -1);

    struct mdd_node *n = node;
    while (n && n->schema == node->schema) {
        if (n != node) {
            rlt = dump_write_str(buf, size, posi, ",");
            CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump ','"), -1);
        }

        rlt = dump_container_body(n, buf, size, posi);
        CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump list body"), -1);

        n = n->next;
    }

    rlt = dump_write_str(buf, size, posi, "]");
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump ']'"), -1);

    *next = n;
    return rlt;
}

static int dump_leaf_node(struct mdd_leaf *leaf, char **buf, size_t *size, size_t *posi, struct mdd_node **next)
{
    int rlt = dump_node_name(leaf->schema->name, buf, size, posi);
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump leaf name!"), -1);

    rlt = dump_write_str(buf, size, posi, ":");
    CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump ':'"), -1);

    if (is_str_leaf((struct mds_leaf* )(leaf->schema))) {
        rlt = dump_write_str(buf, size, posi, "\"");
        CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump '\"'"), -1);

        rlt = dump_write_str(buf, size, posi, leaf->value.strv);
        CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump str value"), -1);

        rlt = dump_write_str(buf, size, posi, "\"");
        CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump '\"'"), -1);
    } else if (is_int_leaf((struct mds_leaf* )(leaf->schema))) {
        char tmp[40];
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%lld", leaf->value.intv);
        rlt = dump_write_str(buf, size, posi, tmp);
        CHECK_DO_RTN_VAL(rlt, LOG_WARN("Failed to dump int value"), -1);
    } else {
        LOG_WARN("Invalid leaf type");
        return -1;
    }

    *next = leaf->next;
    return 0;
}

static int dump_mdd_node(struct mdd_node *node, char **buf, size_t *size, size_t *posi, struct mdd_node **next)
{
    int rlt = 0;
    switch (node->schema->mtype) {
        case MDS_MT_CONTAINER:
            rlt = dump_container_node(node, buf, size, posi, next);
        break;

        case MDS_MT_LEAF:
            rlt = dump_leaf_node((struct mdd_leaf*) node, buf, size, posi, next);
        break;

        case MDS_MT_LIST:
            rlt = dump_list_node(node, buf, size, posi, next);
        break;

        default:
            LOG_WARN("Invalid schema type:%d", node->schema->mtype)
            ;
            return -1;
    }
    return rlt;
}

int mdd_dump_data(struct mdd_node *root, char **json_str)
{
    CHECK_DO_RTN_VAL(!root || !json_str, LOG_WARN("Null arg"), -1);

    size_t size = 100 * 1024;//TODO: only support size under 100K
    size_t posi = 0;
    char *buf = calloc(1, size);
    CHECK_DO_RTN_VAL(!buf, LOG_WARN("No memory"), -1);

    struct mdd_node *cur = root;
    struct mdd_node *next = NULL;
    int rlt = dump_write_str(&buf, &size, &posi, "{");
    CHECK_DO_GOTO(rlt, LOG_WARN("Failed to dump '{'"), CLEAN);

    rlt = dump_mdd_node(cur, &buf, &size, &posi, &next);
    CHECK_DO_GOTO(rlt, LOG_WARN("Failed to dump data tree"), CLEAN);

    dump_write_str(&buf, &size, &posi, "}");
    CHECK_DO_GOTO(rlt, LOG_WARN("Failed to dump '}'"), CLEAN);

    *json_str = buf;
    return 0;

CLEAN:
    free(buf);
    return -1;
}

void mdd_free_diff(mdd_diff *diff)
{
    CHECK_NULL(diff);

    for (size_t i = 0; i < diff->size; i++) {
        struct mdd_mo_diff *modiff = (struct mdd_mo_diff*) (diff->vec[i]);
        vector_free(&modiff->diff_leafs);
    }

    vector_free(diff);
    free(diff);
}

static int is_leaf_equal(struct mdd_leaf *leaf_run, struct mdd_leaf *leaf_edit)
{
    CHECK_NULL_RTN2(leaf_run, leaf_edit, 0);

    if (is_int_leaf((struct mds_leaf* )(leaf_run->schema))) {
        return leaf_run->value.intv == leaf_edit->value.intv;
    } else if (is_str_leaf((struct mds_leaf* )(leaf_run->schema))) {
        return !strcmp(leaf_run->value.strv, leaf_edit->value.strv);
    } else {
        LOG_WARN("leaf type no support yes");
    }
    return 0;
}

//TODO: maybe slow, sort mdd tree by schema will be better
static struct mdd_node* find_child_node(struct mdd_node *mo, struct mds_node *child_schema)
{
    struct mdd_node *child = mo->child;
    while (child) {
        if (child->schema == child_schema) {
            return child;
        }

        child = child->next;
    }
    return NULL;
}

struct mdd_leaf_diff* build_leaf_diff(struct mdd_leaf *leaf_run, struct mdd_leaf *leaf_edit)
{
    struct mdd_leaf_diff *leafdiff = calloc(1, sizeof(struct mdd_leaf_diff));
    CHECK_DO_RTN_VAL(!leafdiff, LOG_WARN("No memory"), NULL);

    leafdiff->edit_leaf = leaf_edit;
    leafdiff->run_leaf = leaf_run;
    return leafdiff;
}

static struct mdd_mo_diff* init_mo_diff_modify(struct mdd_node *mo_run, struct mdd_node *mo_edit,
        struct mdd_leaf_diff *leafdiff)
{
    struct mdd_mo_diff *modiff = calloc(1, sizeof(struct mdd_mo_diff));
    CHECK_DO_RTN_VAL(!modiff, LOG_WARN("No memory"), NULL);

    modiff->type = DF_MODIFY;
    modiff->run_data = mo_run;
    modiff->edit_data = mo_edit;
    int rt = vector_init(&modiff->diff_leafs, (void*) leafdiff);
    if (rt) {
        LOG_WARN("Init diff leaf vec failed");
        free(modiff);
        return NULL;
    }
    return modiff;
}

static int compare_leaf(struct mds_leaf *leaf, struct mdd_node *mo_run, struct mdd_node *mo_edit,
        struct mdd_mo_diff **modiff)
{
    int rt = -1;
    struct mdd_leaf *leaf_run = (struct mdd_leaf*) find_child_node(mo_run, leaf);
    struct mdd_leaf *leaf_edit = (struct mdd_leaf*) find_child_node(mo_edit, leaf);
    CHECK_DO_RTN_VAL(!leaf_run && !leaf_edit, LOG_INFO("leaf %s not exit for both", leaf->name), 0);

    if (!is_leaf_equal(leaf_run, leaf_edit)) {
        struct mdd_leaf_diff *leafdiff = build_leaf_diff(leaf_run, leaf_edit);
        CHECK_DO_RTN_VAL(!leafdiff, LOG_WARN("No memory"), -1);

        if (*modiff == NULL) {
            *modiff = init_mo_diff_modify(mo_run, mo_edit, leafdiff);
            CHECK_DO_RTN_VAL(!modiff, LOG_WARN("Failed to init diff mo");free(leafdiff), -1);
        } else {
            rt = vector_add(&(*modiff)->diff_leafs, (void*) leafdiff);
            CHECK_DO_RTN_VAL(!rt, LOG_WARN("Failed to add diff leaf");free(leafdiff), -1);
        }
    }
    return 0;
}

static struct mdd_mo_diff* build_mo_diff_modify(struct mds_node *mos, struct mdd_node *mo_run, struct mdd_node *mo_edit)
{
    struct mdd_mo_diff *modiff = NULL;
    struct mds_node *child = mos->child;
    while (child) {
        if (is_leaf_node(child)) {
            int rt = compare_leaf((struct mds_leaf*) child, mo_run, mo_edit, &modiff);
            if (rt) {
                if (modiff) {
                    vector_free(&(modiff->diff_leafs));
                    free(modiff);
                }
                return rt;
            }
        }
        child = child->next;
    }
    return modiff;
}

static struct mdd_mo_diff* build_mo_diff_add(struct mdd_node *mo_edit)
{
    struct mdd_mo_diff *modiff = calloc(1, sizeof(struct mdd_mo_diff));
    CHECK_DO_RTN_VAL(!modiff, LOG_WARN("No memory"), NULL);

    modiff->type = DF_ADD;
    modiff->run_data = NULL;
    modiff->edit_data = mo_edit;
    return modiff;
}

static struct mdd_mo_diff* build_mo_diff_del(struct mdd_node *mo_run)
{
    struct mdd_mo_diff *modiff = calloc(1, sizeof(struct mdd_mo_diff));
    CHECK_DO_RTN_VAL(!modiff, LOG_WARN("No memory"), NULL);

    modiff->type = DF_DELETE;
    modiff->run_data = mo_run;
    modiff->edit_data = NULL;
    return modiff;
}

static int compare_self(struct mds_node *mos, struct mdd_node *mo_run, struct mdd_node *mo_edit, mdd_diff *diff)
{
    int rt = -1;
    struct mdd_mo_diff *modiff = NULL;
    if (!mo_run && !mo_edit) {
        return 0;
    } else if (!mo_run) {
        modiff = build_mo_diff_add(mo_edit);
        CHECK_DO_RTN_VAL(!modiff, LOG_WARN("Failed to build add diff for: %s", mos->name), -1);
    } else if (!mo_edit) {
        modiff = build_mo_diff_del(mo_run);
        CHECK_DO_RTN_VAL(!modiff, LOG_WARN("Failed to build del diff for: %s", mos->name), -1);
    } else {
        modiff = build_mo_diff_modify(mos, mo_run, mo_edit);
    }

    if (modiff) {
        rt = vector_add(diff, (void*) modiff);
        CHECK_DO_RTN_VAL(!rt, LOG_WARN("Failed to add modiff"), -1);
    }
    return 0;
}

static int get_list_key(struct mdd_node *list, const char *key)
{
    struct mdd_node *child = list->child;
    while (child) {
        if (!strcmp(child->schema->name, key)) {
            CHECK_DO_RTN_VAL(!is_leaf(child->schema->mtype) || !is_int_leaf((struct mds_leaf*)(child->schema)),
                    LOG_WARN("Invalid key:%s", key), -1);
            return ((struct mdd_leaf*) child)->value.intv;
        }
        child = child->next;
    }
    return -1;
}

static struct mdd_node* find_child_list(struct mdd_node *parent, struct mds_node *lists, int targetKey)
{
    struct mdd_node *list_child = find_child_node(parent, lists);
    while (list_child && list_child->schema == lists) {
        int key = get_list_key(list_child, "Id");
        if (key == targetKey) {
            return list_child;
        }
        list_child = list_child->next;
    }
    return NULL;
}

static int compare_list(struct mds_node *lists, struct mdd_node *mo_run_parent, struct mdd_node *mo_edit_parent,
        mdd_diff *diff)
{
    int rt = -1;
    int key = -1;
    struct mdd_mo_diff *modiff = NULL;
    struct mdd_node *list_run = find_child_node(mo_run_parent, lists);
    while (list_run != NULL && list_run->schema == lists) {
        modiff = NULL;
        key = get_list_key(list_run, "Id");
        CHECK_DO_RTN_VAL(-1 == key, LOG_WARN("Failed to get list key"), -1);

        struct mdd_node *find_edit = find_child_list(mo_edit_parent, lists, key);
        rt = compare_container(lists, list_run, find_edit, diff);
        CHECK_DO_RTN_VAL(rt, LOG_WARN("Failed to add modiff"), -1);

        list_run = list_run->next;
    }

    struct mdd_node *list_edit = find_child_node(mo_edit_parent, lists);
    while (list_edit != NULL && list_edit->schema == lists) {
        key = get_list_key(list_edit, "Id");
        CHECK_DO_RTN_VAL(-1 == key, LOG_WARN("Failed to get list key"), -1);
        LOG_INFO("Find list inst:%s[%d]", list_edit->schema->name, key);

        struct mdd_node *find_run = find_child_list(mo_run_parent, lists, key);
        if (!find_run) {
            LOG_INFO("Find add list inst:%s[%d]", list_edit->schema->name, key);
            rt = compare_container(lists, find_run, list_edit, diff);
            CHECK_DO_RTN_VAL(rt, LOG_WARN("Failed to add modiff"), -1);
        }

        list_edit = list_edit->next;
        if (list_edit) {
            LOG_INFO("Find next list inst:%s[%d]", list_edit->schema->name, key);
        } else {
            LOG_INFO("All list inst over");
        }
    }
    return 0;
}

static int compare_container(struct mds_node *mos, struct mdd_node *mo_run, struct mdd_node *mo_edit, mdd_diff *diff)
{
    int rt = compare_self(mos, mo_run, mo_edit, diff);
    CHECK_DO_RTN_VAL(rt, LOG_WARN("Failed to get diff for mo:%s", mos->name), rt);

    struct mds_node *childs = mos->child;
    while (childs) {
        if (is_cont_node(childs)) {
            struct mdd_node *child_run = find_child_node(mo_run, childs);
            struct mdd_node *child_edit = find_child_node(mo_edit, childs);

            rt = compare_container(childs, child_run, child_edit, diff);
            CHECK_DO_RTN_VAL(rt, LOG_WARN("Failed to get diff for mo:%s", childs->name), rt);
        } else if (is_list_node(childs)) {
            rt = compare_list(childs, mo_run, mo_edit, diff);
            CHECK_DO_RTN_VAL(rt, LOG_WARN("Failed to get diff for list mo:%s", childs->name), rt);
        }

        childs = childs->next;
    }
    return 0;
}

mdd_diff* mdd_get_diff(struct mds_node *schema, struct mdd_node *root_run, struct mdd_node *root_edit)
{
    CHECK_NULL_RTN3(schema, root_run, root_edit, NULL);

    mdd_diff *diff = (mdd_diff*) malloc(sizeof(mdd_diff));
    int rt = vector_init(diff, NULL);
    CHECK_DO_GOTO(rt, LOG_WARN("Failed to init vector"), EXCEPTION);

    struct mds_node *mos = schema;
    if (is_cont_node(mos)) {
        rt = compare_container(mos, root_run, root_edit, diff);
        CHECK_DO_GOTO(rt, LOG_WARN("Failed to compare mo:%s", mos->name), EXCEPTION);
    }

    return diff;

EXCEPTION:
    mdd_free_diff(diff);
    return NULL;
}

static const char* get_diff_mo_name(struct mdd_mo_diff *modiff)
{
    struct mdd_mo *mo = modiff->edit_data ? modiff->edit_data : modiff->run_data;
    return mo->schema->name;
}

static const char* get_diff_type(struct mdd_mo_diff *modiff)
{
    switch (modiff->type) {
        case DF_DELETE:
            return "DELETE";
        case DF_MODIFY:
            return "MODIFY";
        case DF_ADD:
            return "ADD";
        default:
            return "UNKNOWN";
    }
}

void mdd_dump_diff(mdd_diff *diff)
{
    for (size_t i = 0; i < diff->size; i++) {
        struct mdd_mo_diff *modiff = (struct mdd_mo_diff*) diff->vec[i];
        LOG_INFO("modiff:%s - %s", get_diff_type(modiff), get_diff_mo_name(modiff));
    }
}
