// funkcje z tools.h

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include "tools.h"

// SEMAFORY

int create_semaphores(int key, int number) { // tworzenie semaforow

    int id = semget(key, number, 0666 | IPC_CREAT); // 0666 - prawa zapisu/odczytu dla wszystkich

    if (id == -1) {
        perror("Error creating semaphores");
        exit(1);
    }
    return id;

}

void remove_semaphores(int sem_id) { // usuwanie semaforow
    if (semctl(sem_id, 0, IPC_RMID) == -1) { // IPC_RMID = remove id
        perror("Error removing semaphores");
    }
}

void lock_sem(int sem_id, int sem_num) { // zablokowanie semafora (oczekiwanie badz ruszenie)
    struct sembuf op;
    op.sem_num = sem_num; // numer semafora
    op.sem_op = -1;       // -1 oznacza zajecie miejsce, jezeli = 0 zatrzymuje proces
    op.sem_flg = 0;       // standard

    if (semop(sem_id, &op, 1) == -1) {
        perror("Error locking semaphore");
        exit(1);
    }
}

void unlock_sem(int sem_id, int sem_num) {
    // odblokowanie semafora
    struct sembuf op;
    op.sem_num = sem_num;
    op.sem_op = 1;        // +1 oznacza "zwolnij" czyli zwieksza licznik
    op.sem_flg = 0;

    if (semop(sem_id, &op, 1) == -1) {
        perror("Error unlocking semaphore");
        exit(1);
    }
}

// shared memory

//tworzenie pamieci
int create_shared_memory(int key, int size) {
    int id = shmget(key, size, 0666 | IPC_CREAT);
    if (id == -1) {
        perror("Error creating shared memory");
        exit(1);
    }
    return id;
}

void remove_shared_memory(int shm_id) {
    shmctl(shm_id, IPC_RMID, NULL);
}

// podlaczanie procesu do pamieci
int* attach_memory(int shm_id) {

    void* addr = shmat(shm_id, NULL, 0); // zwraca void*, rzutujemy na int*

    if (addr == (void*)-1) {
        perror("Error attaching memory");
        exit(1);
    }
    return (int*)addr;
}

void detach_memory(int* addr) {
    shmdt(addr);
}


// kolejka wiadomosci

// tworzenie kolejki
int create_msg_queue(int key) {
    int id = msgget(key, 0666 | IPC_CREAT);
    if (id == -1) {
        perror("Error creating queue");
        exit(1);
    }
    return id;
}

void remove_msg_queue(int msg_id) {
    msgctl(msg_id, IPC_RMID, NULL);
}

// wysylanie "biletu"
void send_ticket(int msg_id, TicketMessage msg) {

    int size = sizeof(msg) - sizeof(long); // rozmiar danych


    if (msgsnd(msg_id, &msg, size, 0) == -1) { // 0 = blokada, kolejka pelna
        perror("Error sending ticket");
        exit(1);
    }
}

//odbieranie "biletu"
TicketMessage receive_ticket(int msg_id) {
    TicketMessage msg;
    int size = sizeof(msg) - sizeof(long);

    // MSG_TICKET to typ wiadomości, na którą czekamy (z pliku common.h)
    // Funkcja czeka (usypia proces), aż w skrzynce pojawi się wiadomość tego typu
    if (msgrcv(msg_id, &msg, size, MSG_TICKET, 0) == -1) {
        perror("Error receiving ticket");
        exit(1);
    }
    return msg;
}