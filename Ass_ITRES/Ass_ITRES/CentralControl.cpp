/*
CentralControl:
		handle elevator process and request list
		Referring:
			CIBSE Guide D: Transportation Systems in Buildings 2015
		After considering time and resources:
	Basic design:
		1. MAXLEVEL is 100, MINLEVEL is 1.
		2. Two elevators are elevatorA and elevatorB:
			-> using the weight to choice which elevator serve the request.
			-> adding a condition, MAXPASSENGERS, for the elevator's capacity.
		3. Simply traveling time : MOVINGTIME
		4. Usage of floor  --> this condition is not be consider this time
		5. Rush hour/busy floor and Building type --> doesn't consider this time as well.
*/

#include <iostream>
#include <thread>
#include <string>
#include <functional>
#include <conio.h>
#include <vector>
#include "CentralControl.h"
#include <algorithm>
using namespace std;

/*
	Main function:
		initial Central Control for elevators. Default elevator will be three elevators (elevatorA, evatorB, high_Elevator)
		the new thread for elevator selection.
*/
CentralControl::CentralControl() {
    initCentralControl();
    thread elevatorThread(&CentralControl::elevatorselected, this);
    elevatorThread.detach();

}


/*
	To judge the direction of elevator for checking the director of vistor/resident and the current status of elevator.
*/
int UpOrDown(int inFloor, int outFloor) {
    if (outFloor - inFloor > 0) {
        return UP;
    } else if (outFloor - inFloor < 0) {
        return DOWN;
    }
    return STOP;
}


/*
	Simulating someone wants to take elevators from I/O.
	First input, in_Floor: where a visitor/resident presses the button.
	Second input, out_Floor: where the visitor/resident wants to go.
	input must be between MINLEVEL and MAXLEVEL.

*/
void CentralControl::takingElevator() {
    int in_Floor;
    int out_Floor;
    Person* person;
    try {
        while (true) {
            bool isValid = true;
            cin >> in_Floor >> out_Floor;
            if (in_Floor > MAXLEVEL || in_Floor < MINLEVEL) isValid = false;
            if (out_Floor > MAXLEVEL || out_Floor < MINLEVEL) isValid = false;
            if (in_Floor == out_Floor) isValid = false;
            if (cin.fail()) isValid = false;
            if (isValid) {

                cout << "Reqest Floor: " << in_Floor << ". Destination: " << out_Floor << endl;
                person = new Person(in_Floor, out_Floor);
                rqt_list.push_back(person);
            } else {
                std::cin.clear();
                std::cin.ignore(256, '\n');
                cout << "The max floor is " << MAXLEVEL << ", and the min floor is 1 (Lobby). Please type again where you are and where you want to go. \n Please enter again." << endl;
            }
        }
    } catch (exception& e) {
        cout << e.what() << '\n';
    }
}

