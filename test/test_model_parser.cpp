#include "gtest/gtest.h"
#include "model_parser.h"

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
        mds_free_model(root);
        root = NULL;
    }

    void assert_mo(const char *name, struct mds_node *node)
    {
        ASSERT_TRUE(NULL != node);
        ASSERT_STREQ(name, node->name);
        ASSERT_EQ(MDS_MT_CONTAINER, node->mtype);
    }

    void assert_list(const char *name, struct mds_node *node)
    {
        ASSERT_TRUE(NULL != node);
        ASSERT_STREQ(name, node->name);
        ASSERT_EQ(MDS_MT_LIST, node->mtype);
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

    root = mds_load_model(VALID_MODEL_JSON);
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

    root = mds_load_model(VALID_MODEL_JSON);
    assert_mo("Data", root);
    assert_leaf("Name", MDS_DT_STR, root->child);
}

TEST_F(ModelTest, should_load_mo_and_multi_leaf_succ)
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
        },
        "Value": {
            "@attr": {
                "mtype": "leaf",
                "dtype": "int"
            }
        }
    }
})";

    root = mds_load_model(VALID_MODEL_JSON);
    assert_mo("Data", root);
    assert_leaf("Name", MDS_DT_STR, root->child);
    assert_leaf("Value", MDS_DT_INT, root->child->next);
    ASSERT_TRUE(NULL == root->child->next->next);
}

TEST_F(ModelTest, should_load_multi_mo_and_multi_leaf_succ)
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
        },
        "Value": {
            "@attr": {
                "mtype": "leaf",
                "dtype": "int"
            }
        },
        "ChildData": {
            "@attr": {
                "mtype": "list"
            },
            "Id": {
                "@attr": {
                    "mtype": "leaf",
                    "dtype": "int"
                }
            }
        }
    }
})";

    root = mds_load_model(VALID_MODEL_JSON);
    assert_mo("Data", root);
    assert_leaf("Name", MDS_DT_STR, root->child);
    assert_leaf("Value", MDS_DT_INT, root->child->next);
    assert_list("ChildData", root->child->next->next);
    assert_leaf("Id", MDS_DT_INT, root->child->next->next->child);
}


TEST_F(ModelTest, should_load_complicated_schema_succ)
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
        },
        "ChildData": {
            "@attr": {
                "mtype": "list"
            },
            "Id": {
                "@attr": {
                    "mtype": "leaf",
                    "dtype": "int"
                }
            }
        },
        "Value": {
            "@attr": {
                "mtype": "leaf",
                "dtype": "int"
            }
        }
    }
})";

    root = mds_load_model(VALID_MODEL_JSON);
    assert_mo("Data", root);
    assert_leaf("Name", MDS_DT_STR, root->child);
    assert_list("ChildData", root->child->next);
    assert_leaf("Id", MDS_DT_INT, root->child->next->child);
    assert_leaf("Value", MDS_DT_INT, root->child->next->next);
}