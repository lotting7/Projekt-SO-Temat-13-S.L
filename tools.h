// definicje funkcji

#ifndef TOOLS_H
#define TOOLS_H

#include "common.h"

// semafory

int create_semaphores(int key, int number); // tworzenie semaforow (id)
void remove_semaphores(int sem_id); // usuwanie semaforow
void lock_sem(int sem_id, int sem_num); // jesli semafor = 0, proces czeka
void unlock_sem(int sem_id, int sem_num); // zwalnia miejsce i zwieksza semafor


// shared memory

int create_shared_memory(int key, int size);
void remove_shared_memory(int shm_id);

int* attach_memory(int shm_id); // proces -> pamiec (zwraca adres do tablicy)
void detach_memory(int* addr); // odlacza proces od pamieci


// wiadomosci

int create_msg_queue(int key); // tworzenie kolejki komunikatow
void remove_msg_queue(int msg_id);

void send_ticket(int msg_id, TicketMessage msg); // wysylanie

TicketMessage receive_ticket(int msg_id); /// odbieranie wiadomosci

#endif