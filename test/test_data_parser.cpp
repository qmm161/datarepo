#include "gtest/gtest.h"

extern "C" {
#include "data_parser.h"
#include "model_test_util.h"
}

using namespace testing;

const char *TEST_MODEL_JSON =
        R"({
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
            "Value": {
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

class DataParser: public Test, public ModelTestUtil
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
    assert_data_container("Data", data);
    assert_data_string_leaf("Name", "vc1000", data->child);
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
    assert_data_container("Data", data);
    assert_data_string_leaf("Name", "vc1000", data->child);
    assert_data_int_leaf("Value", 100, data->child->next);
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
    assert_data_container("Data", data);
    assert_data_string_leaf("Name", "vc1000", data->child);
    assert_data_int_leaf("Value", 100, data->child->next);
    assert_data_container("ChildData", data->child->next->next);
    assert_data_int_leaf("Id", 100, data->child->next->next->child);
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
    assert_data_container("Data", data);
    assert_data_container("ChildData", data->child);
    assert_data_int_leaf("Id", 100, data->child->child);
    assert_data_int_leaf("Value", 100, data->child->next);
    assert_data_string_leaf("Name", "vc1000", data->child->next->next);
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
    assert_data_container("Data", data);
    assert_data_string_leaf("Name", "vc1000", data->child);
    assert_data_list("ChildList", data->child->next);
    assert_data_list("ChildList", data->child->next->next);
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
    assert_data_container("Data", data);
    assert_data_string_leaf("Name", "vc1000", data->child);
    assert_data_list("ChildList", data->child->next);
    assert_data_int_leaf("Id", 1, data->child->next->child);
    assert_data_list("SubChildList", data->child->next->child->next);
    assert_data_int_leaf("Id", 1, data->child->next->child->next->child);
    assert_data_int_leaf("IntLeaf", 100, data->child->next->child->next->child->next);
    assert_data_list("SubChildList", data->child->next->child->next->next);
    assert_data_list("ChildList", data->child->next->next);
    assert_data_int_leaf("Id", 2, data->child->next->next->child);
}

TEST_F(DataParser, test_should_build_multi_layer_list_2)
{
    const char *TEST_DATA_JSON = R"({
        "Data": {
            "Value": 100,
            "Name": "vc1000",
            "ChildList": [
                {
                    "Id": 1, 
                    "SubChildList":[
                        {"Id": 2, "IntLeaf":220},
                        {"Id": 3, "IntLeaf":300}
                    ],
                    "Value":1
                }
            ]
        }
    })";
    data = mdd_parse_data(schema, TEST_DATA_JSON);
    assert_data_container("Data", data);
    assert_data_int_leaf("Value", 100, data->child);
    assert_data_string_leaf("Name", "vc1000", data->child->next);
    assert_data_list("ChildList", data->child->next->next);
    assert_data_int_leaf("Id", 1, data->child->next->next->child);
    assert_data_list("SubChildList", data->child->next->next->child->next);
    assert_data_int_leaf("Id", 2, data->child->next->next->child->next->child);
    assert_data_int_leaf("IntLeaf", 220, data->child->next->next->child->next->child->next);
    assert_data_list("SubChildList", data->child->next->next->child->next->next);
    assert_data_int_leaf("Id", 3, data->child->next->next->child->next->next->child);
    assert_data_int_leaf("IntLeaf", 300, data->child->next->next->child->next->next->child->next);
    assert_data_int_leaf("Value", 1, data->child->next->next->child->next->next->next);
}

TEST_F(DataParser, test_should_dump_root_container_as_json)
{
    const char *TEST_DATA_JSON = R"({
        "Data": {
            "Name": "vc1000"
        }
    })";
    data = mdd_parse_data(schema, TEST_DATA_JSON);
    assert_data_container("Data", data);

    char *dump = NULL;
    int rlt = mdd_dump_data(data, &dump);
    ASSERT_EQ(0, rlt);

    printf("dump rlt:%s\n", dump);

    assert_equal_json(dump, TEST_DATA_JSON);
    free(dump);
}

TEST_F(DataParser, test_should_dump_multi_layer_mo)
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
    assert_data_container("Data", data);

    char *dump = NULL;
    int rlt = mdd_dump_data(data, &dump);
    ASSERT_EQ(0, rlt);

    printf("dump rlt:%s\n", dump);

    assert_equal_json(dump, TEST_DATA_JSON);
    free(dump);
}

