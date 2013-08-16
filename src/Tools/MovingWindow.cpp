/*
 * MovingWindow.cpp
 *
 *  Created on: Aug 4, 2013
 *      Author: mario
 */

#include "MovingWindow.h"

#define s2i(S) (int) std::bitset<8>(S).to_ulong()       // String binario a entero
#define i2s(S) (string) std::bitset<8>(S).to_string()   // Entero a string binario

MovingWindow::MovingWindow(string _host, string _role, queue<string> * _mq, cSimpleModule * _module, int _PieceLength, int _WindowLength, int _HowToWait) {
	host = _host;
	role = _role;
	MessageQueue = _mq;
	module = _module;
	PieceLength = _PieceLength;
	WindowLength = _WindowLength;
	HowToWait = _HowToWait;

	// Configuraciones especiales
	Window = new vector<Frame*>();		// Se construye la ventana para comenzar recepción|envío
	FramePointer = 0;					// Se inicializa como puntero a la posición cero
	HowToWaitCount = HowToWait;			// Inicialmente no he enviado nada

	if(role.compare("emisor") == 0) {
		// Se realiza la población de la ventana hasta WindowLength o lo que haya en MessageQueue
		int i;
		for(i = 0; i < (int) MessageQueue->size() && i < WindowLength; i++) {
			Window->push_back(new Frame(
				host,						// Dirección del emisor
				i2s(0),						// Dirección del receptor
				new Control(				// Control
					"Informacion",				// Trama de información
					i2s(FrameCount),			// El total de tramas que envio
					false,						// No es la ultima trama
					i2s((int) Window->size())	// El numero de trama en la ventana (0..WindowLength)
				),
				MessageQueue->front()		// Data
			));
			MessageQueue->pop();		// Lo quito de la cola de Message
		}
		cout << "MovingWindow::MovingWindow(...) he agregado " << i << "/" << WindowLength << " elementos a la ventana" << endl;
		state = "espera";				// Seteo el estado como en espera
	}
	else if(role.compare("receptor") == 0) {
		SpectedFrame = 0;				// Espero la trama 0
		state = "recibiendo";			// Seteo el estado como recibiendo
		MessageQueue = new queue<string>();		// Se crea la cola mensajes
	}
	else 
		cout << "#Error MovingWindow::MovingWindow(...) se ha construido con un rol inválido <" << role << ">" << endl;
}

void MovingWindow::begin() {
	// Construcción de la trama no-numerada para solicitar conexión
	Frame lframe (
		host,	// Dirección del emisor
		i2s(0),	// Dirección del receptor
		new Control(
			"No-numerada",	// Trama no numerada para envio de mensajes de conexión
			i2s(1),			// 1. Solicitud de conexión
			true,			// Poll final 1
			i2s(1)			// 1. Solicitud de conexión
		),
		"0"		// Data no utilizada
	);
	// Envio del mensaje no-numerado que solicita la conexión
	ev << "Terminal" << s2i(host) << ": Enviando mensaje" << endl;
	ev << " |->Destino: Terminal" << s2i(lframe.addressee) << endl;
	ev << " |->Tipo: " << lframe.ctrl->type << endl;
	module->send(new cMessage(lframe.get().c_str(), 0), "to_net");
}

void MovingWindow::broker(Frame * frame) {
	// Detección de que tipo de trama es
	if(frame->ctrl->type.compare("No-numerada") == 0) {
		int nro = s2i(frame->ctrl->dataA);
		if(nro == 1) { // Se trata de una trama de solicitud de conexión
			// Llegando a estas altura ya existe la ventana deslizante, por lo tanto
			// queda por enviar el acepto la solicitud de conexión
			Frame response (
				host,
				frame->address,
				new Control(
					"No-numerada",	// Tipo de trama
					i2s(2),			// 2. ACK conexión
					false,			// PollFinal
					i2s(2)			// 2. ACK conexión
				),
				"0"				// Chunk
			);
			module->send(new cMessage(response.get().c_str(), 0), "to_net");
		}
		else if(nro == 2) { // Me han aceptado la conexión por lo tanto resumo el trabajo
			resume();
		}
		else if(nro == 3) {
			// Me han pasado el token por lo tanto debo hacer la primitiva de conexión
			if(state.compare("bloqueado") == 0) {
				// En realidad ya habia establecio la conexión entonces me dicen que
				// me toca trabajar, continuo con resume.
				resume();
			} else if(state.compare("espera") == 0) // Me toca conectarme
				begin();
		}
	}
}

void MovingWindow::resume() {
	// A estas alturas se tiene HowToWaitCount como el número de tramas que 
	// aun queda por enviar por lo tanto se realizará el envio de esa cantidad
	// de tramas. Pero no se deben enviar mas de module->TokenWait tramas
	cout << "Resumiendo envio en Terminal" << s2i(host) << "con HowToWaitCount=" << HowToWaitCount << " y Window->size()=" << (int)Window->size() << endl;
	state = "enviando";
	while(HowToWaitCount > 0) {
		// Envío de la trama al medio
		module->send(new cMessage(Window->at(FramePointer)->get().c_str(), 0), "to_net");

		// Se alteran los elementos de control
		HowToWaitCount--;
		FramePointer++;
	}
	state = "bloqueado";
}

void MovingWindow::resume(Frame * frame) {

}

void MovingWindow::releaseToken() {
	// Finalmente es necesario pasar el Token al siguiente nodo de la red
	// para que este haga lo propio. Para esto es necesario construir y enviar
	// una trama No-numerada.
	cout << "Terminal" << s2i(host) << " pasando el token al siguiente" << endl;
	Frame frame (
		host, 					// Origen
		i2s((s2i(host))%4+1),	// Destino
		new Control(
			"No-numerada",	// Tipo de trama
			i2s(2), 		// 3. Paso del token
			false,			// PollFinal
			i2s(2)			// 3. Paso del token
		),
		"0"						// Chunk
	);
	module->send(new cMessage(frame.get().c_str(), 0), "to_net"); // Paso token
}