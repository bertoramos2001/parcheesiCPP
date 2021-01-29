//Este archivo contiene el código fuente de la segunda versión del proyecto de FP1 de Alberto Ramos y Gonzalo Segovia, grupo 4 de prácticas

#include <iostream>
#include <ctime>
#include <Windows.h>
#include <fstream>
using namespace std;

typedef enum { Amarillo, Azul, Rojo, Verde, Gris, Ninguno } tColor;
const int NUM_JUGADORES = 4, NUM_FICHAS = 4, NUM_CASILLAS = 68, CASILLA_META = 108, CASILLA_CASA = -1, CASILLA1_RECTA_META = 101;

typedef int tFichas[NUM_FICHAS];

typedef struct { //ahora, los jugadores están representados por una estructura que contiene su color y el array de fichas
    tColor color;
    tFichas fichas;
}tJugador;

typedef struct { //ahora, cada casilla contendrá su información (quié color hay en la calle 1 y en la calle 2)
    tColor calle1; //color de la ficha que hay en calle1
    tColor calle2; //color de la ficha que hay en calle2
}tCasilla;

typedef tJugador tJugadores[NUM_JUGADORES]; //ahora, tJugadores será un array de NUM_JUGADORES tJugador
typedef tCasilla tCasillas[NUM_CASILLAS]; //ahora, tCasillas será un array de NUM_CASILLAS tCasilla

typedef struct { //estructura que engloba al juego
    tCasillas casillas;
    tJugadores jugadores;
    tColor jugadorTurno;
    int tirada;
    int premio;
    int seises;
    int ultimaFichaMovida;  //inicializamos ultimaFichaMovida a -1 para no encontrarnos con el fallo de falta de inicializacion de la variable; además así podremos comprobar si es la primera ficha que se va a mover en el turno o ya se han movido otras fichas antes
}tJuego;

//funciones añadidas de la primera versión
bool esSeguro(int casilla);
int zanataJugador(tColor color);
int salidaJugador(tColor jugador);
string colorACadena(tColor color);
//funciones segunda versión
int cuantasEn(const tFichas jugador, int casilla);
int primeraEn(const tFichas jugador, int casilla);
int segundaEn(const tFichas jugador, int casilla);

void iniciar(tJuego& juego);
void saleFicha(tJuego& juego);
void aCasita(tJuego& juego, int casilla);
bool procesa5(tJuego& juego, bool& pasaTurno);
bool puente(tCasillas casillas, int casilla);
bool puedeMover(tJuego& juego, int ficha, int& casilla);
void mover(tJuego& juego, int ficha, int casilla);
void abrirPuente(tJuego& juego, int casillaOrig, int casillaDest);
bool procesa6(tJuego& juego, bool& pasaTurno);
bool todasEnMeta(const tFichas jugador);
bool jugar(tJuego& juego, bool& fin);

//variables globales para cargar y debugear el codigo
const bool DEBUG = true;
const string ARCHIVO = "C:/Users/salgu/OneDrive/Escritorio/parchisV3G04/parchisV3G04/archivoDebug.txt";
ifstream archivo;
//funcion para debuggear el código 
void cargar(tJuego& juego, ifstream& archivo);


// PROTOTIPOS DE LOS SUBPROGRAMAS DE VISUALIZACIÓN DEL TABLERO
void mostrar(const tJuego& juego);
void setColor(tColor color);
void iniciaColores();



