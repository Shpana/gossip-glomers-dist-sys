#include <gtest/gtest.h>

class HandlersProcessorTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(HandlersProcessorTest, Add) {}

TEST_F(HandlersProcessorTest, AddTwice) {}

TEST_F(HandlersProcessorTest, AddNonTrivialState) {}

TEST_F(HandlersProcessorTest, ValidEnvironment) {}

TEST_F(HandlersProcessorTest, OnExecutor) {}

TEST_F(HandlersProcessorTest, Concurrently) {}

TEST_F(HandlersProcessorTest, RequestFailWithoutExceptions) {}
