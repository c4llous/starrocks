// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.

#include "formats/json/nullable_column.h"

#include <gtest/gtest.h>

#include "column/column_helper.h"
#include "runtime/types.h"
#include "simdjson.h"

namespace starrocks::vectorized {

class AddNullableColumnTest : public ::testing::Test {};

TEST_F(AddNullableColumnTest, test_add_numeric) {
    TypeDescriptor t(TYPE_FLOAT);
    auto column = ColumnHelper::create_column(t, true);

    simdjson::ondemand::parser parser;
    auto json = R"(  { "f_float": 3.14}  )"_padded;
    auto doc = parser.iterate(json);
    simdjson::ondemand::value val = doc.find_field("f_float");

    auto st = add_nullable_column(column.get(), t, "f_float", &val, false);
    ASSERT_TRUE(st.ok());

    ASSERT_EQ("[3.14]", column->debug_string());
}

TEST_F(AddNullableColumnTest, test_add_binary) {
    TypeDescriptor t = TypeDescriptor::create_char_type(20);
    auto column = ColumnHelper::create_column(t, true);

    simdjson::ondemand::parser parser;
    auto json = R"(  { "f_float": "3.14"}  )"_padded;
    auto doc = parser.iterate(json);
    simdjson::ondemand::value val = doc.find_field("f_float");

    auto st = add_nullable_column(column.get(), t, "f_float", &val, false);
    ASSERT_TRUE(st.ok());

    ASSERT_EQ("['3.14']", column->debug_string());
}

TEST_F(AddNullableColumnTest, test_add_boolean) {
    TypeDescriptor t = TypeDescriptor::create_char_type(20);
    auto column = ColumnHelper::create_column(t, true);

    simdjson::ondemand::parser parser;
    auto json = R"(  { "f_boolean": true}  )"_padded;
    auto doc = parser.iterate(json);
    simdjson::ondemand::value val = doc.find_field("f_boolean");

    auto st = add_nullable_column(column.get(), t, "f_boolean", &val, false);
    ASSERT_TRUE(st.ok());

    ASSERT_EQ("['1']", column->debug_string());
}

TEST_F(AddNullableColumnTest, test_add_invalid_as_null) {
    TypeDescriptor t{TYPE_INT};
    auto column = ColumnHelper::create_column(t, true);

    simdjson::ondemand::parser parser;
    auto json = R"(  { "f_object": {"f_int": 1}}  )"_padded;
    auto doc = parser.iterate(json);
    simdjson::ondemand::value val = doc.find_field("f_object");

    auto st = add_nullable_column(column.get(), t, "f_object", &val, true);
    ASSERT_TRUE(st.ok());

    ASSERT_EQ("[NULL]", column->debug_string());
}

TEST_F(AddNullableColumnTest, test_add_invalid) {
    TypeDescriptor t{TYPE_INT};
    auto column = ColumnHelper::create_column(t, true);

    simdjson::ondemand::parser parser;
    auto json = R"(  { "f_object": {"f_int": 1}}  )"_padded;
    auto doc = parser.iterate(json);
    simdjson::ondemand::value val = doc.find_field("f_object");

    auto st = add_nullable_column(column.get(), t, "f_object", &val, false);
    ASSERT_TRUE(st.is_invalid_argument());
}

} // namespace starrocks::vectorized
