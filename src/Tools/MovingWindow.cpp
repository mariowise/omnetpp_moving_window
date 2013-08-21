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
	FrameCounter = 1;
	FramePushCounter = 1;
	locker = true;
	srand(time(NULL));

	if(role.compare("emisor") == 0) {
		// Se realiza la población de la ventana hasta WindowLength o lo que haya en MessageQueue
		FrameCount = (int) MessageQueue->size();
		int i;
		for(i = 0; i < (int) MessageQueue->size() && i < WindowLength; i++) {
			Window->push_back(new Frame(
				host,						// Dirección del emisor
				i2s(0),						// Dirección del receptor
				new Control(				// Control
					"Informacion",				// Trama de información
					i2s(0),						// El total de tramas que envio
					false,						// No es la ultima trama
					i2s((int) Window->size())	// El numero de trama en la ventana (0..WindowLength)
				),
				MessageQueue->front()		// Data
			));
			MessageQueue->pop();		// Lo quito de la cola de Message
			FramePushCounter++;
		}
		cout << "Terminal" << s2i(host) << ": MovingWindow::MovingWindow(...) he agregado " << i << "/" << WindowLength << " elementos a la ventana de un total de " << FrameCount << endl;
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
					"No-numerada",		// Tipo de trama
					i2s(2),				// 2. ACK conexión
					false,				// PollFinal
					i2s(2)				// 2. ACK conexión
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
	} else if(frame->ctrl->type.compare("Informacion") == 0) {
		// Estoy recibiendo una trama de información
		int nroTrama = s2i(frame->ctrl->dataB);
		cout << "Terminal" << s2i(host) << ": recibe trama de Informacion desde Terminal" << s2i(frame->address) << endl;
		cout << "\t|->nroTrama=" << nroTrama << " (" << frame->ctrl->dataB << ")" << endl;
		cout << "\t|->SpectedFrame=" << SpectedFrame << endl << endl;
		if(nroTrama == SpectedFrame) {

			if(!acquire(frame)) // Intento incorporar la trama, si no puedo envio el NACK y no continúa
				goto nackTrama;	// Salida de la función

			// Si recibo lo que estoy esperando no hay problemas
			HowToWaitCount--;
			SpectedFrame = (SpectedFrame + 1) % WindowLength;


			// Si ya recibi toda la ventana
			if(HowToWaitCount == 0) {
				// Si ya recibí todas las tramas del lote. Debo reiniciar los contadores
				// y enviar el ACK de trama
				HowToWaitCount = HowToWait;
				sendACK(frame->address, nroTrama);
				module->getParentModule()->bubble("Enviando ACK");
			}
		} else {
			// Si no es la trama que estoy esperando simplemente envio un NACK pidiendo lo que espero
			nackTrama:
			if(locker) {
			    locker = false;
                sendNACK(frame->address, SpectedFrame);
                module->getParentModule()->bubble("Enviando NACK");
			}
		}
	} else if(frame->ctrl->type.compare("Supervisora") == 0) {
		// Estoy recibiendo una trama supervisora
		int type = s2i(frame->ctrl->dataA);
		int nro = s2i(frame->ctrl->dataB);
		if(type == 1) { // Se trata de un ACK
			// Es necesario reemplazar todos los elementos distintos del ACK hasta encontrar la trama que
			// fue indicada como ACK, luego esa trama tambien hay que reemplazarla.
			cout << "Terminal" << s2i(host) << ": Recibe trama supervisora ACK-" << nro << endl;
			int i, j;
			for(i = 0; i < FramePointer && FramePushCounter <= FrameCount; i++) {
				j = s2i(Window->at(i)->ctrl->dataB);
				free((*Window)[i]);	// Se libera el puntero al que apunta la posición del vector
				(*Window)[i] = new Frame(
					host,						// Dirección del emisor
					i2s(0),						// Dirección del receptor
					new Control(				// Control
						"Informacion",				// Trama de información
						i2s(0),						// El total de tramas que envio
						(FramePushCounter == FrameCount) ? true : false,	// No es la ultima trama
						i2s(i)						// El numero de trama en la ventana (0..WindowLength)
					),
					MessageQueue->front()		// Data
				);
				cout << "\t|->Agregando a la ventana a la trama Nro." << FramePushCounter << "/" << FrameCount << endl;
				FramePushCounter++;
				MessageQueue->pop();		// Lo quito de la cola de Message
				if(nro == j) // Si es la trama que me hacen ACK
					break; // No itero mas
			}
			cout << endl;
			// Si el nro del ACK que ha llegado es igual a FramePointer-1 entonces el lote ha sido recibido
			// de forma satisfactoria. Por lo tanto es necesario pasar el token al siguiente terminal.
			HowToWaitCount = HowToWait;
			releaseToken();
		} else if(type == 2) { // Se trata de un NACK
			// Un NACK en cualquier caso significa que el lote no fue recepcionado en su totalidad
			// por lo tanto se repite el reenvío del lote.
			FrameCounter -= HowToWait;
			HowToWaitCount = HowToWait;
			FramePointer = (WindowLength + FramePointer - HowToWait) % WindowLength;
			cout << "Terminal" << s2i(host) << ": reenviando lote debido NACK" << endl << endl;
			resume();
		}
	}
}

