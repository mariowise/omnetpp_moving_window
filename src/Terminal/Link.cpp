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

#include "Link.h"

#define s2i(S) (int) std::bitset<8>(S).to_ulong()       // String binario a entero
#define i2s(S) (string) std::bitset<8>(S).to_string()   // Entero a string binario

using namespace std;

void Link::initialize() {
    // Inicio de parámetros recibidos del usuario
    WindowLength = par("WindowLength");
    HowToWait = par("HowToWait");
    nPieces = par("nPieces");

    // Tramas enviadas antes de ceder el token.
    TokenWait = 5;

    // Inicio de objetos  
    messageQueue = new queue<string>();
    WindowMap = new map<string, MovingWindow*>();
    
    // Bautizo de dispositivos
    if(strcmp(this->getParentModule()->getName(), "Terminal0") == 0)
        address = i2s(0);
    if(strcmp(this->getParentModule()->getName(), "Terminal1") == 0)
        address = i2s(1);
    if(strcmp(this->getParentModule()->getName(), "Terminal2") == 0)
        address = i2s(2);
    if(strcmp(this->getParentModule()->getName(), "Terminal3") == 0)
        address = i2s(3);
    if(strcmp(this->getParentModule()->getName(), "Terminal4") == 0)
        address = i2s(4);
}

void Link::handleMessage(cMessage * msg) {
    // Es necesario discriminar que tipo de mensaje esta manejando la capa link. Dentro de las
    // posibilidades estan: Desde físico (from net) y desde la capa middle (from middle) con la
    // eventual posibilidad que se trate de un automensaje tambien (veremos)
    if (msg->arrivedOn("from_net")) {
        // Se construye un objeto trama
        Frame * frame = new Frame(msg->getName());

        // Se revisa si es que el mensaje es para mi (este terminal)
        if (this->address.compare(frame->addressee) != 0) 
            send(msg, "to_net"); // Envia el mensaje a la red nuevamente
        else { // El mensaje si es para mi
            // Es necesario verificar si tengo una ventana dezlizante en mi arreglo asociativo
            // para procesar la trama que ha llegado. Sino es necesario crearla y pasarsela
            if(WindowMap->find(frame->address) != WindowMap->end()) { // Si existe la llave
                // Se le entrega la trama al movingWindow respectivo para que la procese
                (*WindowMap)[frame->address]->broker(frame);
            } else { // No existe esa llave
                // Es necesario crear una nueva MovingWindow
                (*WindowMap)[frame->address] = new MovingWindow(
                    address,
                    (string) "receptor",
                    NULL,
                    this,
                    8,
                    WindowLength,
                    HowToWait
                );
                // Luego pasarle la trama para que la procese en recepción
                (*WindowMap)[frame->address]->broker(frame);
            }
            ev << "Terminal" << s2i(address) << ": Recibe mensaje" << endl;
            ev << " |->Origen: Terminal" << s2i(frame->address) << endl;
            ev << " |->Tipo: " << frame->ctrl->type << endl;
        }
    }
    else if(msg->arrivedOn("from_middle")) {
        // Si recibe una trama desde middle siempre se encola en la messageQueue
        messageQueue->push(msg->getName());

        // Se se trata de la ultima trama que recibiré desde middle entonces el
        // mensaje ya esta completo en capa link y es necesario comenzar a enviar
        if ((int) messageQueue->size() == nPieces) {
            // Se construye la ventana deslizante
            (*WindowMap)[i2s(0)] = new MovingWindow(
                address, 
                (string) "emisor", 
                messageQueue, 
                this, 
                8, 
                WindowLength, 
                HowToWait
            );
            
            // Si soy el Terminal1 asumimos que para la simulación el comenzará
            // teniendo el token, por lo tanto solo el comienza con la solicitud
            // de conexión
            if(address.compare(i2s(1)) == 0)
                (*WindowMap)[i2s(0)]->begin();
        }
    } else { // Auto-message
        send(msg, "to_net");
    }
}