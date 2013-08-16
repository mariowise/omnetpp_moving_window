/*
 * Control.cpp
 *
 *  Created on: Aug 1, 2013
 *      Author: mario
 */

#include "Control.h"

Control::Control(string _type, string _dataA, bool _pollFinal, string _dataB) {
    type = _type;
    if(_type.compare("Informacion") == 0)
    	dataA = _dataA.substr(_dataA.length()-3, 3);
    else
    	dataA = _dataA.substr(_dataA.length()-2, 2);
    pollFinal = _pollFinal;
    dataB =_dataB.substr(_dataB.length()-3, 3);
}

Control::Control(string input) {
	if(input.length() != 8) {
		cout << "#Error: Control::Control(" << input << ") largo != 8" << endl;
	} else {
		if(input[0] == '0') {			// Trama de Informacion
			type = "Informacion";
			dataA = "";
			dataA.append(input, 1, 1);
		} else if(input[1] == '0') {	// Trama Supervisora
			type = "Supervisora";
			dataA = "";
		} else if(input[1] == '1') {	// Trama No-numerada
			type = "No-numerada";
			dataA = "";
		} else {
			cout << "#Error: Control::Control(" << input << ") tipo inconsecuente" << endl;
		}
		dataA.append(input, 2, 2);
		pollFinal = (input[4] == '1');
		dataB = "";
		dataB.append(input, 5, 3);
	}
}

string Control::get() {
	string response = "";
	if(type.compare("Informacion") == 0)
		response.append("0");
	else if(type.compare("Supervisora") == 0) 
		response.append("10");
	else if(type.compare("No-numerada") == 0)
		response.append("11");
	response.append(dataA);
	response.append((pollFinal) ? "1" : "0");
	response.append(dataB);
	if(response.length() != 8)
		cout << "#Error: Control::get() respondiendo largo != 8 <" << response << ">" << endl;
	return response;
}

void Control::_dump() {
	cout << "('" << 
	    type << "', '" <<
	    dataA << "', '" << 
	    pollFinal << "', '" <<
	    dataB << "') = " << 
	    get() << endl;
}