TEST_F(DataParser, test_should_dump_list)
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
    assert_data_container("Data", data);

    char *dump = NULL;
    int rlt = mdd_dump_data(data, &dump);
    ASSERT_EQ(0, rlt);

    printf("dump rlt:%s\n", dump);

    assert_equal_json(dump, TEST_DATA_JSON);
    free(dump);
}

TEST_F(DataParser, test_should_dump_multi_layer_list)
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
    assert_data_container("Data", data);

    char *dump = NULL;
    int rlt = mdd_dump_data(data, &dump);
    ASSERT_EQ(0, rlt);

    printf("dump rlt:%s\n", dump);

    assert_equal_json(dump, TEST_DATA_JSON);
    free(dump);
}

TEST_F(DataParser, test_should_get_root_container_diff)
{
    const char *TEST_DATA_JSON_1 = R"({
        "Data": {
            "Name": "vc1000",
            "Value": 100
        }
    })";
    struct mdd_node *data1 = mdd_parse_data(schema, TEST_DATA_JSON_1);

    const char *TEST_DATA_JSON_2 = R"({
        "Data": {
            "Name": "vc1000",
            "Value": 200
        }
    })";
    struct mdd_node *data2 = mdd_parse_data(schema, TEST_DATA_JSON_2);

    mdd_diff *diff = mdd_get_diff(schema, data1, data2);
    ASSERT_TRUE(NULL != diff);
    ASSERT_EQ(1, diff->size);

    struct mdd_mo_diff* modiff = (struct mdd_mo_diff*)diff->vec[0];
    ASSERT_EQ(DF_MODIFY, modiff->type);
    ASSERT_EQ(1, modiff->diff_leafs.size);

    mdd_free_diff(diff);

    mdd_free_data(data1);
    mdd_free_data(data2);
}

TEST_F(DataParser, test_should_get_root_container_diff_2_leaf)
{
    const char *TEST_DATA_JSON_1 = R"({
        "Data": {
            "Name": "vc1000",
            "Value": 100
        }
    })";
    struct mdd_node *data1 = mdd_parse_data(schema, TEST_DATA_JSON_1);

    const char *TEST_DATA_JSON_2 = R"({
        "Data": {
            "Name": "vc2000",
            "Value": 200
        }
    })";
    struct mdd_node *data2 = mdd_parse_data(schema, TEST_DATA_JSON_2);

    mdd_diff *diff = mdd_get_diff(schema, data1, data2);
    ASSERT_TRUE(NULL != diff);
    ASSERT_EQ(1, diff->size);

    struct mdd_mo_diff* modiff = (struct mdd_mo_diff*)diff->vec[0];
    ASSERT_EQ(DF_MODIFY, modiff->type);
    ASSERT_EQ(2, modiff->diff_leafs.size);

    mdd_free_diff(diff);

    mdd_free_data(data1);
    mdd_free_data(data2);
}

TEST_F(DataParser, test_should_get_multi_layer_diff)
{
    const char *TEST_DATA_JSON_1 = R"({
        "Data": {
            "Value": 100,
            "Name": "vc1000"
        }
    })";
    struct mdd_node *data1 = mdd_parse_data(schema, TEST_DATA_JSON_1);

    const char *TEST_DATA_JSON_2 = R"({
        "Data": {
            "Value": 200,
            "ChildData": {
                "Id":200
            },
            "Name": "vc1200"
        }
    })";
    struct mdd_node *data2 = mdd_parse_data(schema, TEST_DATA_JSON_2);

    mdd_diff *diff = mdd_get_diff(schema, data1, data2);
    ASSERT_TRUE(NULL != diff);
    ASSERT_EQ(2, diff->size);

    struct mdd_mo_diff* modiff = (struct mdd_mo_diff*)diff->vec[0];
    ASSERT_EQ(DF_MODIFY, modiff->type);
    ASSERT_EQ(2, modiff->diff_leafs.size);

    modiff = (struct mdd_mo_diff*)diff->vec[1];
    ASSERT_EQ(DF_ADD, modiff->type);
    ASSERT_EQ(0, modiff->diff_leafs.size);

    mdd_free_diff(diff);

    mdd_free_data(data1);
    mdd_free_data(data2);
}

