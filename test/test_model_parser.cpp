#include "gtest/gtest.h"
#include "model_parser.h"
#include "model_test_util.h"

using namespace std;
using namespace testing;

class ModelTest : public Test, public ModelTestUtil
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
    assert_model_mo("Data", root);
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
    assert_model_mo("Data", root);
    assert_model_leaf("Name", MDS_DT_STR, root->child);
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
    assert_model_mo("Data", root);
    assert_model_leaf("Name", MDS_DT_STR, root->child);
    assert_model_leaf("Value", MDS_DT_INT, root->child->next);
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
    assert_model_mo("Data", root);
    assert_model_leaf("Name", MDS_DT_STR, root->child);
    assert_model_leaf("Value", MDS_DT_INT, root->child->next);
    assert_model_list("ChildData", root->child->next->next);
    assert_model_leaf("Id", MDS_DT_INT, root->child->next->next->child);
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
    assert_model_mo("Data", root);
    assert_model_leaf("Name", MDS_DT_STR, root->child);
    assert_model_list("ChildData", root->child->next);
    assert_model_leaf("Id", MDS_DT_INT, root->child->next->child);
    assert_model_leaf("Value", MDS_DT_INT, root->child->next->next);
}