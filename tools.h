// Plik naglowkowy z deklaracjami funkcji pomocniczych do obslugi semaforow,
// pamieci dzielonej i kolejek komunikatow.

#ifndef TOOLS_H
#define TOOLS_H

#include "common.h"

// Kolo kolorowe logi w terminalu (ANSI)
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// SEMAFORY	- deklaracje funkcji
// Tworzy semafory
int create_semaphores(int key, int number);

// Usuwa semafory
void remove_semaphores(int sem_id);

// Operacje na semaforach - blokowanie/odblokowywanie
void lock_sem(int sem_id, int sem_num);
void unlock_sem(int sem_id, int sem_num);

// Ustawianie wartosci semafora
void set_sem_value(int sem_id, int sem_num, int val);

// Pobieranie wartosci semafora
int get_sem_value(int sem_id, int sem_num);

// PAMIEC DZIELONA - deklaracje funkcji
// Tworzy segment pamieci dzielonej
int create_shared_memory(int key, int size);

// Oznacza segment pamieci do usuniecia
void remove_shared_memory(int shm_id);

// Podlacza pamiec dzielona do przestrzeni adresowej procesu
int* attach_memory(int shm_id);

// Odłącza pamiec dzielona od przestrzeni adresowej procesu
void detach_memory(int* addr);


// KOLEJKA KOMUNKATOW - deklaracje funkcji
// Tworzy kolejke komunikatow
int create_msg_queue(int key);

// Usuwa kolejke komunikatow
void remove_msg_queue(int msg_id);

// WYSYLANIE I ODBIERANIE BILETOW
void send_ticket(int msg_id, int sem_id, TicketMessage msg);
TicketMessage receive_ticket(int msg_id, int sem_id);

#endif