#include <gtest/gtest.h>
#include <map>
#include <string>
#include <math.h>
#include "cjson/cJSON.h"

extern "C" {
#include "macro.h"
}

using namespace std;
using namespace testing;

class JsonDiffTest : public Test{
public:
    void SetUp()
    {
        cj1 = NULL;
        cj2 = NULL;
    }

    void TearDown()
    {
        cJSON_Delete(cj1);
        cJSON_Delete(cj2);
    }

    bool compareJsonObj(cJSON *obj1, cJSON *obj2)
    {
        CHECK_RTN_VAL(!obj1 || !obj2, false);
        CHECK_RTN_VAL(!cJSON_IsObject(obj1) || !cJSON_IsObject(obj2), false);

        map<string, cJSON*> nodes1;
        map<string, cJSON*> nodes2;
        cacheAllChildNode(obj1, nodes1);
        cacheAllChildNode(obj2, nodes2);
        CHECK_RTN_VAL(nodes1.size() != nodes2.size(), false);

        for(auto ele1 : nodes1){
            auto it2 = nodes2.find(ele1.first);
            CHECK_RTN_VAL(it2 == nodes2.end(), false);
            CHECK_RTN_VAL(ele1.second->type != it2->second->type, false);

            CHECK_RTN_VAL(!compareJsonNode(ele1.second, it2->second), false);
        }
        return true;
    }

    void cacheAllChildNode(cJSON *obj, map<string, cJSON*> &nodes)
    {
        for(cJSON *it = obj->child; it != NULL; it = it->next){
            nodes[it->string] = it;
        }
    }

    bool compareJsonArray(cJSON *arr1, cJSON *arr2)
    {
        int cnt1 = cJSON_GetArraySize(arr1);
        int cnt2 = cJSON_GetArraySize(arr2);
        CHECK_RTN_VAL(cnt1 != cnt2, false);
        CHECK_RTN_VAL(cnt1 == 0, true);

        cJSON *ele1 = cJSON_GetArrayItem(arr1, 0);
        LOG_INFO("element type:%d", ele1->type);
        CHECK_RTN_VAL(ele1->type == cJSON_Array, false);

        cJSON *a = arr1->child;
        cJSON *b = arr2->child;
        if(ele1->type != cJSON_Object){
            for(; a != NULL && b != NULL; a = a->next, b = b->next){
                CHECK_RTN_VAL(!compareJsonNode(a, b), false);
            }
        } else {
            for(; a != NULL; a = a->next) {
                CHECK_RTN_VAL(!matchAny(a, b), false);
            }
        }

        return true;
    }

    bool compareJsonNode(cJSON *node1, cJSON *node2)
    {
        CHECK_RTN_VAL(node1->type != node2->type, false);

        switch (node1->type) {
        case cJSON_Number:
            CHECK_RTN_VAL((node1->valueint != node2->valueint) || (!compare_double(node1->valuedouble, node2->valuedouble)), false)
            break;

        case cJSON_String:
            CHECK_RTN_VAL(strcmp(node1->valuestring, node2->valuestring), false)
            break;

        case cJSON_True:
        case cJSON_False:
            break;

        case cJSON_Object:
            CHECK_RTN_VAL(!compareJsonObj(node1, node2), false)
            break;

        case cJSON_Array:
            CHECK_RTN_VAL(!compareJsonArray(node1, node2), false)
            break;

        default:
            LOG_WARN("type not supported yet: %d", node1->type)
            return false;
        }
        return true;
    }

    bool matchAny(cJSON *target, cJSON *array)
    {
        for(cJSON *ele = array; ele != NULL; ele = ele->next) {
            if(compareJsonNode(target, ele)) {
                return true;
            }
        }
        return false;
    }

    bool compare_double(double a, double b)
    {
        double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
        return (fabs(a - b) <= maxVal * DBL_EPSILON);
    }

    cJSON *cj1;
    cJSON *cj2;
};

TEST_F(JsonDiffTest, should_eq_when_all_node_eq)
{
    cj1 = cJSON_Parse(R"({"a":1, "b":"2"})");
    cj2 = cJSON_Parse(R"({"b":"2", "a":1})");

    ASSERT_TRUE(compareJsonObj(cj1, cj2));
}

