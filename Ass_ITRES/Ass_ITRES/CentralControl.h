#pragma once
#include "Elevator.h"
#include "Person.h"
#include <vector>
#include <iostream>


using namespace std;
class CentralControl {
private:
    Elevator *elevatorA; // able to stop in each floor once having request
    Elevator *elevatorB; // able to stop in each floor once having request
    void elevatorAction();
    mutex elevatorLocker;  //control thread to lock or unlock
public:
    void initCentralControl();
    vector <Person *> rqt_list;  // vector to store all request.
    void takingElevator();
    void elevatorselected();
    CentralControl();
    ~CentralControl();
};