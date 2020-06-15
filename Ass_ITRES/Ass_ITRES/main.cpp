
#include "CentralControl.h"
#include <thread>
int main() {
    CentralControl *cc = new CentralControl();
    cout << "*****************Elevator simulations****************** \n";
    cout << "Guiding: \n Please type where floor you are and where you want to go?\n";
    cout << "eg: 5 13 (and press enter key) \n";
    cout << "eg: 28 13 (and press enter key) \n";
    cc->takingElevator();

    return 1;
}