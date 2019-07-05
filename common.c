#include "header.h"
 
int reserveSem(int semid, int semNum)
{ //riservo il semaforo diminuendo di 1 il valore del semaforo
    struct sembuf x;
    x.sem_num = semNum;
    x.sem_op = -1;
    x.sem_flg = 0;
    return semop(semid, &x, 1);
}
int relaseSem(int semid, int semNum)
{ //rilascio il semaforo aumentando di 1 il valore del semaforo
    struct sembuf x;
    x.sem_num = semNum;
    x.sem_op = 1;
    x.sem_flg = 0;
    return semop(semid, &x, 1);
}
int relaseSem_perTutti(int semid, int semNum)
{ //rilascio il semaforo aumentando di INIT_PEOPLE il valore del semaforo, serve a far partire con temporaneamente tutti i processi
    struct sembuf x;
    x.sem_num = semNum;
    x.sem_op = INIT_PEOPLE;
    x.sem_flg = 0;
    return semop(semid, &x, 1);
}
int semAvaiable(int semID, int semNum)
{ // inizializzo semaforo a 0
    union semun arg;
    arg.val = 0;
    return semctl(semID, semNum, SETVAL, arg);
}

int gcd(int a, int b)
{ // Calcola il massimo comun divisore
    int mcd;
    while (a != b)
    {
        if (a > b)
            a = a - b;
        else
            b = b - a;
    }
    mcd = a;
    return mcd;
}
