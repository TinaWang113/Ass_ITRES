#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <Windows.h>
#include "Person.h"

#define STOP 0
#define UP 1
#define DOWN 2

#define MINLEVEL 1 //default: lobby
#define MOVINGTIME 2 // the moving time between floors
#define MAXLEVEL 100 //maxlevel
#define MAXPASSENGER 30

#define COLORTEXT_DEFAULT 0  // Default color for console 
#define COLORTEXT_ELEVATOR_MSG 1 // The color setting for massage form Elevators
#define COLORTEXT_DOOR_MSG 2 //The color setting from the door opening / closing massage.

class Elevator {
private:
	int maxLevel;
	int minLevel;
    int elevatorID;
    int currentFloor;
    std::thread *elevatorRun;
    int elevatorStatus;
    bool runThread;
    int currentPassenger;

public:
    std::mutex locker;
    std::vector<Person* > rqt_List;
    std::vector<Person*> moving_List;
    ~Elevator();
    Elevator(int init_elevatorID, int init_currentFloor);
    int getElevatorStatus();
    void setElevatorStatus(int state);
    int getElevatorCurrentFloor();
    bool addRequest(Person* person);
    bool addMovingList(int inFloor, int outFlor);
    void stopElevator();
    void run();
    void move();
    void arrive();
    void setTextColor(int textAttribute);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

};