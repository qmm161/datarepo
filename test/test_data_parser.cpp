#include "gtest/gtest.h"
#include "data_parser.h"

using namespace std;
using namespace testing;

const char *TEST_MODEL_JSON = R"({
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

class DataParser : public Test
{
public:
    void SetUp()
    {
        schema = mdm_load_model(TEST_MODEL_JSON);
        data = NULL;
    }

    void TearDown()
    {
        mdd_free_data(data);
        data = NULL;

        mdm_free_model(schema);
        schema = NULL;
    }

    void assert_mo(const char *name, struct mdd_node *node)
    {
        ASSERT_EQ(MDS_MT_CONTAINER, node->schema->mtype);
        ASSERT_STREQ(name, node->schema->name);
    }

    void assert_string_leaf(const char *name, const char *value, struct mdd_node *node)
    {
        ASSERT_EQ(MDS_MT_LEAF, node->schema->mtype);
        ASSERT_EQ(MDS_DT_STR, ((struct mds_leaf*)node->schema)->dtype);
        ASSERT_STREQ(name, node->schema->name);

        struct mdd_leaf *leaf = (struct mdd_leaf*)node;
        ASSERT_STREQ(value, leaf->value.strv);
    }

    struct mds_node *schema;
    struct mdd_node *data;
};

TEST_F(DataParser, test_should_build_single_mo)
{
    const char *TEST_DATA_JSON = R"({
        "Data": {
            "Name": "vc1000"
        }
    })";
    data = mdd_parse_data(schema, TEST_DATA_JSON);
    assert_mo("Data", data);
    assert_string_leaf("Name", "vc1000", data->child);
}