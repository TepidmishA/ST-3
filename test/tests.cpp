// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TimedDoor.h"

#include <thread>
#include <chrono>
#include <stdexcept>

using ::testing::Exactly;
using ::testing::Return;

//
// Mock classes
//

class MockTimerClient : public TimerClient {
public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
public:
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(bool, isDoorOpened, (), (override));
};

//
// Fixture
//

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

TEST_F(TimedDoorTest, Constructor_DoorInitiallyClosed)
{
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, Constructor_TimeoutStored)
{
    EXPECT_EQ(1, door->getTimeOut());
}

//
// unlock
//

TEST_F(TimedDoorTest, Unlock_OpensDoor)
{
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, Unlock_NoException)
{
    EXPECT_NO_THROW(door->unlock());
}

//
// lock
//

TEST_F(TimedDoorTest, Lock_ClosesDoor)
{
    door->unlock();
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, Lock_NoException)
{
    EXPECT_NO_THROW(door->lock());
}

//
// throwState
//

TEST_F(TimedDoorTest, ThrowState_ThrowsRuntimeError)
{
    EXPECT_THROW(door->throwState(), std::runtime_error);
}

//
// DoorTimerAdapter
//

TEST(AdapterTest, TimeoutThrowsIfDoorOpen)
{
    TimedDoor door(1);
    door.unlock();

    DoorTimerAdapter adapter(door);

    EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST(AdapterTest, TimeoutNoThrowIfDoorClosed)
{
    TimedDoor door(1);
    door.lock();

    DoorTimerAdapter adapter(door);

    EXPECT_NO_THROW(adapter.Timeout());
}

//
// Timer
//

TEST(TimerTest, TimerCallsTimeout)
{
    MockTimerClient client;

    EXPECT_CALL(client, Timeout()).Times(Exactly(1));

    Timer timer;
    timer.tregister(0, &client);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

TEST(TimerTest, TimerDoesNotThrow)
{
    MockTimerClient client;

    Timer timer;

    EXPECT_NO_THROW(timer.tregister(0, &client));
}

//
// Timer + Door integration
//

TEST(SystemTest, OpenDoorThenTimeoutTriggersAdapter)
{
    TimedDoor door(0);

    door.unlock();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_TRUE(door.isDoorOpened());
}

//
// MockDoor
//

TEST(MockDoorTest, LockCalledOnce)
{
    MockDoor door;

    EXPECT_CALL(door, lock()).Times(Exactly(1));

    door.lock();
}

TEST(MockDoorTest, UnlockCalledOnce)
{
    MockDoor door;

    EXPECT_CALL(door, unlock()).Times(Exactly(1));

    door.unlock();
}

TEST(MockDoorTest, IsDoorOpenedReturnTrue)
{
    MockDoor door;

    EXPECT_CALL(door, isDoorOpened()).WillOnce(Return(true));

    EXPECT_TRUE(door.isDoorOpened());
}
