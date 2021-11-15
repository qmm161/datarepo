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
                "mtype": "container"
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
        },
        "ChildList":{
            "@attr": {
                "mtype": "list"
            },
            "Id": {
                "@attr": {
                    "mtype": "leaf",
                    "dtype": "int"
                }
            },
            "SubChildList": {
                "@attr": {
                    "mtype": "list"
                },
                "Id": {
                    "@attr": {
                        "mtype": "leaf",
                        "dtype": "int"
                    }
                },   
                "IntLeaf": {
                    "@attr": {
                        "mtype": "leaf",
                        "dtype": "int"
                    }
                }       
            }                         
        }
    }
})";

class DataParser : public Test
{
public:
    void SetUp()
    {
        schema = mds_load_model(TEST_MODEL_JSON);
        data = NULL;
    }

    void TearDown()
    {
        mdd_free_data(data);
        data = NULL;

        mds_free_model(schema);
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

    void assert_int_leaf(const char *name, long long value, struct mdd_node *node)
    {
        ASSERT_EQ(MDS_MT_LEAF, node->schema->mtype);
        ASSERT_EQ(MDS_DT_INT, ((struct mds_leaf*)node->schema)->dtype);
        ASSERT_STREQ(name, node->schema->name);

        struct mdd_leaf *leaf = (struct mdd_leaf*)node;
        ASSERT_EQ(value, leaf->value.intv);
    }

    void assert_container(const char *name, struct mdd_node *node)
    {
        ASSERT_EQ(MDS_MT_CONTAINER, node->schema->mtype);
        ASSERT_STREQ(name, node->schema->name);        
    }

    void assert_list(const char *name, struct mdd_node *node)
    {
        ASSERT_EQ(MDS_MT_LIST, node->schema->mtype);
        ASSERT_STREQ(name, node->schema->name);               
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

TEST_F(DataParser, test_should_build_single_mo_with_multi_leaf)
{
    const char *TEST_DATA_JSON = R"({
        "Data": {
            "Name": "vc1000",
            "Value": 100
        }
    })";
    data = mdd_parse_data(schema, TEST_DATA_JSON);
    assert_mo("Data", data);
    assert_string_leaf("Name", "vc1000", data->child);
    assert_int_leaf("Value", 100, data->child->next);
}

TEST_F(DataParser, test_should_build_multi_layer_mo)
{
    const char *TEST_DATA_JSON = R"({
        "Data": {
            "Name": "vc1000",
            "Value": 100,
            "ChildData": {
                "Id":100
            }
        }
    })";
    data = mdd_parse_data(schema, TEST_DATA_JSON);
    assert_mo("Data", data);
    assert_string_leaf("Name", "vc1000", data->child);
    assert_int_leaf("Value", 100, data->child->next);
    assert_container("ChildData", data->child->next->next);
    assert_int_leaf("Id", 100, data->child->next->next->child);
}

TEST_F(DataParser, test_should_build_multi_layer_mo_disorder)
{
    const char *TEST_DATA_JSON = R"({
        "Data": {
            "ChildData": {
                "Id":100
            },
            "Value": 100,
            "Name": "vc1000"
        }
    })";
    data = mdd_parse_data(schema, TEST_DATA_JSON);
    assert_mo("Data", data);
    assert_container("ChildData", data->child);
    assert_int_leaf("Id", 100, data->child->child);
    assert_int_leaf("Value", 100, data->child->next);    
    assert_string_leaf("Name", "vc1000", data->child->next->next);
}

TEST_F(DataParser, test_should_build_list)
{
    const char *TEST_DATA_JSON = R"({
        "Data": {
            "Name": "vc1000",
            "ChildList": [
                {"Id": 1},
                {"Id": 2}
            ]
        }
    })";
    data = mdd_parse_data(schema, TEST_DATA_JSON);
    assert_mo("Data", data);
    assert_string_leaf("Name", "vc1000", data->child);
    assert_list("ChildList", data->child->next);
    assert_list("ChildList", data->child->next->next);
}

TEST_F(DataParser, test_should_build_multi_layer_list)
{
    const char *TEST_DATA_JSON = R"({
        "Data": {
            "Name": "vc1000",
            "ChildList": [
                {
                    "Id": 1, 
                    "SubChildList":[
                        {"Id": 1, "IntLeaf":100},
                        {"Id": 2, "IntLeaf":200}
                    ]
                },
                {"Id": 2}
            ]
        }
    })";
    data = mdd_parse_data(schema, TEST_DATA_JSON);
    assert_mo("Data", data);
    assert_string_leaf("Name", "vc1000", data->child);
    assert_list("ChildList", data->child->next);
    assert_int_leaf("Id", 1, data->child->next->child);
    assert_list("SubChildList", data->child->next->child->next);
    assert_int_leaf("Id", 1, data->child->next->child->next->child);
    assert_int_leaf("IntLeaf", 100, data->child->next->child->next->child->next);
    assert_list("SubChildList", data->child->next->child->next->next);
    assert_list("ChildList", data->child->next->next);
    assert_int_leaf("Id", 2, data->child->next->next->child);
}