TEST_F(DataParser, test_should_get_multi_layer_diff2)
{
    const char *TEST_DATA_JSON_1 = R"({
        "Data": {
            "Value": 100,
            "Name": "vc1000",
            "ChildData": {
                "Id":100
            }
        }
    })";
    struct mdd_node *data1 = mdd_parse_data(schema, TEST_DATA_JSON_1);

    const char *TEST_DATA_JSON_2 = R"({
        "Data": {
            "Name": "vc1000"
        }
    })";
    struct mdd_node *data2 = mdd_parse_data(schema, TEST_DATA_JSON_2);

    mdd_diff *diff = mdd_get_diff(schema, data1, data2);
    ASSERT_TRUE(NULL != diff);
    ASSERT_EQ(2, diff->size);

    struct mdd_mo_diff* modiff = (struct mdd_mo_diff*)diff->vec[0];
    ASSERT_EQ(DF_MODIFY, modiff->type);
    ASSERT_EQ(1, modiff->diff_leafs.size);

    modiff = (struct mdd_mo_diff*)diff->vec[1];
    ASSERT_EQ(DF_DELETE, modiff->type);
    ASSERT_EQ(0, modiff->diff_leafs.size);

    mdd_free_diff(diff);

    mdd_free_data(data1);
    mdd_free_data(data2);
}

TEST_F(DataParser, test_should_get_multi_layer_diff_with_list)
{
    const char *TEST_DATA_JSON_1 = R"({
        "Data": {
            "Value": 100,
            "Name": "vc1000",
            "ChildList": [
                {
                    "Id": 1, 
                    "Value":1,
                    "SubChildList":[
                        {"Id": 1, "IntLeaf":100},
                        {"Id": 2, "IntLeaf":200}
                    ]
                },
                {"Id": 2}
            ]
        }
    })";
    struct mdd_node *data1 = mdd_parse_data(schema, TEST_DATA_JSON_1);
    ASSERT_TRUE(NULL != data1);

    printf("begin parse test data2\n");

    const char *TEST_DATA_JSON_2 = R"({
        "Data": {
            "Value": 100,
            "Name": "vc1000",
            "ChildList": [
                {
                    "Id": 1, 
                    "SubChildList":[
                        {"Id": 2, "IntLeaf":220},
                        {"Id": 3, "IntLeaf":300}
                    ],
                    "Value":1
                },
                {
                    "Id": 2,
                    "Value":2,
                    "SubChildList":[
                        {"Id": 1, "IntLeaf":100},
                        {"Id": 2, "IntLeaf":200}
                    ]
                }
            ]
        }
    })";
    struct mdd_node *data2 = mdd_parse_data(schema, TEST_DATA_JSON_2);
    ASSERT_TRUE(NULL != data2);

    mdd_diff *diff = mdd_get_diff(schema, data1, data2);
    ASSERT_TRUE(NULL != diff);
    mdd_dump_diff(diff);
    ASSERT_EQ(6, diff->size);

    mdd_free_diff(diff);

    mdd_free_data(data1);
    mdd_free_data(data2);
}

TEST_F(DataParser, test_should_get_root_container_diff_when_edit_json)
{
    const char *TEST_DATA_JSON_1 = R"({
        "msg_id" : "1",
        "msg_name" : "Set",
        "msg_body" : {
            "Data": {
                "Name": "vc1000",
                "Value": 100
            }
        }
    })";
    cJSON *request = cJSON_Parse(TEST_DATA_JSON_1);
    cJSON *child = request->child;
    cJSON *jsondata1 = NULL;
    while (child) {
        if (!strcmp(child->string, "msg_body")) {
            jsondata1 = child;
            cJSON_DetachItemViaPointer(request, child);
            break;
        }
        child = child->next;
    }
    cJSON_Delete(request);

    struct mdd_node *data1 = mdd_parse_json(schema, jsondata1);

    const char *TEST_DATA_JSON_2 = R"({
        "Data": {
            "Name": "vc1000",
            "Value": 200
        }
    })";
    struct mdd_node *data2 = mdd_parse_data(schema, TEST_DATA_JSON_2);

    mdd_diff *diff = mdd_get_diff(schema, data1, data2);
    ASSERT_TRUE(NULL != diff);
    ASSERT_EQ(1, diff->size);

    struct mdd_mo_diff* modiff = (struct mdd_mo_diff*)diff->vec[0];
    ASSERT_EQ(DF_MODIFY, modiff->type);
    ASSERT_EQ(1, modiff->diff_leafs.size);

    mdd_free_diff(diff);

    mdd_free_data(data1);
    mdd_free_data(data2);
}
