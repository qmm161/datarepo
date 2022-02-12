#include <gtest/gtest.h>
#include <string>
#include "common.h"

using namespace std;
using namespace testing;

class CommonTest: public Test
{
public:
    CommonTest() = default;
    ~CommonTest() = default;

    struct TestData
    {
        int d;
    };

    string b;
};

TEST_F(CommonTest, should_add_element_to_vec_and_do_loop_and_free)
{
    struct mdd_vector dvec;
    struct TestData t1 = {1};
    struct TestData t2 = {2};
    struct TestData t3 = {3};
    struct TestData t4 = {4};
    struct TestData t5 = {5};
    struct TestData t6 = {6};
    struct TestData t7 = {7};
    struct TestData t8 = {8};
    struct TestData t9 = {9};
    struct TestData t10 = {10};
    struct TestData t11 = {11};
    struct TestData t12 = {12};

    int rt = vector_init(&dvec, (void*)&t1);
    ASSERT_EQ(0, rt);
    ASSERT_EQ(1, dvec.size);
    ASSERT_EQ(10, dvec.capacity);

    vector_add(&dvec, (void*)&t2);
    vector_add(&dvec, (void*)&t3);
    vector_add(&dvec, (void*)&t4);
    vector_add(&dvec, (void*)&t5);
    vector_add(&dvec, (void*)&t6);
    vector_add(&dvec, (void*)&t7);
    vector_add(&dvec, (void*)&t8);
    vector_add(&dvec, (void*)&t9);
    vector_add(&dvec, (void*)&t10);
    vector_add(&dvec, (void*)&t11);
    vector_add(&dvec, (void*)&t12);

    ASSERT_EQ(12, dvec.size);
    ASSERT_EQ(20, dvec.capacity);

    ASSERT_EQ(11, ((struct TestData*)(dvec.vec[10]))->d);

    vector_free(&dvec);
}