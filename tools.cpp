// funkcje z tools.h

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "tools.h"
#include <errno.h>

// SEMAFORY

// tworzenie semaforow
int create_semaphores(int key, int number) {

    int id = semget(key, number, 0600 | IPC_CREAT); // tworzenie semafora z podanym kluczem i liczba semaforow

    if (id == -1) {
        perror("Error creating semaphores!");
        exit(1);
    }
    return id;

}

// usuwanie semaforow
void remove_semaphores(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) == -1) { // usuwanie semafora
        perror("Error removing semaphores");
    }
}

// blokowanie semafora
void lock_sem(int sem_id, int sem_num) {
    struct sembuf op;
    op.sem_num = sem_num; // numer semafora w zestawie
    op.sem_op = -1; // zmniejszenie wartosci semafora o 1 (blokada)
    op.sem_flg = 0; // flagi operacji

    // wykonanie operacji semafory
    if (semop(sem_id, &op, 1) == -1) {
        // Ignorujemy błędy przy zamykaniu
        if (errno == EIDRM || errno == EINVAL) {
            exit(0);
        }
        perror("Error locking semaphore");
        exit(1);
    }
}

// odblokowanie semafora
void unlock_sem(int sem_id, int sem_num) {
    struct sembuf op; // struktura operacji semafora
    op.sem_num = sem_num; // numer semafora w zestawie
    op.sem_op = 1; // zwiekszenie wartosci semafora o 1 (odblokowanie)
    op.sem_flg = 0; // flagi operacji

    // wykonanie operacji semafory
    if (semop(sem_id, &op, 1) == -1) {
        // Ignorujemy błędy przy zamykaniu
        if (errno == EIDRM || errno == EINVAL) {
            exit(0);
        }
        perror("Error unlocking semaphore");
        exit(1);
    }
}

// SHARED MEMORY

//tworzenie pamieci
int create_shared_memory(int key, int size) {
    int id = shmget(key, size, 0600 | IPC_CREAT);
    if (id == -1) {
        perror("Error creating shared memory");
        exit(1);
    }
    return id;
}

// usuwanie pamieci
void remove_shared_memory(int shm_id) {
    shmctl(shm_id, IPC_RMID, NULL);
}

// podlaczanie procesu do pamieci
int* attach_memory(int shm_id) {

    void* addr = shmat(shm_id, NULL, 0); // NULL = system wybiera adres, 0 = read+write

    if (addr == (void*)-1) {
        perror("Error attaching memory");
        exit(1);
    }
    return (int*)addr; // zwracamy adres pamieci jako int*
}

// odlaczanie procesu od pamieci
void detach_memory(int* addr) {
    shmdt(addr);
}

// KOLEJKA WIADOMOSCI

// tworzenie kolejki
int create_msg_queue(int key) {
    int id = msgget(key, 0600 | IPC_CREAT);
    if (id == -1) {
        perror("Error creating queue");
        exit(1);
    }
    return id;
}

// usuwanie kolejki
void remove_msg_queue(int msg_id) {
    msgctl(msg_id, IPC_RMID, NULL);
}

// wysylanie "biletu"
void send_ticket(int msg_id, TicketMessage msg) {

    int size = sizeof(msg) - sizeof(long); // rozmiar bez pola mtype


    if (msgsnd(msg_id, &msg, size, 0) == -1) { // 0 = blokujacy tryb wysylki, -1 = blad
        perror("Error sending ticket");
        exit(1);
    }
}

// ustawianie wartosci semafora
void set_sem_value(int sem_id, int sem_num, int val) {

    if (semctl(sem_id, sem_num, SETVAL, val) == -1) { // setval = ustaw wartosc
        perror("Error setting semaphore value");
        exit(1);
    }
}


//odbieranie "biletu" oraz priorytet czyli omijanie kolejki
TicketMessage receive_ticket(int msg_id) {
    TicketMessage msg;
    int size = sizeof(msg) - sizeof(long);

    // sprawdzenie czy jest wiadomosc o priorytecie 2 (VIP)
    if (msgrcv(msg_id, &msg, size, 2, IPC_NOWAIT) != -1) {
        return msg; // zwroc wiadomosc priorytetowa
    }

    // jesli blad inny niz brak wiadomosci to wypisz
    if (errno != ENOMSG && errno != 0) {
        perror("Blad podczas sprawdzania priorytetu");
    }

    // odbierz normalna wiadomosc (typ 1)
    if (msgrcv(msg_id, &msg, size, 0, 0) == -1) {
        perror("Error receiving ticket");
        exit(1);
    }
    return msg;
}