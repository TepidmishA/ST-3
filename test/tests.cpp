// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TimedDoor.h"

#include <thread>
#include <chrono>
#include <stdexcept>

using ::testing::Exactly;

class TimedDoorTest : public ::testing::Test {
 protected:
    TimedDoor* door;

    void SetUp() override {
        door = new TimedDoor(1);
    }

    void TearDown() override {
        delete door;
    }
};


//
// Constructor
//
TEST_F(TimedDoorTest, Constructor_DoorInitiallyClosed) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, Constructor_TimeoutStored) {
    EXPECT_EQ(1, door->getTimeOut());
}

//
// unlock
//
TEST_F(TimedDoorTest, Unlock_OpensDoor) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, Unlock_OnUnlockedNoException) {
    door->unlock();
    EXPECT_NO_THROW(door->unlock());
}

//
// lock
//
TEST_F(TimedDoorTest, Lock_ClosesDoor) {
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, Lock_OnLockedNoException) {
    EXPECT_NO_THROW(door->lock());
}

//
// throwState
//
TEST_F(TimedDoorTest, ThrowState_ThrowsRuntimeError) {
    EXPECT_THROW(door->throwState(), std::runtime_error);
}


//
// DoorTimerAdapter
//
class DoorTimerAdapterTest : public ::testing::Test {
 protected:
    TimedDoor* door;
    DoorTimerAdapter* adapter;

    void SetUp() override {
        door = new TimedDoor(1);
        adapter = new DoorTimerAdapter(*door);
    }

    void TearDown() override {
        delete door;
        delete adapter;
    }
};

TEST_F(DoorTimerAdapterTest, TimeoutThrowsIfDoorOpen) {
    door->unlock();
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(DoorTimerAdapterTest, TimeoutNoThrowIfDoorClosed) {
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}

//
// Timer
//
class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

TEST(TimerTest, TimerDoesNotThrow) {
    MockTimerClient client;
    Timer timer;

    EXPECT_NO_THROW(timer.tregister(0, &client));
}

TEST(TimerTest, TimerCallsTimeout) {
    MockTimerClient client;
    EXPECT_CALL(client, Timeout()).Times(Exactly(1));

    Timer timer;
    timer.tregister(0, &client);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

//
// Integration
//
class TestDoorTimerAdapter : public DoorTimerAdapter {
 public:
    std::exception_ptr* exception;

    TestDoorTimerAdapter(TimedDoor& door, std::exception_ptr* ex)
        : DoorTimerAdapter(door), exception(ex) {}

    void Timeout() override {
        try {
            DoorTimerAdapter::Timeout();
        }
        catch (...) {
            *exception = std::current_exception();
        }
    }
};

class IntegrationTest : public ::testing::Test {
 protected:
    TimedDoor* door;

    TestDoorTimerAdapter* test_adapter;
    std::exception_ptr exception;

    void SetUp() override {
        door = new TimedDoor(1);

        exception = nullptr;
        test_adapter = new TestDoorTimerAdapter(*door, &exception);

        door->changeAdapter(test_adapter);
    }

    void TearDown() override {
        delete door;
    }
};

TEST_F(IntegrationTest, ThrowsWhenDoorOpenAfterTimeout) {
    door->unlock();

    std::this_thread::sleep_for(std::chrono::milliseconds(1050));

    ASSERT_NE(exception, nullptr);
}

TEST_F(IntegrationTest, NoThrowWhenDoorClosedAfterTimeout) {
    door->unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    door->lock();

    std::this_thread::sleep_for(std::chrono::milliseconds(1050));

    ASSERT_EQ(exception, nullptr);
}
