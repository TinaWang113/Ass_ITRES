#include <iostream>
#include <thread>
#include <string>
#include <functional>
#include <conio.h>
#include "Elevator.h"
#include <vector>
#include <algorithm>
using namespace std;


/*
	Initial Elevator and new thread.
*/
Elevator::Elevator(int init_elevatorID, int init_currentFloor) {
    elevatorID = init_elevatorID;
    currentFloor = init_currentFloor;
    elevatorStatus = STOP;
    runThread = true;
    elevatorRun = new thread(&Elevator::run, this);
    elevatorRun->detach();
}

int Elevator::getElevatorStatus() {
    return elevatorStatus;
}

void Elevator::setElevatorStatus(int state) {
    this->elevatorStatus = state;
}

int Elevator::getElevatorCurrentFloor() {
    return this->currentFloor;
}

/*
	adding a request form Central control
*/
bool Elevator::addRequest(Person* person) {
    rqt_List.push_back(person);


    return true;
}

void Elevator::stopElevator() {
    this->runThread = false;
}

/*
	Run the thread as MOVINGTIME for mimicking the elevator which moves between floors.
	Using Locker.lock/unlock for avoiding Race condition

*/
void Elevator::run() {
    try {
        while (runThread) {
            //only moving when rqt_list is not empty
            while (!this->rqt_List.empty() || !this->moving_List.empty()) {
                locker.lock();
                move();
                locker.unlock();
                std::this_thread::sleep_for(std::chrono::seconds(MOVINGTIME));
            }
            elevatorStatus = STOP;
        }
    } catch (exception& e) {
        //std::terminate();
        cout << e.what() << '\n';
    }
}

/*
	Elevator running function:
	Main Parts:
		1. To pick up the visitor/resident from the request list, rqt_List
		2. Arriving, arrive(), the floor where someone to go.
		3. Open door when elevator arrives on the floor in the rqt_List/moving_List. ==> should consider the loading time, but it doesn't including this time.
			: ideally, it would be a function with delay to more close to reality
*/
void Elevator::move() {
    try {

        setTextColor(COLORTEXT_ELEVATOR_MSG);
        std::cout << "Elevator #" << this->elevatorID << " in the floor# " << currentFloor << ". \n";
        setTextColor(COLORTEXT_DEFAULT);
        for (vector<Person *>::iterator rqt_it = rqt_List.begin(); rqt_it != rqt_List.end();) {
            if (currentFloor == (*rqt_it)->in_Floor) {
                setTextColor(COLORTEXT_DOOR_MSG);
                std::cout << "Door is open." << endl;
                std::cout << "[MASSAGE] Elevator #" << elevatorID <<": Passengers get into the elevator in the floor. \n";
                std::cout << "Door is close." << endl;
                setTextColor(COLORTEXT_DEFAULT);
                addMovingList((*rqt_it)->in_Floor, (*rqt_it)->out_Floor);
                rqt_it = rqt_List.erase(rqt_it);
            }
            if (rqt_List.empty()) break;
            rqt_it++;
        }
        arrive();
        if (elevatorStatus == UP) {
            currentFloor++;
            if (currentFloor >= MAXLEVEL) setElevatorStatus(STOP);
        }
        if (elevatorStatus == DOWN) {
            currentFloor--;
            if (currentFloor <= MINLEVEL) setElevatorStatus(STOP);
        }
        //showing list for validate data
        if (elevatorStatus != STOP) {
            cout << "Waiting for getting in elevator#" << elevatorID << ": " << rqt_List.size() << endl;
            if (rqt_List.size() != 0) {
                cout << "waiting floor:";
                for (int i = 0; i < rqt_List.size(); i++) {
                    cout << "[" << rqt_List[i]->in_Floor << "] ";
                }
                cout << "\n";
            }
            cout << "Elevator#" << elevatorID << " : waiting for getting off: " << rqt_List.size() << endl;
            if (moving_List.size() != 0) {
                cout << "get off floor:";
                for (int i = 0; i < moving_List.size(); i++) {
                    cout << "[" << moving_List[i]->out_Floor << "] ";
                }
                cout << "\n";
            }

        }

    } catch (exception& e) {
        cout << e.what() << '\n';
    }
}

bool Elevator::addMovingList(int inFloor, int outFloor) {
    Person *person = new Person(inFloor, outFloor);
    moving_List.push_back(person);
    return true;
}


/*
	check anyone wants to get off from the moving_List
*/
void Elevator::arrive() {
    try {
        for (vector<Person *>::iterator moving_it = moving_List.begin(); moving_it != moving_List.end(); ) {
            if ((*moving_it)->out_Floor == currentFloor) {
                setTextColor(COLORTEXT_ELEVATOR_MSG);
                std::cout << "Door is open." << endl;
                std::cout << "[MASSAGE] Elevator #" << elevatorID << ": Passengers get off the elevator in the floor. \n";
                std::cout << "Door is close." << endl;
                setTextColor(COLORTEXT_DEFAULT);
                moving_it = moving_List.erase(moving_it);
            } else if ((*moving_it)->in_Floor == currentFloor) {
                if ((*moving_it)->out_Floor > currentFloor) {
                    setElevatorStatus(UP);
                    cout << "Elevator #" << elevatorID << " is going up. \n";
                } else if ((*moving_it)->out_Floor < currentFloor) {
                    setElevatorStatus(DOWN);
                    cout << "Elevator #" << elevatorID << " is going down. \n";
                }
            }

            //check
            if (moving_List.empty()) {
                cout << "Elevator#" << elevatorID << "Standby." <<endl;
                setElevatorStatus(STOP);
                break;
            }
            moving_it++;
        }
    } catch (exception& e) {
        cout << e.what() << '\n';
    }
}

/*
	Setting the text color for easy read msg from console
	The text color of massage form ElevatorA is Blue and from ElevatorB is Green.
	For the door opening/closing massage is red.

*/
void Elevator::setTextColor(int textAttribute) {
    switch (textAttribute) {
    case COLORTEXT_DEFAULT:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
        break;
    case COLORTEXT_ELEVATOR_MSG:
        if (elevatorID == 1) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE);
        } else if (elevatorID == 2) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        }
        break;
    case COLORTEXT_DOOR_MSG:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        break;
    default:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
        break;
    }
}

Elevator::~Elevator() {
    delete(elevatorRun);

}
