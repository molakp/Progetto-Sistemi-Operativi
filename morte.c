#include "common.c"

int main(int argc, char *argv[])
{
    printf("MORTE! \n");

    int sim_time = atoi(argv[0]);           //prendo dalla execlp() alla posizoine 0 il valore di sim_time
    int birth_death = atoi(argv[1]);        //prendo dalla execlp() alla posizoine 1 il valore di birth_death
    int time_since_last_death = time(NULL); // altrimenti entra nel primo if subito
    int starting_time = time(NULL);         //lo setto al memento iniziale della simulazione
    int actual_time = 0;                    //lo resetto ogni volta che termino qualcuno

    for (;;)
    {
        actual_time = time(NULL);                      //setto il momento in cui ricominciare il ciclo
        if ((actual_time - starting_time) >= sim_time) //
        {
            printf("Simulazione finita, avvio la procedura di chiusura \n");
            kill(getppid(), SIGUSR2); // la simulazione e terminata, chiudo tutto
            for (;;)
            {
                //ciclo in attesa di essere terminato anch'io
            }
        }
        if ((actual_time - time_since_last_death) >= birth_death)
        {
            printf("QUALCUNO DEVE TERMINARE \n");
            kill(getppid(), SIGUSR1);           // SIGUSR1 fa morire un processo a caso
            time_since_last_death = time(NULL); // salvo il momento dell'ultima morte
        }
    }

    return 1;
}