/*
The function to deal with elevators who are making requests for high volume visitors/residents.
	Strategy for selecting elevators:
	1. calculate a weight for each elevator:
	  -> the same direction
	  -> which elevator is closer to the visitor/resident.
	2. Based on the weight, assign the visitor/resident to the rqt_list of each elevator.


*/
#define DEFAULT_WEIGHT 100
void CentralControl::elevatorAction() {
    int direction;
    typedef struct select {
        Elevator *elevator;
        int state;
        int weight = DEFAULT_WEIGHT;
    } priority;
    priority mgm_ElevatorA = { elevatorA, elevatorA->getElevatorStatus() };
    priority mgm_ElevatorB = { elevatorB, elevatorB->getElevatorStatus() };

    elevatorA->locker.lock();
    elevatorB->locker.lock();

    //set a variable for calculate how many request left.
    int num = rqt_list.size();
    try {
        for (vector<Person *>::iterator rqt_it = rqt_list.begin(); rqt_it < rqt_list.end(); rqt_it++) {
            mgm_ElevatorA.weight = DEFAULT_WEIGHT;
            mgm_ElevatorB.weight = DEFAULT_WEIGHT;
            direction = UpOrDown((*rqt_it)->in_Floor, (*rqt_it)->out_Floor);
            //for elevatorA. Calculting the weight
            if (mgm_ElevatorA.state == direction) {
                if (direction == UP && (elevatorA->getElevatorCurrentFloor() < (*rqt_it)->in_Floor)) {
                    mgm_ElevatorA.weight = abs(elevatorA->getElevatorCurrentFloor() - (*rqt_it)->in_Floor);
                } else if(direction == DOWN && (elevatorA->getElevatorCurrentFloor() > (*rqt_it)->in_Floor)) {//double check the second condition
                    mgm_ElevatorA.weight = abs(elevatorA->getElevatorCurrentFloor() - (*rqt_it)->in_Floor);
                }
            } else if (mgm_ElevatorA.state == STOP) {
                mgm_ElevatorA.weight =abs(elevatorA->getElevatorCurrentFloor() - (*rqt_it)->in_Floor);
            }
            //for elevatorB. Calculting the weight
            if (mgm_ElevatorB.state == direction) {
                if (direction == UP && (elevatorB->getElevatorCurrentFloor() < (*rqt_it)->in_Floor)) {
                    mgm_ElevatorB.weight = abs(elevatorB->getElevatorCurrentFloor() - (*rqt_it)->in_Floor);
                } else if (direction == DOWN && (elevatorB->getElevatorCurrentFloor() > (*rqt_it)->in_Floor)) {//double check the second condition
                    mgm_ElevatorB.weight = abs(elevatorB->getElevatorCurrentFloor() - (*rqt_it)->in_Floor);
                }
            } else if (mgm_ElevatorB.state == STOP) {
                mgm_ElevatorB.weight = abs(elevatorB->getElevatorCurrentFloor() - (*rqt_it)->in_Floor);
            }

            //skipping request.
            if (mgm_ElevatorA.weight == DEFAULT_WEIGHT && mgm_ElevatorB.weight == DEFAULT_WEIGHT) {
                continue;
            }
            // 2. assigning the request to the suitable elevator
            if (mgm_ElevatorA.weight <= mgm_ElevatorB.weight) {
                elevatorA->addRequest(*rqt_it);
                if (elevatorA->getElevatorStatus() == STOP) {
                    int toPickUp = ((*rqt_it)->in_Floor - elevatorA->getElevatorCurrentFloor());
                    if (toPickUp > 0) {
                        elevatorA->setElevatorStatus(UP);
                    } else if (toPickUp < 0) {
                        elevatorA->setElevatorStatus(DOWN);
                    } else {
                        toPickUp = ((*rqt_it)->out_Floor - (*rqt_it)->in_Floor) ? UP : DOWN;
                        elevatorA->setElevatorStatus(toPickUp);
                    }
                }
            } else {
                elevatorB->addRequest((*rqt_it));
                if (elevatorB->getElevatorStatus() == STOP) {
                    int toPickUp = ((*rqt_it)->in_Floor - elevatorB->getElevatorCurrentFloor());
                    if (toPickUp > 0) {
                        elevatorB->setElevatorStatus(UP);
                    } else if (toPickUp < 0) {
                        elevatorB->setElevatorStatus(DOWN);
                    } else {
                        toPickUp = ((*rqt_it)->out_Floor - (*rqt_it)->out_Floor) ? UP : DOWN;
                        elevatorB->setElevatorStatus(toPickUp);
                    }
                }
            }

            rqt_it = rqt_list.erase(rqt_it);
            num--;
            if (num == 0) break;
        }
        elevatorA->locker.unlock();
        elevatorB->locker.unlock();
    } catch (exception& e) {
        cout << e.what() << '\n';
    }

}

/*
	Create and initial elevators.
*/
void CentralControl::initCentralControl() {
    elevatorA = new Elevator(1, 1);
    elevatorB = new Elevator(2, 1);
}

/*
	Running a thread to taking/handel reqeust for each 0.5 sec.
*/
void CentralControl::elevatorselected() {
    try {
        while (true) {
            elevatorLocker.lock();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            while (!rqt_list.empty()) {
                elevatorAction();
            }
            elevatorLocker.unlock();
        }
    } catch (exception& e) {
        cout << e.what() << '\n';
    }
}

/*
	destory
*/
CentralControl::~CentralControl() {
    //destroy
    delete(elevatorA);
    delete(elevatorB);

}