int main() {
    //nuevas variables en main(); tJuego almacena todos los datos actuales del juego y tCasillas almacena los dos arrays de casillas con su información
    tJuego juego;
    ifstream archivo;

    bool finPartida = false, pasaTurno = true, jugada = false; //"jugada" indica si existe una jugada forzosa (intervienen el procesa5 o procesa6); "puede"indica si la ficha actual se puede mover o no
    juego.premio = 0, juego.seises = 0, juego.ultimaFichaMovida = -1; //inicializamos ultimaFichaMovida a -1 para no encontrarnos con el fallo de falta de inicializacion de la variable; además así podremos comprobar si es la primera ficha que se va a mover en el turno o ya se han movido otras fichas antes

    iniciaColores();
    iniciar(juego);

    if (DEBUG) {
        archivo.open(ARCHIVO);
        //funcion para debuggear el código 
        if (archivo.is_open()) {
            cargar(juego, archivo);
        }
        else {
            cout << "No se pudo abrir el archivo" << endl;
        }
    }
    mostrar(juego);

    while (!finPartida) {
        setColor(juego.jugadorTurno);
        cout << "Turno para el jugador " << colorACadena(juego.jugadorTurno) << endl;
        //si ultimaFichaMovida está en el -1, hemos estipulado que aún no se ha movido ninguna ficha en el turno de ese jugador
        ////si hay premio, el movimiento será este; si no lo hay, el movimiento vendrá dado por una tirada de dados aleatoria
        jugada = false; //se pone jugada = false por si ha habido una jugada forzosa en la última iteración, para que no interrumpa el código
        if (juego.premio > 0) {
            cout << "Ha de contar " << juego.premio << "!" << endl;
            juego.tirada = juego.premio;
            juego.premio = 0;
        }
        else {
            if (DEBUG) {
                if (archivo.is_open()) { //el archivo solo estará abierto mientras se puedan seguir leyendo valores; cuando leamos el -1 cerraremos el archivo
                    archivo >> juego.tirada; //cada valor del archivo representa un movimiento
                    if (juego.tirada == -1) { //cuando leemos el -1 en el archivo
                        archivo.close(); //primero cerraremos el archivo
                        //ahora pediremos al usuario que inserte un número, ya que si no lo hiciéramos, saldría tirada -1
                        cout << "Dame tirada (0 para salir): ";
                        cin >> juego.tirada;
                        if (juego.tirada == 0) {
                            return 0; //salir de la ejecución del programa
                        }
                    }
                }
                else { //si ya se han leído todos los números del archivo o el archivo no se ha podido abrir, pediremos al usuario que inserte los valores
                    cout << "Dame tirada (0 para salir): ";
                    cin >> juego.tirada;
                    if (juego.tirada == 0) {
                        return 0; //salir de la ejecución del programa
                    }
                }
            }
            else {
                juego.tirada = (rand() % 6) + 1;
            }
            cout << "Sale un " << juego.tirada << endl;
        }
        if ((cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, CASILLA_CASA) == 4) && juego.tirada != 5) { //si todas las fichas estan en casa y la tirada actual no es un 5
            cout << "El jugador tiene todas las fichas en casa... No puede mover" << endl;
            if ((juego.tirada == 6) && (juego.seises != 2)) { // si la tirada ha sido un 6 y no van ya dos seises consecutivos
                juego.seises++; //incrementamos el contador de seises
                pasaTurno = false; // no pasamos turno
            }
        }
        else if ((juego.tirada == 5) && (cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, -1) != 0)) { //en otro caso, si la tirada es 5
            jugada = procesa5(juego, pasaTurno);
        }
        else if (juego.tirada == 6) { //si la tirada es 6
            jugada = procesa6(juego, pasaTurno);
        }
        if (!jugada) { //si ni pocesa5 ni procesa6 han hecho una jugada forzosa
            pasaTurno = jugar(juego, finPartida);
        }
        if (!finPartida) { //si el usuario no ha decidido finalizar la partida
            system("pause");
            mostrar(juego);
            if (cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, CASILLA_META) == 4) { //si el jugador tiene todas las fichas en la meta
                setColor(juego.jugadorTurno);
                cout << "El jugador " << colorACadena(juego.jugadorTurno) << " ha ganado la partida! Enhorabuena!" << endl;
                finPartida = true;
                system("pause");
            }
            else if (pasaTurno) {
                juego.jugadorTurno = tColor((int(juego.jugadorTurno) + 1) % NUM_JUGADORES);
                juego.seises = 0;
                juego.ultimaFichaMovida = -1; //le volvemos a dar el valor -1 a ultimaFichaMovida para que no se confunda con la ultima ficha movida del jugador anterior
            }
        }
    }
    return 0;
}
//devuelve true si esa casilla es un seguro.
bool esSeguro(int casilla) {
    bool seguro;
    if (casilla == 0 || casilla == 5 || casilla == 12 || casilla == 17 || casilla == 22 || casilla == 29 || casilla == 34 || casilla == 39 || casilla == 46 || casilla == 51 || casilla == 56 || casilla == 63) {
        seguro = true;
    }
    else {
        seguro = false;
    }

    return seguro;
}
//recibe el tColor de un jugador y devuelve un int que indica la zanata de ese tColor.
int zanataJugador(tColor color) {
    int casillaZanata = -1;

    switch (color) {
    case Rojo:
        casillaZanata = 34;
        break;
    case Verde:
        casillaZanata = 51;
        break;
    case Azul:
        casillaZanata = 17;
        break;
    case Amarillo:
        casillaZanata = 0;
        break;
    default:
        break;
    }

    return casillaZanata;
}
//recibe el tColor de un jugador y devuelve un int que indica la casilla salida de ese tColor
int salidaJugador(tColor jugador) {
    int casillaSalida = -1;

    switch (jugador) {
    case Rojo:
        casillaSalida = 39;
        break;
    case Verde:
        casillaSalida = 56;
        break;
    case Azul:
        casillaSalida = 22;
        break;
    case Amarillo:
        casillaSalida = 5;
        break;
    default:
        break;
    }

    return casillaSalida;
}
//devuelve el nombre del color
string colorACadena(tColor color) {
    string stringColor;

    if (color == Rojo) {
        stringColor = "Rojo";
    }
    else if (color == Verde) {
        stringColor = "Verde";
    }
    else if (color == Azul) {
        stringColor = "Azul";
    }
    else if (color == Amarillo) {
        stringColor = "Amarillo";
    }

    return stringColor;
}
//devuelve el numero de fichas que hay en una casilla insertada por el usuario
int cuantasEn(const tFichas jugador, int casilla) {
    int contador = 0;

    for (int i = 0; i < NUM_FICHAS; i++) {
        if (jugador[i] == casilla) {
            contador++;
        }
    }
    return contador;
}
//devuelve el menor indice de las fichas del jugador que estan en esa casilla
int primeraEn(const tFichas jugador, int casilla) {
    bool encontrado = false;
    int i = 0, primeraFicha = -1;
    while (!encontrado && i < NUM_FICHAS) {
        if (jugador[i] == casilla) {
            primeraFicha = i;
            encontrado = true; //en el momento que el valor de una de las celdas de tFicha es el mismo que el de la casilla introducida, devolver el valor del indice (el return hace que se salga de la funcion)
        }
        i++;
    }

    return primeraFicha; //en caso de que no se encuentre ninguna ficha en esa casilla, devolvera -1 (el return hace que se salga de la funcion)
}
//devuelve el mayor indice de las fichas del jugador que estan en esa casilla (como solo puede haber un maximo de dos fichas pro casilla, devolverá la segunda, que es la última)
int segundaEn(const tFichas jugador, int casilla) {
    bool encontrado = false;
    int i = NUM_FICHAS - 1, ultimaFicha = -1;

    while (!encontrado && i >= 0) {
        if (jugador[i] == casilla) {
            ultimaFicha = i;
            encontrado = true; //en el momento que el valor de una de las celdas de tFicha es el mismo que el de la casilla introducida, devolver el valor del indice (el return hace que se salga de la funcion)
        }
        i--;
    }

    return ultimaFicha; //en caso de que no se encuentre ninguna ficha en esa casilla, devolvera -1 (el return hace que se salga de la funcion)
}
//inicia la partida
void iniciar(tJuego& juego) {
    srand(time(NULL)); //inicializar el generador de numeros automaticos
    setColor(Gris); //inicializar el color de las casillas a gris
    for (int i = 0; i < NUM_JUGADORES; i++) {
        for (int j = 0; j < NUM_FICHAS; j++) {
            juego.jugadores[i].fichas[j] = -1; //bucle doble para asignar todos los valores a -1 (están dentro de una matriz)
        }
    }
    for (int i = 0; i < NUM_CASILLAS; i++) { //bucle para asignar colores a las calles
        juego.casillas[i].calle1 = Ninguno;
        juego.casillas[i].calle2 = Ninguno;
    }
    juego.jugadorTurno = tColor(rand() % NUM_JUGADORES); //elegir el turno del jugador
}
//saca una ficha de la casa de JugadorTurno, sin tener en cuenta el hecho de que no haya fichas en casa o no se pueda sacar ninguna porque hay un puente
void saleFicha(tJuego& juego) {
    int salidaJugadorActual, primeraFichaEnCasa;
    salidaJugadorActual = salidaJugador(juego.jugadorTurno);
    primeraFichaEnCasa = primeraEn(juego.jugadores[juego.jugadorTurno].fichas, -1); //-1 es la casa, habría que buscar ahí la primera ficha

    juego.casillas[salidaJugadorActual].calle2 = juego.casillas[salidaJugadorActual].calle1;
    juego.casillas[salidaJugadorActual].calle1 = juego.jugadorTurno;

    juego.jugadores[juego.jugadorTurno].fichas[primeraFichaEnCasa] = salidaJugadorActual;

}
//envía a casa a la ficha que esté en esa casilla de la calle2
void aCasita(tJuego& juego, int casilla) {
    tColor colorFichaAComer = juego.casillas[casilla].calle2;
    int numeroFichaAComer = segundaEn(juego.jugadores[colorFichaAComer].fichas, casilla);

    juego.jugadores[colorFichaAComer].fichas[numeroFichaAComer] = -1;
    juego.casillas[casilla].calle2 = Ninguno;
}
//procesa los casos que pueden ocurrir si sale un 5
bool procesa5(tJuego& juego, bool& pasaTurno) {
    int salidaJugadorActual = salidaJugador(juego.jugadorTurno);
    bool jugadaForzosa = true; //variable que devolverá si se ha realizado una jugada forzosa en el procesa5 (solo devolverá false cuando no se pueda salir de casa)
    if (juego.casillas[salidaJugadorActual].calle2 == Ninguno) { //si en la calle 2 de la salida del jugador actual no hay ninguna ficha
        saleFicha(juego);
        pasaTurno = true;
    }
    else if (juego.casillas[salidaJugadorActual].calle2 == juego.jugadorTurno && juego.casillas[salidaJugadorActual].calle1 == juego.jugadorTurno) { //si tanto en la calle 1 como en la calle 2 hay fichas del jugador que sale
        pasaTurno = true;
        jugadaForzosa = false; //unico caso en el que se devuelve false
    }
    else if (juego.casillas[salidaJugadorActual].calle1 != juego.jugadorTurno && juego.casillas[salidaJugadorActual].calle2 == juego.jugadorTurno) { //si en la calle 1 hay una ficha de otro color y en la calle 2 hay una ficha del color del jugador que sale
        //se cambia la ficha que hay en calle 1 con la que hay en calle 2 y se come la ficha del color que no corresponde
        tColor temp = juego.casillas[salidaJugadorActual].calle2;
        juego.casillas[salidaJugadorActual].calle2 = juego.casillas[salidaJugadorActual].calle1;
        juego.casillas[salidaJugadorActual].calle1 = temp;
        tColor jugadorComido = juego.casillas[salidaJugadorActual].calle2; //guardamos el color del jugador que comemos para mostrarlo por pantalla
        aCasita(juego, salidaJugadorActual);
        saleFicha(juego);
        cout << "Ha comido una ficha del jugador " << colorACadena(jugadorComido) << endl;
        juego.premio = 20;
        pasaTurno = false;
    }
    else { //resto de casos (tanto si hay una ficha del color del jugador actual en la calle1 y otra en la calle 2 como si ambas calles tienen fichas de colores diferentes al de la salida)
        tColor jugadorComido = juego.casillas[salidaJugadorActual].calle2; //guardamos el color del jugador que comemos para mostrarlo por pantalla
        aCasita(juego, salidaJugadorActual);
        saleFicha(juego);
        cout << "Ha comido una ficha del jugador " << colorACadena(jugadorComido) << endl;
        juego.premio = 20;
        pasaTurno = false;
    }
    return jugadaForzosa; //si no ha llegado al if que pone la variable jugadaForzosa a false, esta variable será true (se devuelve true en todos los casos excepto en el que está específicamente diseñado para devolver false)
}
//indica si en la casilla hay un puente
bool puente(tCasillas casillas, int casilla) {
    if ((casillas[casilla].calle1 == casillas[casilla].calle2) && (casillas[casilla].calle1 != Ninguno)) { //si el color de la ficha en calle 1 en la casilla indicada es el mismo que el color de la ficha en calle2 en esa misma casilla (y no son Ninguno), devolverá true (hay un puente)
        return true;
    }
    return false; //si no hay dos fichas del mismo color en la misma casilla, devolverá false (no hay puente)
}
//indica si la ficha del jugadorTurno puede avanzar la tirada
bool puedeMover(tJuego& juego, int ficha, int& casilla) {
    int movimientos = 0;
    int casillaActual = casilla;
    bool hayPuente, haySeguro, puedeMover = true, finMovimientos = false;

    while (puedeMover && !finMovimientos) { //se ejecuta mientras se pueda mover la ficha y no se haya llegado al fin de movimientos
        if (casillaActual == zanataJugador(juego.jugadorTurno)) {
            casillaActual = CASILLA1_RECTA_META; //si la ficha actual es la zanata, la siguiente sera la primera casilla de la recta final a la meta
            if ((cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, casillaActual) == 2) && (movimientos == (juego.tirada - 1))) { //si justo en la primera casilla de la recta final a meta ya hay dos fichas y el movimiento es el ultimo no se podrá mover, pero si podrá saltarlas
                puedeMover = false;
            }
        }
        else if (casillaActual >= CASILLA1_RECTA_META) { //si estamos en la recta final
            casillaActual++; //avanzaremos una posicion
            if (((casillaActual == CASILLA_META) && (movimientos < (juego.tirada - 1))) || casillaActual > CASILLA_META) { //si la casillaActual es la meta y no es el ultimo movimiento, puedeMover() devolverá false
                puedeMover = false;
            }
            if ((cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, casillaActual) == 2) && (movimientos == (juego.tirada - 1)) && (casillaActual != 108)) { //si hay dos fichas en la misma casilla de la recta final se podrán saltar pero no se puede caer en esa casilla
                puedeMover = false;
            }
        }
        else if ((casillaActual >= 0) && (casillaActual < NUM_CASILLAS)) {
            casillaActual = ((casillaActual + 1) % NUM_CASILLAS); //si estamos en las calles, avanzamos una posicion (teniendo en cuenta que la última casilla es la 67 y la siguiente es la 0
            if ((juego.casillas[casillaActual].calle1 != Ninguno) && (juego.casillas[casillaActual].calle2 != Ninguno && (movimientos == (juego.tirada - 1)))) { //si en la casillaActual ya hay dos fichas y es el ultimo movimiento, puedeMover() devolverá false
                puedeMover = false;
            }
            hayPuente = puente(juego.casillas, casillaActual);
            haySeguro = esSeguro(casillaActual);
            if (hayPuente && haySeguro) { //si en la casilla actual hay un puente y es un seguro, no podemos pasar y puedeMover() devolverá false
                puedeMover = false;
            }
        }
        else { //todos los casos que no se correspondan con los anteriores (que esté en casa) devolveran false
            puedeMover = false;
        }

        movimientos++; //se suma un movimiento
        if (movimientos == juego.tirada) { //si el numero de movimientos es igual a la tirada, se habra llegado al final
            finMovimientos = true; //pondremos finMovimientos a true y se saldrá del bucle
        }
    }

    casilla = casillaActual;
    return puedeMover;
}
//Se llamará solo cuando puedeMover() devuelva true y moverá la ficha que seleccione el usuario
void mover(tJuego& juego, int ficha, int casilla) {
    int casillaActual = juego.jugadores[juego.jugadorTurno].fichas[ficha]; //la casilla en la que se encuentra la ficha antes de moverla

    if (juego.casillas[casillaActual].calle1 == juego.jugadorTurno) { //si el jugador actual está en la calle 1
        juego.casillas[casillaActual].calle1 = juego.casillas[casillaActual].calle2; //el valor de la calle 1 pasará a ser el de la ficha que haya en calle 2 (si no hay ninguna pasara a ninguno)
        juego.casillas[casillaActual].calle2 = Ninguno; //el valor de la calle 2 pasará a ninguno (si no hubera ninguna ficha no pasaría nada)
    }
    else if (juego.casillas[casillaActual].calle2 == juego.jugadorTurno) { //si el jugador actual está en la calle 2
        juego.casillas[casillaActual].calle2 = Ninguno; //el valor de la calle 2 pasará a valer ninguno y la calle 1 se quedará igual
    }

    juego.jugadores[juego.jugadorTurno].fichas[ficha] = casilla; //movemos la ficha actual del jugador actual a la casilla resultante de la tirada

    if (casilla == CASILLA_META) { //si la casilla a la que se mueve es la meta (casilla 108)
        cout << "La ficha del jugador ha llegado a la meta" << endl;
        juego.premio = 10; //se le otorgará un premio de 10 movimientos
    }
    else if (casilla < NUM_CASILLAS && juego.casillas[casilla].calle1 != Ninguno) { //si la casilla a la que se mueve está en la calle y en la calle1 ya hay alguna ficha
        juego.casillas[casilla].calle2 = juego.casillas[casilla].calle1; //pase lo que pase, la ficha que estaba en la calle 1 siempre ira a la calle 2
        if (esSeguro(casilla) || (juego.casillas[casilla].calle1 == juego.jugadorTurno)) { //si es un seguro la casilla en la que caemos o ya hay una ficha del mismo color
            juego.casillas[casilla].calle1 = juego.jugadorTurno;
        }
        else { //si la ficha que estaba en la calle1 no era del mismo color y no estaba en un seguro (calle1[casilla] != jugadorTurno), comeremos al jugador
            tColor jugadorComido = juego.casillas[casilla].calle2; //guardamos el color del jugador que comemos para mostrarlo por pantalla
            aCasita(juego, casilla);
            juego.casillas[casilla].calle1 = juego.jugadorTurno;
            juego.premio = 20;
            cout << "Ha comido una ficha del jugador " << colorACadena(jugadorComido) << endl;
        }
    }
    else { //si no hay ninguna ficha en la calle1 de la casilla en la que cae
        juego.casillas[casilla].calle1 = juego.jugadorTurno;
    }
}
//se llamará cuando el jugador esté forzado a abrir un puente, debido a que ha sacado un 6
void abrirPuente(tJuego& juego, int casillaOrig, int casillaDest) {
    int segundaFichaPuente = segundaEn(juego.jugadores[juego.jugadorTurno].fichas, casillaOrig);
    cout << "Se abre el puente de la casilla " << casillaOrig << endl;
    mover(juego, segundaFichaPuente, casillaDest);
    juego.ultimaFichaMovida = segundaFichaPuente;
}
//procesará una jugada de 6 para el jugadorTurno
bool procesa6(tJuego& juego, bool& pasaTurno) {
    bool jugadaForzosa = false; //variable que indica si se ha realizado una jugada forzosa durante el procesa6 (lo inicializamos a false porque nos deba runtime errors al no estar inicializada (pese a que todos los casos posibles están especificados))
    juego.seises++; //incrementamos el contador de seises
    int fichasEnCasa = cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, CASILLA_CASA); //calculamos el numero de fichas que el jugador tiene en casa

    if (fichasEnCasa == 0) { //si no hay ninguna ficha en casa
        juego.tirada = 7; //la tirada pasará a valer 7
        cout << "Como no tiene fichas en casa, cuenta 7!" << endl;
    }

    if (juego.seises == 3) { //si ha habido 3 seises seguidos
        int  casillaInicial = juego.jugadores[juego.jugadorTurno].fichas[juego.ultimaFichaMovida]; //calculamos la casilla en la que se encuentra la ultima ficha movida

        if (casillaInicial > 100) { //si la ultima ficha movida esta en la subida a meta
            cout << "Tercer seis consecutivo... La ultima ficha movida se salva por estar en la subida a meta!" << endl;
            pasaTurno = true;
            jugadaForzosa = true;
        }
        else { //si la ultima ficha movida no esta en la recta final a meta
            cout << "Tercer seis consecutivo... La ultima ficha movida se va a casa!" << endl;

            tColor temp = juego.casillas[casillaInicial].calle2; //cambiamos la ficha que pueda haber en calle 2 para poder mandar la recién llegada a casa
            juego.casillas[casillaInicial].calle2 = juego.casillas[casillaInicial].calle1;
            juego.casillas[casillaInicial].calle1 = temp;
            aCasita(juego, casillaInicial); //mandamos a su casa a la ultima ficha movida, la cual ya hemos puesto e calle2 previamente

            pasaTurno = true;
            jugadaForzosa = true;
        }
    }
    else { //si no ha habido 3 seises seguidos
        pasaTurno = false;
        //ahora comprobaremos la apertura forzosa de un puente
        int casillaPuente1 = -1, casillaPuente2 = -1; //inicializamos los valores de las casillasPuente a -1 (para que no confundamos si tienen puente realmente o no)
        bool hayPuente, notCasillaPuente1;
        for (int i = 0; i < NUM_FICHAS; i++) { //buscamos entre todas las fichas del jugador
            hayPuente = puente(juego.casillas, juego.jugadores[juego.jugadorTurno].fichas[i]);
            notCasillaPuente1 = juego.jugadores[juego.jugadorTurno].fichas[i] != casillaPuente1;

            if (hayPuente && notCasillaPuente1) { //si existe un puente en alguna casilla y esta es diferente a la casilla del puente 1 (por eso inicializamos casillaPuente1 a -1)
                casillaPuente2 = casillaPuente1; //casillaPuente2 ahora tendrá el valor que tenía casillaPuente1 (si esta tuviera -1 no pasa nada pero si esta ya tuviera una casilla en la que hay un puente, obtendrá ese nuevo valor)
                casillaPuente1 = juego.jugadores[juego.jugadorTurno].fichas[i]; //guardamos la nueva casilla en la que hay un puente en casillaPuente1
            }
        }
        if (casillaPuente1 > casillaPuente2 && (casillaPuente2 != -1)) { //si el valor de casillaPuente1 es mayor que casillaPuente2 (siempre que la casillaPuente2 NO sea -1), cambiaremos el orden (para así tener en casillaPuente1 la casilla con menos número)
            int temp = casillaPuente1;
            casillaPuente1 = casillaPuente2;
            casillaPuente2 = temp;
        }
        if (casillaPuente1 != -1 && casillaPuente2 == -1) { //si solo hay un puente (casillaPuente2 aun tiene el valor -1 y casillaPuente1 tiene un valor distinto a -1)
            if (puedeMover(juego, juego.casillas[casillaPuente1].calle2, casillaPuente1)) {
                abrirPuente(juego, casillaPuente1 - juego.tirada, casillaPuente1); //tenemos que poner casillaPuente1 - tirada porque puedeMover() ha actualizado el valor de casillaPuente1 a casillaPuente1 + tirada
                jugadaForzosa = true;
            }
            else {
                jugadaForzosa = false;
            }
        }
        else if (casillaPuente1 != -1 && casillaPuente2 != -1) { //si hay dos puentes (ambas casillas son distintas de -1)
            bool puedeMoverPuente1, puedeMoverPuente2;
            puedeMoverPuente1 = puedeMover(juego, juego.casillas[casillaPuente1].calle2, casillaPuente1);
            puedeMoverPuente2 = puedeMover(juego, juego.casillas[casillaPuente2].calle2, casillaPuente2);

            if (puedeMoverPuente1 && puedeMoverPuente2) {
                jugadaForzosa = false; //el codigo para que el jugador decida que puente abrir se ejecutará fuera del procesa6
            }
            else if (puedeMoverPuente1 && !puedeMoverPuente2) { //si puede mover el puente 1 pero no el 2
                abrirPuente(juego, casillaPuente1 - juego.tirada, casillaPuente1); //tenemos que poner casillaPuente1 - tirada porque puedeMover() ha actualizado el valor de casillaPuente1 a casillaPuente1 + tirada
                jugadaForzosa = true;
            }
            else if (!puedeMoverPuente1 && puedeMoverPuente2) { //si puede mover el puente 2 pero no el 1
                abrirPuente(juego, casillaPuente2 - juego.tirada, casillaPuente2); //tenemos que poner casillaPuente2 - tirada porque puedeMover() ha actualizado el valor de casillaPuente1 a casillaPuente2 + tirada
                jugadaForzosa = true;
            }
            else { // si no se puede mover ningun puente
                cout << "No se puede abrir ninguno de los dos puentes! Se ignora la tirada..." << endl;
                jugadaForzosa = true; //como no se puede hecer ninguno movimiento, resultaría redundante poner en el main que ninguna ficha se puede mover, por lo que le damos a jugadaForzosa el valor de true
            }
        }

    }
    return jugadaForzosa;
}
//devuelve true si todas las fichas del jugador están en la meta y false en caso contrario
bool todasEnMeta(const tFichas jugador) {
    bool partidaGanada;
    if (cuantasEn(jugador, CASILLA_META) == 4) { //si las 4 fichas están en la meta del jugador
        partidaGanada = true;
    }
    else {
        partidaGanada = false;
    }
    return partidaGanada;
}
//mover, si es posible, alguna ficha del jugadorTurno
bool jugar(tJuego& juego, bool& fin) {
    bool pasaraTurno = true; //variable booleana que devolvera la funcion; será true si pasa turno y false si no
    int fichasQuePuedoMover = 0, fichaPuedoMover;
    for (int f = 0; f < NUM_FICHAS; f++) { //bucle para contabilizar las fichas que puedo mover
        int casilla = juego.jugadores[juego.jugadorTurno].fichas[f];
        if (puedeMover(juego, f, casilla)) {
            fichasQuePuedoMover++;
            fichaPuedoMover = f; //esta variable guardará la última ficha qu puedo mover (nos resultará útil para el caso en que solo podemos mover una ficha, porque nos ahorraremos volver a hacer el bucle ya que ya tenemos el valor de la ficha guardado en esta variable)
        }
    }
    if (fichasQuePuedoMover == 0) { //si no puedo mover ninguna ficha
        cout << "No se puede mover ninguna de las fichas." << endl;
    }
    else if (fichasQuePuedoMover == 1) { //si solo puedo mover una ficha
        int casilla = juego.jugadores[juego.jugadorTurno].fichas[fichaPuedoMover]; //declaramos la casilla a la que nos vamos a mover (esta casilla se modificará una vez entre en la función puedeMover()
        int casillaInicial; //declaramos la casilla de la que partimos
        if (puedeMover(juego, fichaPuedoMover, casilla)) { //esta funcion nos dara nuestra casilla de destino, ya que ya sabemos que si podemos mover esta ficha
            casillaInicial = juego.jugadores[juego.jugadorTurno].fichas[fichaPuedoMover]; //la casilla inicial será la posicion actual de la ficha
        }
        mover(juego, fichaPuedoMover, casilla);
        cout << "Se mueve la ficha " << fichaPuedoMover + 1 << " de la casilla " << casillaInicial << " a la casilla " << casilla << endl;
        juego.ultimaFichaMovida = fichaPuedoMover;
    }
    else { //si puedo mover dos o mas fichas
        int fichaAMover, casillaAMover; //iniciamos fichaAMover y casillaAMover para que no de errores el do while
        do {
            cout << "Por favor, elige la ficha que quieres mover..." << endl;
            for (int f = 0; f < NUM_FICHAS; f++) { //bucle para mostrar el menú de movimiento al usuario
                int casilla = juego.jugadores[juego.jugadorTurno].fichas[f];
                if (puedeMover(juego, f, casilla)) { //puedeMover() transforma el valor inicial de casilla (que era la casilla en la que se encontraba la ficha) a la casilla en la que acabaría tras la tirada
                    int casillaActual = juego.jugadores[juego.jugadorTurno].fichas[f];
                    cout << f + 1 << ": De la casilla " << casillaActual << " a la casilla " << casilla << endl;
                }
            }
            cout << "Ficha (0 para salir): ";
            cin >> fichaAMover;
            if (fichaAMover == 0) {
                fin = true;
            }
            fichaAMover = fichaAMover - 1;
            casillaAMover = juego.jugadores[juego.jugadorTurno].fichas[fichaAMover];
        } while (!puedeMover(juego, fichaAMover, casillaAMover) && fichaAMover != -1); //pedir de nuevo el numero al usuario cada vez que introduzca un numero que no es correcto
        if (fichaAMover != -1) {
            mover(juego, fichaAMover, casillaAMover);
            juego.ultimaFichaMovida = fichaAMover;
        }
    }
    if (juego.premio > 0 || juego.tirada == 6 || juego.tirada == 7 || (juego.tirada > 7 && juego.seises > 0)) { //si se acaba de ganar un premio, o se acaba de jugar un 6/7 o un premio y el numero de seises es > 0, la funcion devolvera false
        pasaraTurno = false;
    }
    else { //en caso contrario, se reseteará el contador de seises y la función devolverá true
        juego.seises = 0;
        pasaraTurno = true;
    }
    return pasaraTurno;
}
//cargar distintos casos de prueba con un fichero externo
void cargar(tJuego& juego, ifstream& archivo) {
    int jugador, casilla;

    for (int i = 0; i < NUM_JUGADORES; i++) {
        for (int f = 0; f < NUM_FICHAS; f++) {
            archivo >> casilla;
            juego.jugadores[i].fichas[f] = casilla;
            if ((casilla >= 0) && (casilla < NUM_CASILLAS)) {
                if (juego.casillas[casilla].calle1 == Ninguno) {
                    juego.casillas[casilla].calle1 = tColor(i);
                }
                else {
                    juego.casillas[casilla].calle2 = tColor(i);
                }
            }
        }
    }
    archivo >> jugador;
    juego.jugadorTurno = tColor(jugador);
}


