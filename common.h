// definicje / korzystanie z pamieci

#ifndef COMMON_H
#define COMMON_H

// limity (max osob/wiek)

const int LIMIT_ROUTE_1 = 10;
const int LIMIT_ROUTE_2 = 15;
const int LIMIT_BRIDGE = 3;

const int AGE_SENIOR = 75;     // seniorzy >75 lat
const int AGE_CHILD = 8;        // dzieci <8 lat

// klucze - proces musi znalezc semafor badz pamiec

const int KEY_SEM = 1111;
const int KEY_SHM = 2222;
const int KEY_MSG = 3333;

const int MSG_TICKET = 1; // typ wiadomosci do kolejki

// shared memory

struct CaveState {

    // ile osob przebywa na trasie
    int people_on_route1;
    int people_on_route2;
    int people_on_bridge;

    int bridge_direction; // 0 - pusta, 1 - wchodza, 2 - wychodza

    int tickets_sold;
    int is_open;    // 1 - jaskinia otwarta, 0 - zamknieta
};

// message

struct TicketMessage {

    long mtype;

    int visitor_id;   // id procesu
    int age;          // wiek turysty
    int route_choice; // trasa 1 albo 2
    int has_guardian; // 1 = ma opiekuna, 0 = sam
};

#endif