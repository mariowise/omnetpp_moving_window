//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef LINK_H_
#define LINK_H_

#include <string.h>
#include <omnetpp.h>
#include <map>
#include <queue>

#include <bitset>

// #include "Control.h"
// #include "Frame.h"
#include "MovingWindow.h"

using namespace std;

class Link : public cSimpleModule {
	private:
		string address;

	public:
		int WindowLength;
		int HowToWait;
		int TokenWait;
		int nPieces;
	
    protected:
    	map<string, MovingWindow*> * WindowMap;
    	queue<string> * messageQueue;

        virtual void initialize();
        virtual void handleMessage(cMessage * msg);
};

Register_Class(Link);

#endif /* LINK_H_ */
