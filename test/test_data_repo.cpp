#include <gtest/gtest.h>

extern "C" {
#include "model_test_util.h"
#include "data_repo.h"
}

using namespace std;
using namespace testing;

class DataRepoTest: public ModelTestUtil, public Test
{
public:
    void SetUp()
    {
        int rlt = repo_init("../test/testdata/testmodel.json", "../test/testdata/testdata.json");
        ASSERT_EQ(0, rlt);
    }

    void TearDown()
    {
        repo_free();
    }
};

TEST_F(DataRepoTest, should_get_top_node_succ)
{
    struct mdd_node *out = NULL;
    int rlt = repo_get("Data", &out);
    ASSERT_EQ(0, rlt);
    assert_data_container("Data", out);
}

TEST_F(DataRepoTest, should_get_child_leaf_node_succ)
{
    struct mdd_node *out = NULL;
    int rlt = repo_get("Data/Name", &out);
    ASSERT_EQ(0, rlt);
    assert_data_string_leaf("Name", "TestData", out);
}

TEST_F(DataRepoTest, should_get_child_leaf2_node_succ)
{
    struct mdd_node *out = NULL;
    int rlt = repo_get("Data/Value", &out);
    ASSERT_EQ(0, rlt);
    assert_data_int_leaf("Value", 100, out);
}

TEST_F(DataRepoTest, should_get_child_mo_node_succ)
{
    struct mdd_node *out = NULL;
    int rlt = repo_get("Data/ChildData/IntLeaf", &out);
    ASSERT_EQ(0, rlt);
    assert_data_int_leaf("IntLeaf", 100, out);
}

TEST_F(DataRepoTest, should_get_child_list_node_by_key_succ)
{
    struct mdd_node *out = NULL;
    int rlt = repo_get("Data/ChildList[Id=1]/IntLeaf", &out);
    ASSERT_EQ(0, rlt);
    assert_data_int_leaf("IntLeaf", 1, out);
}

TEST_F(DataRepoTest, should_get_child_list_node_2_by_key_succ)
{
    struct mdd_node *out = NULL;
    int rlt = repo_get("Data/ChildList[Id=22]/IntLeaf", &out);
    ASSERT_EQ(0, rlt);
    assert_data_int_leaf("IntLeaf", 22, out);
}

TEST_F(DataRepoTest, should_get_child_list_child_list_by_key_succ)
{
    struct mdd_node *out = NULL;
    int rlt = repo_get("Data/ChildList[Id=22]/SubChildList[Id=222]/StrLeaf", &out);
    ASSERT_EQ(0, rlt);
    assert_data_string_leaf("StrLeaf", "222", out);
}

