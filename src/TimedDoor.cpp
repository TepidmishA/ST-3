// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <thread>
#include <chrono>
#include <stdexcept>

//
// DoorTimerAdapter
//
DoorTimerAdapter::DoorTimerAdapter(TimedDoor& _door) : door(_door) {}

void DoorTimerAdapter::Timeout() {
    if (door.isDoorOpened()) {
        door.throwState();
    }
}

//
// TimedDoor
//
TimedDoor::TimedDoor(int timeout) {
    iTimeout = timeout;
    isOpened = false;
    adapter = new DoorTimerAdapter(*this);
}

bool TimedDoor::isDoorOpened() {
    return isOpened;
}

void TimedDoor::unlock() {
    isOpened = true;

    Timer timer;
    timer.tregister(iTimeout, adapter);
}

void TimedDoor::lock() {
    isOpened = false;
}

int TimedDoor::getTimeOut() const {
    return iTimeout;
}

void TimedDoor::throwState() {
    throw std::runtime_error("Door left opened for too long");
}

//
// Timer
//
void Timer::sleep(int time)
{
    std::this_thread::sleep_for(std::chrono::seconds(time));
}

void Timer::tregister(int timeout, TimerClient* _client)
{
    client = _client;

    std::thread([this, timeout]()
    {
        sleep(timeout);
        client->Timeout();
    }).detach();
}