TEST_F(JsonDiffTest, should_neq_when_any_node_neq)
{
    cj1 = cJSON_Parse(R"({"a":1, "b":"2"})");
    cj2 = cJSON_Parse(R"({"b":"2", "a":2})");

    ASSERT_FALSE(compareJsonObj(cj1, cj2));
}

TEST_F(JsonDiffTest, should_neq_when_node_cnt_neq)
{
    cj1 = cJSON_Parse(R"({"a":1, "b":"2"})");
    cj2 = cJSON_Parse(R"({"b":"2", "a":1, "c":1})");

    ASSERT_FALSE(compareJsonObj(cj1, cj2));
}

TEST_F(JsonDiffTest, should_neq_when_node_type_neq)
{
    cj1 = cJSON_Parse(R"({"a":1, "b":"2"})");
    cj2 = cJSON_Parse(R"({"b":"2", "a":"1"})");

    ASSERT_FALSE(compareJsonObj(cj1, cj2));
}

TEST_F(JsonDiffTest, should_eq_when_all_base_type_eq)
{
    cj1 = cJSON_Parse(R"({"a":1, "b":"2", "c":true, "d":false})");
    cj2 = cJSON_Parse(R"({"b":"2", "a":1, "d":false, "c":true})");

    ASSERT_TRUE(compareJsonObj(cj1, cj2));
}

TEST_F(JsonDiffTest, should_eq_when_nest_obj_eq)
{
    cj1 = cJSON_Parse(R"({"a":1, "b":"2", "c":{"c1":1, "c2":"2"}})");
    cj2 = cJSON_Parse(R"({"b":"2", "a":1, "c":{"c2":"2", "c1":1}})");

    ASSERT_TRUE(compareJsonObj(cj1, cj2));
}

TEST_F(JsonDiffTest, should_neq_when_nest_obj_neq)
{
    cj1 = cJSON_Parse(R"({"a":1, "b":"2", "c":{"c1":true, "c2":"2"}})");
    cj2 = cJSON_Parse(R"({"b":"2", "a":1, "c":{"c2":"2", "c1":1}})");

    ASSERT_FALSE(compareJsonObj(cj1, cj2));
}

TEST_F(JsonDiffTest, should_eq_when_nest_array_eq)
{
    cj1 = cJSON_Parse(R"({"a":1, "b":"2", "c":[1,2,3]})");
    cj2 = cJSON_Parse(R"({"b":"2", "a":1, "c":[1,2,3]})");

    ASSERT_TRUE(compareJsonObj(cj1, cj2));
}

TEST_F(JsonDiffTest, should_neq_when_nest_array_neq)
{
    cj1 = cJSON_Parse(R"({"a":1, "b":"2", "c":[1,2,3]})");
    cj2 = cJSON_Parse(R"({"b":"2", "a":1, "c":[3,2,1]})");

    ASSERT_FALSE(compareJsonObj(cj1, cj2));
}

TEST_F(JsonDiffTest, should_eq_when_nest_object_array_eq)
{
    cj1 = cJSON_Parse(R"({"a":1, "b":"2", "c":[{"m":"m1", "n":"n1"},{"m":"m2", "n":"n2"}]})");
    cj2 = cJSON_Parse(R"({"b":"2", "a":1, "c":[{"n":"n2", "m":"m2"},{"n":"n1", "m":"m1"}]})");

    ASSERT_TRUE(compareJsonObj(cj1, cj2));
}

TEST_F(JsonDiffTest, should_neq_when_nest_object_array_neq)
{
    cj1 = cJSON_Parse(R"({"a":1, "b":"2", "c":[{"m":"m1", "n":"n1"},{"m":"m2", "n":"n2"}]})");
    cj2 = cJSON_Parse(R"({"b":"2", "a":1, "c":[{"n":"n2", "m":"m2"},{"n":"n0", "m":"m1"}]})");

    ASSERT_FALSE(compareJsonObj(cj1, cj2));
}
