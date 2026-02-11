#ifndef COMMON_H
#define COMMON_H

// Maksymalna liczba osób na poszczególnych trasach i moście
const int LIMIT_ROUTE_1 = 10;
const int LIMIT_ROUTE_2 = 15;
const int LIMIT_BRIDGE = 3;

// Granice wieku
const int AGE_SENIOR = 75;
const int AGE_CHILD = 8;

// KLUCZE DO ZASOBOW IPC - Dzięki nim różne procesy znajdą te same zasoby
const int KEY_SEM = 1111; // semafory
const int KEY_SHM = 2222; // pamięc dzielona
const int KEY_MSG = 3333; // kolejka komunikatów

const int MSG_TICKET = 1; // typ wiadomości biletu normalnego

// INDEKSY SEMAFORÓW - do kontroli dostępu i zasobów

// chroni dostęp do pamięci dzielonej
const int SEM_ACCESS = 0;

// liczba wolnych miejsc na trasach i moście
const int SEM_ROUTE1 = 1;
const int SEM_ROUTE2 = 2;
const int SEM_BRIDGE = 3;

// semafor do logów (chroni dostęp do plików logów)
const int SEM_LOG = 4;

// semafory do zarządzania mostem
const int SEM_BRIDGE_ENTRY = 5;
const int SEM_BRIDGE_EXIT = 6;

// semafor do kontroli przestrzeni w kolejce komunikatów
const int SEM_QUEUE_SPACE = 7;

// liczba semaforów
const int SEM_COUNT = 8;

// maksymalna liczba wiadomości w kolejce
const int MSG_QUEUE_MAX = 3500;

// STRUKTURA PAMIECI DZIELONEJ JASKINI - do stanu jaskini
struct CaveState {
	// liczba osób na trasach i moście
    int people_on_route1;
    int people_on_route2;
    int people_on_bridge;

	// sterowanie mostem
    int bridge_direction;
    int bridge_waiting_in;
    int bridge_waiting_out;

 	// bilans biletów
    int tickets_sold;
    int tickets_free;

	// stan jaskini i tras
    int is_open;
    int route1_open;
    int route2_open;

	// PIDy procesów
    int pid_przewodnik;
    int pid_manager;
	int pid_straznik;
	int pid_kasjer;
	int pid_klient;
};

// STRUKTURA WIADOMOSCI BILETU - do kolejki komunikatów
struct TicketMessage {
    long mtype; // typ wiadomości (1 = VIP, 2 = normalny)
    int visitor_id; // PID odwiedzającego
    int age;
    int route_choice; // wybrana trasa (1 lub 2)
    int has_guardian; // czy ma opiekuna (0 = nie, 1 = tak)
    int is_repeater; // czy jest powracającym gościem (0 = nie, 1 = tak)

};

#endif