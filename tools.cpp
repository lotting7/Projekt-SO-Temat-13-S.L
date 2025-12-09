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

// semafory

int create_semaphores(int key, int number) {

    int id = semget(key, number, 0666 | IPC_CREAT); // 0666 - prawa zapisu/odczytu dla wszystkich

w
}