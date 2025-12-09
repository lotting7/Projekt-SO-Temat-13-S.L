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
