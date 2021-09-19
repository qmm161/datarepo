#include "gtest/gtest.h"
#include "model.h"

using namespace std;
using namespace testing;

class ModelTest : public Test
{
public:
    void SetUp()
    {
        root = NULL;
    }

    void TearDown()
    {
        mdm_free_model(root);
        root = NULL;
    }

    void assert_mo(const char *name, struct mds_node *node)
    {
        ASSERT_TRUE(NULL != node);
        ASSERT_STREQ(name, node->name);
        ASSERT_EQ(MDS_MT_CONTAINER, node->mtype);
    }

    void assert_leaf(const char *name, mds_dtype dtype, struct mds_node *node)
    {
        ASSERT_TRUE(NULL != node);

        struct mds_leaf *leaf = (struct mds_leaf *)node;

        ASSERT_STREQ(name, leaf->name);
        ASSERT_EQ(MDS_MT_LEAF, leaf->mtype);
        ASSERT_EQ(dtype, leaf->dtype);
    }

    struct mds_node *root;
};

TEST_F(ModelTest, should_load_mo_succ)
{
    const char *VALID_MODEL_JSON = R"({
    "Data": {
        "@attr": {
            "mtype": "container"
        }
    }
})";

    root = mdm_load_model(VALID_MODEL_JSON);
    assert_mo("Data", root);
}

TEST_F(ModelTest, should_load_mo_and_leaf_succ)
{
    const char *VALID_MODEL_JSON = R"({
    "Data": {
        "@attr": {
            "mtype": "container"
        },
        "Name": {
            "@attr": {
                "mtype": "leaf",
                "dtype": "string"
            }
        }
    }
})";

    root = mdm_load_model(VALID_MODEL_JSON);
    assert_mo("Data", root);

    struct mds_node *name = root->child;
    assert_leaf("Name", MDS_DT_STR, name);
}