/*
**********************************************************************
**********************************************************************
A PARTIR DE AQUI VA EL CODIGO QUE SIRVE PARA REPRESENTAR EL TABLERO
**********************************************************************
**********************************************************************
*/

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif


// IMPLEMENTACIÓN DE LOS SUBPROGRAMAS DE VISUALIZACIÓN DEL TABLERO


void mostrar(const tJuego& juego) {
    int casilla, ficha;
    tColor jug;

    cout << "\x1b[2J\x1b[H"; // Se situa en la esquina superior izquierda
    setColor(Gris);
    cout << endl;

    // Filas con la numeración de las casillas...
    for (int i = 0; i < NUM_CASILLAS; i++)
        cout << i / 10;
    cout << endl;
    for (int i = 0; i < NUM_CASILLAS; i++)
        cout << i % 10;
    cout << endl;

    // Borde superior...
    for (int i = 0; i < NUM_CASILLAS; i++)
        cout << '>';
    cout << endl;

    // Primera fila de posiciones de fichas...
    for (int i = 0; i < NUM_CASILLAS; i++) {
        setColor(juego.casillas[i].calle2);
        if (juego.casillas[i].calle2 != Ninguno)
            cout << primeraEn(juego.jugadores[juego.casillas[i].calle2].fichas, i) + 1;
        else
            cout << ' ';
        setColor(Gris);
    }
    cout << endl;

    // "Mediana"   
    for (int i = 0; i < NUM_CASILLAS; i++)
        if (esSeguro(i))
            cout << 'o';
        else
            cout << '-';
    cout << endl;


    // Segunda fila de posiciones de fichas...
    for (int i = 0; i < NUM_CASILLAS; i++) {
        setColor(juego.casillas[i].calle1);
        if (juego.casillas[i].calle1 != Ninguno)
            cout << segundaEn(juego.jugadores[juego.casillas[i].calle1].fichas, i) + 1;
        else
            cout << ' ';
        setColor(Gris);
    }
    cout << endl;

    jug = Amarillo;
    // Borde inferior...
    for (int i = 0; i < NUM_CASILLAS; i++)
        if (i == zanataJugador(jug)) {
            setColor(jug);
            cout << "V";
            setColor(Gris);
        }
        else if (i == salidaJugador(jug)) {
            setColor(jug);
            cout << "^";
            setColor(Gris);
            jug = tColor(int(jug) + 1);
        }
        else
            cout << '>';
    cout << endl;

    // Metas y casas...
    for (int i = 0; i < NUM_FICHAS; i++) {
        casilla = 0;
        jug = Amarillo;
        setColor(jug);
        while (casilla < NUM_CASILLAS) {
            if (casilla == zanataJugador(jug)) {
                ficha = primeraEn(juego.jugadores[jug].fichas, 101 + i);
                if (ficha != -1) {
                    cout << ficha + 1;
                    if (cuantasEn(juego.jugadores[jug].fichas, 101 + i) > 1) {
                        ficha = segundaEn(juego.jugadores[jug].fichas, 101 + i);
                        if (ficha != -1) {
                            cout << ficha + 1;
                        }
                        else
                            cout << "V";
                    }
                    else
                        cout << "V";
                }
                else
                    cout << "VV";
                casilla++;
            }
            else if (casilla == salidaJugador(jug)) {
                if (juego.jugadores[jug].fichas[i] == -1) // En casa
                    cout << i + 1;
                else
                    cout << "^";
                jug = tColor(int(jug) + 1);
                setColor(jug);
            }
            else
                cout << ' ';
            casilla++;
        }
        cout << endl;
    }

    // Resto de metas...
    for (int i = 105; i <= 107; i++) {
        casilla = 0;
        jug = Amarillo;
        setColor(jug);
        while (casilla < NUM_CASILLAS) {
            if (casilla == zanataJugador(jug)) {
                ficha = primeraEn(juego.jugadores[jug].fichas, i);
                if (ficha != -1) {
                    cout << ficha + 1;
                    if (cuantasEn(juego.jugadores[jug].fichas, i) > 1) {
                        ficha = segundaEn(juego.jugadores[jug].fichas, i);
                        if (ficha != -1) {
                            cout << ficha + 1;
                        }
                        else
                            cout << "V";
                    }
                    else
                        cout << "V";
                }
                else
                    cout << "VV";
                casilla++;
                jug = tColor(int(jug) + 1);
                setColor(jug);
            }
            else
                cout << ' ';
            casilla++;
        }
        cout << endl;
    }

    casilla = 0;
    jug = Amarillo;
    setColor(jug);
    while (casilla < NUM_CASILLAS) {
        cout << ((juego.jugadores[jug].fichas[0] == 108) ? '1' : '.');
        cout << ((juego.jugadores[jug].fichas[1] == 108) ? '2' : '.');
        jug = tColor(int(jug) + 1);
        setColor(jug);
        cout << "               ";
        casilla += 17;
    }
    cout << endl;
    casilla = 0;
    jug = Amarillo;
    setColor(jug);
    while (casilla < NUM_CASILLAS) {
        cout << ((juego.jugadores[jug].fichas[2] == 108) ? '3' : '.');
        cout << ((juego.jugadores[jug].fichas[3] == 108) ? '4' : '.');
        jug = tColor(int(jug) + 1);
        setColor(jug);
        cout << "               ";
        casilla += 17;
    }
    cout << endl << endl;
    setColor(Gris);
}

void setColor(tColor color) {
    switch (color) {
    case Azul:
        cout << "\x1b[34;107m";
        break;
    case Verde:
        cout << "\x1b[32;107m";
        break;
    case Rojo:
        cout << "\x1b[31;107m";
        break;
    case Amarillo:
        cout << "\x1b[33;107m";
        break;
    case Gris:
    case Ninguno:
        cout << "\x1b[90;107m";
        break;
    }
}

void iniciaColores() {
#ifdef _WIN32
    for (DWORD stream : {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE}) {
        DWORD mode;
        HANDLE handle = GetStdHandle(stream);

        if (GetConsoleMode(handle, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(handle, mode);
        }
    }
#endif
}