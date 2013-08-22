package lab1;

@license(omnetpp);

simple Application
{
    parameters:
        int nPieces;
    gates:
        input from_middle;
        output to_middle;
}

simple Middle
{
    gates:
        input from_link;
        input from_app;
        output to_link;
        output to_app;
}

simple Link
{
    parameters:
        int WindowLength;
        int HowToWait;
        int nPieces;
        int pError;
    gates:
        input from_middle;
        input from_net;
        output to_middle;
        output to_net;
}

module Terminal
{   
    parameters:
        int nPieces;
        int WindowLength;
        int HowToWait;
        int pError;
    @display("bgb=333,106");
    gates:
        input in;
        output out;
    submodules:
        Application: Application {
            parameters:
                nPieces = nPieces;
            @display("p=264,45;i=block/source");
        }
        Middle: Middle {
            @display("p=159,45;i=old/prot3");
        }
        Link: Link {
            parameters:
                WindowLength = WindowLength;
                HowToWait = HowToWait;
                nPieces = nPieces;
                pError = pError;
            @display("p=53,45;i=block/sink");
        }
    connections:
        Application.to_middle --> Middle.from_app;
        Middle.to_app --> Application.from_middle;
        Middle.to_link --> Link.from_middle;
        Link.to_net --> out;
        in --> Link.from_net;
        Link.to_middle --> Middle.from_link;
}

network Network
{
    parameters:
        int nPieces @prompt ("Indique el nÃºmero de tramas para el servidor: ");
        int WindowLength @prompt ("Indique el largo de la ventana deslizante: ");   
        int HowToWait @prompt ("Indique cuantas tramas se deben enviar antes de esperar un ACK: ");
        int pError @prompt("Indique la probabilidad de error para la simulación, 0-100: ");
    @display("bgb=333,279");
    types:
    submodules:
        Terminal0: Terminal {
            parameters:
                nPieces = nPieces;
                WindowLength = WindowLength;
                HowToWait = HowToWait;
                pError = pError;
            @display("p=65,117;i=device/server2");
        }
        Terminal1: Terminal {
            parameters:
                nPieces = nPieces;
                WindowLength = WindowLength;
                HowToWait = HowToWait;
                pError = pError;
            @display("p=162,44;i=device/pc");
        }
        Terminal2: Terminal {
            parameters:
                nPieces = nPieces;
                WindowLength = WindowLength;
                HowToWait = HowToWait;
                pError = pError;
            @display("p=258,117;i=device/pc");
        }
        Terminal3: Terminal {
            parameters:
                nPieces = nPieces;
                WindowLength = WindowLength;
                HowToWait = HowToWait;
                pError = pError;
            @display("p=219,217;i=device/pc");
        }
        Terminal4: Terminal {
            parameters:
                nPieces = nPieces;
                WindowLength = WindowLength;
                HowToWait = HowToWait;
                pError = pError;
            @display("p=104,217;i=device/pc");
        }
    connections:
        Terminal0.out --> Terminal1.in;
        Terminal1.out --> Terminal2.in;
        Terminal2.out --> Terminal3.in;
        Terminal3.out --> Terminal4.in;
        Terminal4.out --> Terminal0.in;
}

