/*
 * Frame.cpp
 *
 *  Created on: Aug 1, 2013
 *      Author: mario
 */

#include "Frame.h"

#define ESC "01111110"

#define s2i(S) (int) std::bitset<8>(S).to_ulong()       // String binario a entero
#define i2s(S) (string) std::bitset<8>(S).to_string()   // Entero a string binario

Frame::Frame(string _address, string _peer, Control * _ctrl, string _data) {
	address = _address;
	addressee = _peer;
	ctrl = _ctrl;
	data = _data;          
	fcs = CRC(address + addressee + ctrl->get() + data); // Cálculo del CRC para el campo FCS
}

Frame::Frame(string input) {
	int i = input.find(ESC, 0);					// Flag inicial
	int j = input.find(ESC, 8);					// Flag final
	if(!(i == 0 && j > 8 && j < (int) input.length()))
		cout << "#Error Frame::Frame(" << input << ") No se han encontrado las dos banderas" << endl;
	string core = foroz(input.substr(i+8, j-i-8));
	if( ((int) core.length()) < 40)
		cout << "#Error Frame::Frame(" << core << ") El largo de la trama no es de al menos 40 char (" << core.length() << ")" << endl;
	address = core.substr(0, 8);				// Obtengo el address
	addressee = core.substr(8, 8);				// Obtengo el destino
	ctrl = new Control(core.substr(16, 8));		// Creación del objeto de control
	data = core.substr(24, core.length()-40);	// Obtengo el data
    fcs = core.substr(core.length()-16, 16);	// Obtengo el campo de validación (16-bit)
}

string Frame::get() {
    string response (
        ESC +
        fooz(
            address + 
            addressee + 
            ctrl->get() + 
            data +
            fcs
        ) +
        ESC
    );
    return response;
}

void Frame::_dump() {
    cout << "('" <<
        address << "', '" << 
        addressee << "', '" <<
        ctrl->get() << "', '" <<
        data << "', '" <<
        fcs << "') = " <<
        get() << endl;
}

string Frame::CRC(string infoField) {

    std::string polynomial = "10001000000100001";       // Polinomio x^16+x^12+x^5+1.
    std::string fix = "1111111111111111";               // 2 Bytes de ajuste.
    std::string mxxn = infoField + "0000000000000000";  // M(x)*x^n.
    std::string FCS = "";                               // Salida FCS.

    // En caso de que el campo de información inicie con 0, los 2 primeros bytes de
    // M(x)*n^x se operan con XOR con la cadena de ajuste.
    if (mxxn.at(0) == '0') {
        for (int i = 0; i < 16; i++ ) {
            if (mxxn.at(i) == fix.at(i)) mxxn.replace(i,1,"0");
            else mxxn.replace(i,1,"1");
        }
    }

    // Se procede con la división entre M(x)*x^n y el polinomio cuyo resto será FCS.
    int begin = 0;
    while (begin+16 < (int)mxxn.length()){
        for (int i = 0; i < 17; i++) {
            if (mxxn.at(begin + i) == polynomial.at(i)) mxxn.replace(begin+i,1,"0");
            else mxxn.replace(begin+i,1,"1");
        }
    if ((int)mxxn.find_first_of("1") != -1) begin = mxxn.find_first_of("1");
    else break;
    }

    // Finalmente se ajusta el tamaño de FCS a los 16 bits establecidos.
    for (int i = mxxn.length()-16; i < (int)mxxn.length(); i++) FCS.push_back(mxxn.at(i));

    return FCS;
}

bool Frame::isValid() {

    string infoField = address + addressee + ctrl->get() + data;
    string FCS = fcs;

    std::string polynomial = "10001000000100001";   // Polinomio x^16+x^12+x^5+1.
    std::string fix = "1111111111111111";           // 2 Bytes de ajuste.
    std::string mxxn = infoField + FCS;             // M(x) concatenado con FCS.
    bool isValid = true;                            // Respuesta a la validación.

    // En caso de que el campo de información inicie con 0, los 2 primeros bytes de
    // M(x)*n^x se operan con XOR con la cadena de ajuste.
    if (mxxn.at(0) == '0') {
        for (int i = 0; i < 16; i++ ) {
            if (mxxn.at(i) == fix.at(i)) mxxn.replace(i,1,"0");
            else mxxn.replace(i,1,"1");
        }
    }

    // Se procede con la división entre M(x)*x^n y el polinomio cuyo resto será FCS.
    int begin = 0;
    while (begin+16 < (int)mxxn.length()){
        for (int i = 0; i < 17; i++) {
            if (mxxn.at(begin + i) == polynomial.at(i)) mxxn.replace(begin+i,1,"0");
            else mxxn.replace(begin+i,1,"1");
        }
    if ((int)mxxn.find_first_of("1") != -1) begin = mxxn.find_first_of("1");
    else break;
    }

    // Finalmente se comprueba si el resto de la división es 0.
    for (int i = 0; i < (int)mxxn.length(); i++) if(mxxn.at(i)== 1) isValid = false;

    return isValid;
}

string Frame::fooz(string in) {
    int i, u = 0;
    string response = "";
    for(i = 0; i < (int) in.length(); i++) {
        response.push_back(in.at(i));
        if(in.at(i) == '0')
            u = 0;
        else
            u++;
        if(u == 5) {
            response.push_back('0');
            u = 0;
        }
    }
    return response;
}

string Frame::foroz(string in) {
    int i, u = 0, flag = 0;
    string response = "";
    for(i = 0; i < (int) in.length(); i++) {
        if(in[i] == '0')
            u = 0;
        else
            u++;
        if(u == 5 && (i+1 < (int) in.length())) {
            u = 0;
            if(in[i+1] == '0') {
                flag = 1;
                response.push_back(in[i]);
                i++;
            }
        }
        if(flag == 0)
            response.push_back(in[i]);
        flag = 0;
    }
    return response;
}