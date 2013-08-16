
#include "Application.h"

using namespace std;

void Application::initialize()
{
    nPieces = par("nPieces");
    if((strcmp(this->getParentModule()->getName(), "Terminal0") != 0)) {
        if (nPieces == 0) 
            nPieces += 1;
        for (int i = 0; i < abs(nPieces); i++)
            send(new cMessage("01111110",0),"to_middle");
    }
}

void Application::handleMessage(cMessage * msg)
{
    if(msg->arrivedOn("from_middle")) {
        string _msg = msg->getName();
        string _frm = _msg.substr(0, 3);
        _msg = _msg.substr(3, _msg.length()-3);
        ev << this->getParentModule()->getName() << " ha recibido " << _msg << endl;
    }
    // Aca genera la palabra nueva
}
