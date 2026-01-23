// definicje / korzystanie z pamieci

#ifndef COMMON_H
#define COMMON_H

// limity (max osob i wiek)
const int LIMIT_ROUTE_1 = 10;
const int LIMIT_ROUTE_2 = 15;
const int LIMIT_BRIDGE = 3;

const int AGE_SENIOR = 75;     // seniorzy >75 lat
const int AGE_CHILD = 8;       // dzieci <8 lat

// klucze
const int KEY_SEM = 1111;
const int KEY_SHM = 2222;
const int KEY_MSG = 3333;

const int MSG_TICKET = 1; // typ wiadomosci

// shared memory
struct CaveState {

    // ile osob przebywa na trasie
    int people_on_route1;
    int people_on_route2;
    int people_on_bridge;

    int bridge_direction;

    int tickets_sold;
    int tickets_free; // osoby ktore za darmo wchodza

    int is_open;

    // sygnaly dla pliku straznik
    int route1_open; // 1 = otwarta, 0 = zamknieta - sygnal 1
    int route2_open; // 1 = otwarta, 0 = zamknieta - sygnal 2
    int pid_przewodnik; // PID przewodnika do wysylania sygnalow
    int pid_manager; // PID managera do wysylania sygnalu konca
};

// message
struct TicketMessage {
    long mtype;
    int visitor_id;
    int age;
    int route_choice;
    int has_guardian;
    int is_repeater;
};

#endif