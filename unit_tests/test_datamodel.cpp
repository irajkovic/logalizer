#include "gtest/gtest.h"

#include "src/DataModel.hpp"
#include "mocks/ExecMock.hpp"

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

TEST(DataModel, testCreateTwoFilters)
{
    DataModel data;
    auto tabId1 = data.addFilter("filter", ".*filter.*");
    auto tabId2 = data.addFilter("filter", ".*filter.*");

    EXPECT_EQ(tabId1, 0);
    EXPECT_EQ(tabId2, 1);
    EXPECT_EQ(data.getTabCnt(), 2);
}

TEST(DataModel, testFilterShowLineNotMatching)
{
    DataModel data;
    auto tabId = data.addFilter("filter", ".*filter.*");
    auto append = data.getAppender("one");
    append("test");
    data.toggleTab(tabId);

    data.prepareLines();
    EXPECT_EQ(data.nextLine().isValid(), true);
}

TEST(DataModel, testFiltering)
{
    DataModel data;

    auto tabId = data.addFilter("hello", ".*hello.*");
    auto append = data.getAppender("one");
    append("hello world");

    data.prepareLines();
    EXPECT_EQ(data.nextLine().isValid(), true);

    data.toggleTab(tabId);

    data.prepareLines();
    EXPECT_EQ(data.nextLine().isValid(), false);
}


TEST(DataModel, testOnDataAvailableHandler)
{
    DataModel data;
    bool handlerCalled = false;

    data.registerOnNewDataAvailableListener([&handlerCalled](){
        handlerCalled = true;
    });

    auto append = data.getAppender("one");
    ASSERT_EQ(handlerCalled, false);
    append("hello world");
    ASSERT_EQ(handlerCalled, true);
}

TEST(DataModel, testScrollWithNoData)
{
    DataModel data;
    ASSERT_EQ(data.scrollUp(), false);
    ASSERT_EQ(data.scrollDown(), false);
}

TEST(DataModel, testScrollWithOneLine)
{
    DataModel data;
    auto append = data.getAppender("one");
    append("line 1");
    ASSERT_EQ(data.scrollUp(), false);
    ASSERT_EQ(data.scrollDown(), false);
}

TEST(DataModel, testScrollWithTwoLines)
{
    DataModel data;
    auto append = data.getAppender("one");
    append("line 1");
    append("line 2");
    ASSERT_EQ(data.scrollUp(), false);
    ASSERT_EQ(data.scrollDown(), true);
    ASSERT_EQ(data.scrollDown(), false);
    ASSERT_EQ(data.scrollUp(), true);
    ASSERT_EQ(data.scrollUp(), false);
}

TEST(DataModel, testFilters)
{
    DataModel data;
    auto append = data.getAppender("one");
    auto tabApples = data.addFilter("filter-apple", ".*apple.*");
    auto tabOranges = data.addFilter("filter-orange", ".*orange.*");

    append("line 1 apple");
    append("line 2 orange");
    append("line 3 orange");
    append("line 4 apple");

    // By default, all lines are visible
    data.prepareLines();
    EXPECT_EQ(data.nextLine().text, "line 1 apple");
    EXPECT_EQ(data.nextLine().text, "line 2 orange");
    EXPECT_EQ(data.nextLine().text, "line 3 orange");
    EXPECT_EQ(data.nextLine().text, "line 4 apple");
    EXPECT_EQ(data.nextLine().isValid(), false);

    // Hide oranges
    data.toggleTab(tabOranges);
    data.prepareLines();
    EXPECT_EQ(data.nextLine().text, "line 1 apple");
    EXPECT_EQ(data.nextLine().text, "line 4 apple");
    EXPECT_EQ(data.nextLine().isValid(), false);

    // Hide apples
    data.toggleTab(tabApples);
    data.prepareLines();
    EXPECT_EQ(data.nextLine().isValid(), false);
}

TEST(DataModel, testScrollWithFilters)
{
    DataModel data;
    auto append = data.getAppender("one");
    auto tabApples = data.addFilter("filter-apple", ".*apple.*");
    auto tabOranges = data.addFilter("filter-orange", ".*orange.*");

    append("line 1 apple");
    append("line 2 orange");
    append("line 3 orange");
    append("line 4 apple");

    // Hide oranges
    data.toggleTab(tabOranges);

    EXPECT_EQ(data.scrollDown(), true);
    EXPECT_EQ(data.scrollDown(), false);
    data.prepareLines();
    EXPECT_EQ(data.nextLine().text, "line 4 apple");
    EXPECT_EQ(data.nextLine().isValid(), false);

    EXPECT_EQ(data.scrollUp(), true);
    EXPECT_EQ(data.scrollUp(), false);
    data.prepareLines();
    EXPECT_EQ(data.nextLine().text, "line 1 apple");
    EXPECT_EQ(data.nextLine().text, "line 4 apple");
    EXPECT_EQ(data.nextLine().isValid(), false);
}

TEST(DataModel, testAddExternalNoFilters)
{
    DataModel data;
    EXPECT_EQ(data.addExternal("ext", "cat"), false);
}

TEST(DataModel, testAddExternalNoMatchingFilter)
{
    DataModel data;
    data.addFilter("non-matching", "");
    EXPECT_EQ(data.addExternal("ext", "cat"), false);
}

TEST(DataModel, testAddExternalWithMatchingFilter)
{
    auto exec = std::make_shared<ExecMock>();
    int callCnt = 0;
    exec->execHandler = [&callCnt] (auto cmd, auto line) { callCnt++; return "dummy";};
    DataModel data(exec);

    // Setup appender, filter and external command handler
    auto append = data.getAppender("one");
    data.addFilter("hello", ".*hello.*");
    EXPECT_EQ(data.addExternal("hello", "some-command"), true);

    // When non matching line is added, exec handler is not called
    append("Hello world");
    EXPECT_EQ(callCnt, 0);

    // When matching line is added, execHandler is called
    append("hello world");
    EXPECT_EQ(callCnt, 1);

    // Comment for the second line will be set to the value returned by the execHandler
    data.prepareLines();
    EXPECT_EQ(data.nextLine().comment.empty(), true);
    EXPECT_EQ(data.nextLine().comment, "dummy");
}


TEST(DataModel, testAddExternalWithMatchingFilterNoExec)
{
    DataModel data;

    // Setup appender, filter and external command handler
    auto append = data.getAppender("one");
    data.addFilter("hello", ".*hello.*");
    EXPECT_EQ(data.addExternal("hello", "some-command"), true);

    append("Hello world");
    append("hello world");

    // Comment for the second line will be set to the value returned by the execHandler
    data.prepareLines();
    EXPECT_EQ(data.nextLine().comment.empty(), true);
    EXPECT_EQ(data.nextLine().comment.empty(), true);
}
