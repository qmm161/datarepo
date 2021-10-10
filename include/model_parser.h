#ifndef _MODEL_H_
#define _MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MDS_MT_NULL,
    MDS_MT_CONTAINER,
    MDS_MT_LIST,
    MDS_MT_LEAF
} mds_mtype;

typedef enum
{
    MDS_DT_NULL,
    MDS_DT_INT,
    MDS_DT_STR
} mds_dtype;

struct mds_node
{
    char *name;
    mds_mtype mtype;

    struct mds_node *parent;
    struct mds_node *child;
    struct mds_node *prev;
    struct mds_node *next;
};

struct mds_mo
{
    char *name;
    mds_mtype mtype;

    struct mds_node *parent;
    struct mds_node *child;
    struct mds_node *prev;
    struct mds_node *next;
};

struct mds_leaf
{
    char *name;
    mds_mtype mtype;
    
    struct mds_node *parent;
    struct mds_node *child;
    struct mds_node *prev;
    struct mds_node *next;

    mds_dtype dtype;
};

#define is_mo(mtype) ((mtype)==MDS_MT_CONTAINER || (mtype)==MDS_MT_LIST)
#define is_leaf(mtype) ((mtype)==MDS_MT_LEAF)
#define is_cont_node(schema) ((schema)->mtype==MDS_MT_CONTAINER)
#define is_list_node(schema) ((schema)->mtype==MDS_MT_LIST)
#define is_leaf_node(schema) ((schema)->mtype==MDS_MT_LEAF)

struct mds_node *mdm_load_model(const char *model_str);
void mdm_free_model(struct mds_node *root);
struct mds_node *mdm_find_child_schema(struct mds_node*curr, const char *name);
struct mds_node *mdm_find_next_schema(struct mds_node*curr, const char *name);

#ifdef __cplusplus
}
#endif

#endif
