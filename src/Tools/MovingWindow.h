/*
 * MovingWindow.h
 *
 *  Created on: Aug 4, 2013
 *      Author: mario
 */


/* Parámetros
	- Cantidad de tramas para el servidor (Cantidad de partes de un mesaje)		| Aplicación
	- Tamaño de la ventana deslizante											| MovingWindow (WindowLength)
	- Número de tramas a enviar o recibir antes de enviar un ACK				| MovingWindow (HowToWait)
	- Probabilidad de error														| Link

   Constantes
   	- Cuantas tramas envia cada host 											| Link/MovingWindow
*/

#ifndef MOVINGWINDOW_H_
#define MOVINGWINDOW_H_

#include <string.h>
#include <omnetpp.h>

#include <bitset>
#include <vector>
#include <queue>

#include <stdlib.h>
#include <time.h>

#include "Control.h"
#include "Frame.h"

using namespace std;

class MovingWindow {
	private:
		vector<Frame*> * Window;		// Lista de tramas en la ventana deslizante

		int FrameCount;				// Total de tramas
		int FrameCounter;			// Contador de las tramas que van siendo enviadas	
		int FramePushCounter;		// Contador de las tramas que van siendo encoladas para envío
		int SpectedFrame;			// Trama que estoy esperando
		int FramePointer;			// Cabezal sobre el vector Window
		int TokenLimit;				// Cuantas tramas enviar antes de ceder el token
		bool locker;                // Bloquea la recepción hasta que se reciba lo que estoy esperando
		int pError;                 // Probabilidad de error
		int holder;                 // Ayuda a mantener la circularidad de la ventana

		int PieceLength;			// Largo del los pedacitos en que se corte el mensaje
		int WindowLength;			// Largo de la ventana deslizante
		int HowToWait;				// Cuantas tramas esperar antes de bloquearse

		int HowToWaitCount;			// Lleve el registro de cuantos ya he enviado en esta espera

		string role;				// Rol de emisor|receptor
		string state;				// Estado de espera|enviando|reciviendo|bloqueado|terminado
		string host;				// Address del objeto Link que me instancia
		cSimpleModule * module;		// Puntero al módulo para enviar los mensajes

		void releaseToken();		// Envia el token a la siguiente estación
		void sendACK(				// Envía un ACK a un host dado
			string to,					// Destino 
			int nro						// Número de ACK
		);
		void sendNACK(				// Envia un NACK a un host dado
			string to,					// Destino
			int nro 					// Número del NACK
		);
		void replace(int pos);		// Llena espacios de la ventana con elementos del messageQueue
		bool acquire(Frame * frame);// Adquiere la trama que esta llegando, aplica CRC y decide
		void error(
            string * dataField
        );

	public:
		queue<string> * MessageQueue;	// Cola de pedazos de mensaje a enviar

		MovingWindow(				// Construye y configura
			string _host,				// Dirección del dispositivo donde se contruye la Ventana
			string _role,				// Rol del host
			queue<string> * _mq,		// Puntero a message queue
			cSimpleModule * _module,	// Puntero al módulo
			int _PieceLength,			
			int _WindowLength,
			int _HowToWait
		);

		void broker(Frame * frame);	// Decide que es lo que hay que hacer al recibir un stream de bits
		void begin();				// Comienza el envio de la petición asumiendo que tiene el token
		void resume();				// Retoma el trabajo de envío en la ventana
};

#endif /* MOVINGWINDOW_H_ */
