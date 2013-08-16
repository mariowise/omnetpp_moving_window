/*
 * Control.h
 *
 *  Created on: Aug 1, 2013
 *      Author: mario
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include <string.h>
#include <omnetpp.h>

using namespace std;

class Control {
	public:
		string type;		// Tipo información|Supervisora|No-numerada|
		string dataA;		// Info antes pf (NS|S|M) 
		bool pollFinal;		// Bit pf
		string dataB;		// Info despues pf (NR|M)

        Control(			// Constructor
        	string _type,
        	string _dataA,
        	bool _pollFinal,
        	string _dataB
    	);
    	Control(			// Constructor y decodificador
    		string input	
		);

    	string get();		// Codificador	
        void _dump();     // Para tareas de depuracion
};

#endif /* CONTROL_H_ */
