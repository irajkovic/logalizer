#include "gtest/gtest.h"

#include "src/DataModel.hpp"

TEST(DataModel, testEmptyModel)
{
    DataModel data;
    EXPECT_EQ(data.getTabCnt(), 0);
    EXPECT_EQ(!!data.getTab(0), false);
}

TEST(DataModel, testAddingAppender)
{
    DataModel data;
    auto append = data.getAppender("one");
    EXPECT_EQ(data.getTabCnt(), 1);

    auto tab = data.getTab(0);

    EXPECT_EQ(!!tab, true);
    EXPECT_EQ(tab.valid, true);
    EXPECT_EQ(tab.name, "one");
    EXPECT_EQ(tab.rowsCnt, 0U);
}

TEST(DataModel, testReadingWhenNoLinesAvailable)
{
    DataModel data;
    data.prepareLines();
    EXPECT_EQ(data.nextLine().isValid(), false);
}

TEST(DataModel, testReadingWhenLinesAreAvailable)
{
    DataModel data;

    auto append = data.getAppender("one");
    append("test");
    data.prepareLines();
    EXPECT_EQ(data.nextLine().isValid(), true);
}

TEST(DataModel, testAddingInvalidLine)
{
    DataModel data;
    data.addLine("test", 0); // Appender 0 is not defined
    data.prepareLines();
    EXPECT_EQ(data.nextLine().isValid(), false);
}
