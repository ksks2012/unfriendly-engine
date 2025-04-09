#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Define a simple interface
class MyInterface {
public:
    virtual ~MyInterface() = default;
    virtual void doSomething() = 0;
    virtual int getValue() = 0;
};

// Define a Mock class
class MockMyInterface : public MyInterface {
public:
    MOCK_METHOD0(doSomething, void());
    // MOCK_METHOD(void, doSomething, (), (override));
    MOCK_METHOD0(getValue, int());
    // MOCK_METHOD(int, getValue, (), (override));
};

// Test case
TEST(GMockTest, BasicMockTest) {
    MockMyInterface mock;
    
    // Set expectations
    EXPECT_CALL(mock, doSomething()).Times(1);
    EXPECT_CALL(mock, getValue()).WillOnce(testing::Return(42));

    // Call mock methods
    mock.doSomething();
    int result = mock.getValue();
    
    // Verify results
    EXPECT_EQ(result, 42);
}