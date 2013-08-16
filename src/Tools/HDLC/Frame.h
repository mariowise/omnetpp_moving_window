/*
 * Frame.h
 *
 *  Created on: Aug 1, 2013
 *      Author: mario
 */

#ifndef FRAME_H_
#define FRAME_H_

#include <string.h>
#include <omnetpp.h>

#include "Control.h"

using namespace std;

class Frame {
	public:
		string address;		// Dirección del host
		string addressee;	// Dirección del peer
		Control * ctrl;		// Campo de control
		string data;		// Informacion
		string fcs;			// Validacion

		Frame(
			string _address,	// Dirección del host
			string _peer,		// Dirección del peer
			Control * _ctrl,	// Campo de control
			string _data		// Información
		);

		Frame(string input);	// Constructor y decodificador

		string get();			// Codificador
		void _dump();			// Para tareas de depuracion

		string CRC(string infoField);	// Calcula el CRC de un input
		bool isValid();					// Revisa si el data es correcto según FCS

		string fooz(string in);	// Cada cinco 1 agrega un cero
		string foroz(string in);	// cada cinco 
};

#endif /* FRAME_H_ */