void MovingWindow::resume() {
	// A estas alturas se tiene HowToWaitCount como el número de tramas que
	// aun queda por enviar por lo tanto se realizará el envio de esa cantidad
	// de tramas. Pero no se deben enviar mas de module->TokenWait tramas
	cout << "Terminal" << s2i(host) << ": continua el envío" << endl;
	cout << "\t|->HowToWait=" << HowToWaitCount << endl;
	cout << "\t|->Window->size()=" << (int) Window->size() << endl;
	state = "enviando";
	while(HowToWaitCount > 0 && FrameCounter <= FrameCount) {
		cout << "\t|->enviando trama nro. " << FrameCounter << endl;

		if(FrameCounter == FrameCount) { // Es la última trama del mensaje
			Window->at(FramePointer)->ctrl->pollFinal = true; // Se fija como final
		}

		// Envío de la trama al medio
		// module->send(new cMessage(Window->at(FramePointer)->get().c_str(), 0), "to_net");
		module->scheduleAt(simTime(), new cMessage(Window->at(FramePointer)->get().c_str(), 0));

		// Se alteran los elementos de control
		HowToWaitCount--;
		FramePointer = (FramePointer + 1) % WindowLength;
		FrameCounter++;
	}
	cout << endl;
	module->getParentModule()->bubble("Enviando Lote");
	state = "bloqueado";
}

void MovingWindow::releaseToken() {
	// Finalmente es necesario pasar el Token al siguiente nodo de la red
	// para que este haga lo propio. Para esto es necesario construir y enviar
	// una trama No-numerada.
	cout << "Terminal" << s2i(host) << " pasando el token al siguiente terminal" << endl << endl;
	cout << "----------------------------------------------------------------------------------------" << endl << endl;
	Frame frame (
		i2s(0),					// Se falsifica el origen como proveniente de Terminal0
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

void MovingWindow::sendACK(string to, int nro) {
	// Debo enviar un ACK como trama Supervisora indicando el número de ACK
	Frame super (
		host,		// Origen
		to,			// Destino
		new Control(
			"Supervisora",	// Tipo
			i2s(1),			// 1. ACK de trama
			false,			// PollFinal
			i2s(nro)		// Número de trama que estoy acentiendo
		),
		"0"			// Chunk
	);
	module->send(new cMessage(super.get().c_str(), 0), "to_net");
}

void MovingWindow::sendNACK(string to, int nro) {
	// Debo enviar un NACK como trama Supervisora indicando el número de NACK
	Frame super (
		host, 			// Oriden
		to, 			// Destino
		new Control(
			"Supervisora",
			i2s(2),			// 2. NACK de trama
			false,			// PollFinal
			i2s(nro)		// Número de trama que estoy NACKEANDO
		),
		"0"				// Chunk
	);
	module->send(new cMessage(super.get().c_str(), 0), "to_net");
}

bool MovingWindow::acquire(Frame * frame) {
	cout << "Terminal" << s2i(host) << ": revisando CRC y apilando la trama " << FrameCounter << " ";
	int ranError = rand()%100;

	if(!locker)         // Si ya estoy bloqueado por rechazar una trama
	    ranError = 26;  // No la estropeo cuando ahora me llega buena

	if(frame->isValid() && (ranError > 25)) {
		cout << "[OK]" << endl << endl;		// Escribo la respuesta en la consola
		MessageQueue->push(frame->data);	// Apilo la trama en la cola mensaje
		if(frame->ctrl->pollFinal) {
			cout << "#######################################################" << endl;
			cout << "Terminal" << s2i(host) << ": declara haber recibido todas sus tramas (" << (int) MessageQueue->size() << ") desde Terminal" << s2i(frame->address) << endl;
			cout << "#######################################################" << endl;
			sendACK(frame->address, s2i(frame->ctrl->dataB));
		}
		FrameCounter++;
		locker = true;
		return true;
	} else {
		cout << "[Not-Valid!] Error: " << ranError << endl << endl;		// Escribo la respuesta en la consola
		return false;
	}
	cout << endl;
}
