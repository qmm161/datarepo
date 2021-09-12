#include "gtest/gtest.h"
#include "model.h"

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