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
        mdm_free_model(schema);
        schema = NULL;

        mdm_free_data(data);
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
    data = mdm_parse_data(schema, TEST_DATA_JSON);
}