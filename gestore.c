#include "common.c"
// Developed for Unito 
int signal1 = 0, signal2 = 0; // per sapere se ho ricevuto segnali mentre in sezione critica

// Variabili per statistiche simulazione
int tipoA = 0, tipoB = 0;         // n° processi di tipo A, n° di tipo B
char tipo_maxGen, tipo_maxNome;   // tipo del processo con genoma più grande e tipo del processo con genoma più lungo
int gen_maxNome = 0;              // genoma del processo con nome più lungo
int max_gen = 0;                  //genoma più grande
char nome_maxGen[LUNGHEZZA_NOMI]; // nome del processo con genoma più grande
char max_nome[LUNGHEZZA_NOMI];    // nome più lungo
//fine variabili per statistiche simulazione

int flag = 2, PID_Morte = 0;                  // variabili ausiliarie
int sim_time = 0, birth_death = 0, genes = 0; // durata simulazione, intervallo uccisione processi, valore genes per calcolo genoma

// METODI PROPRI DEL GESTORE
static void dd_signal_handler_accoppiamento(int sig) // handler per segnali ricevuti durante funzioni
{
    if (sig == SIGUSR1)
    {
        signal1 = 1;
    }
    if (sig == SIGUSR2)
    {
        signal2 = 1;
    }
}
void print_memory(struct type *b) // Funzione per stampare la memoria condivisa
{
    int smID;                                                  // SEMAFORO MEMORIA CONDIVISA
    int memID;                                                 // ID memoria condivisa
    memID = shmget(KEY, sizeof(struct type) * INIT_PEOPLE, 0); // Ottengo ID SHARED MEMORY
    if (memID < 0)
    {
        printf("Errore creazione memoria condivisa \n");
    }
    if ((smID = semget(KEY + 1, 1, 0)) < 0)
    { // SEMAFORO MEMORIA CONDIVISA
        printf("PRINT Errno was set to ==> %s\n", strerror(errno));
        exit(-1);
    }
    if ((signal(SIGUSR1, dd_signal_handler_accoppiamento)) == SIG_ERR)
    { // se arriva un segnale SIGUSR1 dalla morte lo ignoro
        printf("Handler print non ha funzionato\n");
        exit(0);
    }
    if ((signal(SIGUSR2, dd_signal_handler_accoppiamento)) == SIG_ERR)
    { // se arriva un segnale SIGUSR1 dalla morte lo ignoro
        printf("Handler print non ha funzionato\n");
        exit(0);
    }
    reserveSem(smID, 0); // entro in sezione critica e leggo la shared memory
    printf("*** MEMORIA *** \n");

    for (int h = 0; h < INIT_PEOPLE; h++)
    {
        printf(" %d  PID: %d   Status: %d Genoma: %lu Tipo: %c Rifiuti: %d Nome: %s \n", h, b[h].myPID, b[h].status, b[h].genoma, b[h].tipo, b[h].cont_rifiuti, b[h].nome);
    }
    relaseSem(smID, 0); // esco dalla sezione critica e libero il semaforo
    if (signal2 == 1)
    {
        printf("Rimando signal2 ricevuto in print memory \n");
        signal2 = 0;
        kill(getpid(), SIGUSR2);
    }

    if (signal1 == 1)
    {
        printf("Rimando signal1 ricevuto in print memory \n");
        signal1 = 0;
        kill(getpid(), SIGUSR1);
    }
}
pid_t nasci(int posizione, struct type *process) // Funzione per far nascere un processo
{
    pid_t procesPid;
    int tempA = 0, tempB = 0;     // variabili temporanee
    int tipo = 65 + (rand() % 2); //variabile per memorizzare il tipo del processo
    if (tipo == 'A')
        tipoA++; // incremento numero di processi A
    else
        tipoB++; // incremento numero di processi B
    if (flag == 2)
    {                                     // controllo che facciamo solo alla creazione iniziale dei processi
        if (posizione == INIT_PEOPLE - 1) // se sono nell'ultima posizione in memoria
        {
            if (tipoA == 0)                     // e non ho trovato alcuna A allora
                tipo = 'A';                     //il mio tipo è A cosi mi assicuro che c'è almeno una A
            else if (tipoA == 0 && tipo == 'B') //se non ho A ed il mio tipo è B cambio tipo ed aggiorno i contatori dei processi
            {
                tipoB--;
                tipoA++;
                tipo = 'A';
            }
            if (tipoB == 0)                     // Se non ho contato nessuna B, allora
                tipo = 'B';                     //il mio tipo è un B cosi mi assicuro che c'è almeno un B
            else if (tipoB == 0 && tipo == 'A') //se non ho B ed il mio tipo è A cambio tipo ed aggiorno i contatori dei processi
            {
                tipoA--;
                tipoB++;
                tipo = 'B';
            }
            flag--; // Decremento flag per non eseguire più questo codice
        }
    }
    else
    {
        for (int i = 0; i < INIT_PEOPLE; i++)
        { // conto il numero di A e B in memoria
            if (i == posizione)
            { // se sono alla posizione del processo che dovrà nascere, ignoro il tipo attuale perché è vecchio e sarà sovrascritto
                i++;
            }
            if (i < INIT_PEOPLE) // per evitare errori di lettura
            {
                if (process[i].tipo == 'A') // conto i processi di tipo A e di tipo B
                    tempA++;
                else
                    tempB++;
            }
        }

        if (tempA == 0 && tipo == 'A')      // Inutile ma lo lasciamo per chiarezza
            tipo = 'A';                     //il mio tipo è A cosi mi assicuro che c'è almeno una A
        else if (tempA == 0 && tipo == 'B') // aggiorno i contatori del numero dei processi
        {
            tipoB--;
            tipoA++;
            tipo = 'A';
        }
        if (tempB == 0 && tipo == 'B')      // Inutile ma lo lasciamo per chiarezza
            tipo = 'B';                     //il mio tipo è un B cosi mi assicuro che c'è almeno un B
        else if (tempB == 0 && tipo == 'A') // aggiorno i contatori del numero dei processi
        {
            tipoA--;
            tipoB++;
            tipo = 'B';
        }
    }
    procesPid = fork();

    switch (procesPid)
    {
    case -1:
        printf("Errore creazione figlio! \n");
        exit(0);
        break;
    case 0:
        if (tipo == 65)
        {
            char *nuovo = "./a";                         //percorso codice di A
            char *num_pro = (char *)malloc(sizeof(int)); // serve a passare come argomento il n° identificativo del processo
            sprintf(num_pro, "%d", posizione);           // scrivo nell stringa il numero del processo figlio e lo passo come parametro
            if (execlp(nuovo, num_pro, (char *)NULL) < 0)
            {
                printf("Errore execlp\n");
            }
        }
        else
        {
            char *nuovo = "./b";                         //percorso processo B
            char *num_pro = (char *)malloc(sizeof(int)); // serve a passare come argomento il n° identificativo del
            sprintf(num_pro, "%d", posizione);           // scrivo nell stringa il numero del processo figlio e lo passo come parametro
            if (execlp(nuovo, num_pro, (char *)NULL) < 0)
            {
                printf("Errore execlp\n");
            }
        }

        break;
    default:
    {

        for (int i = 0; i < LUNGHEZZA_NOMI; i++) // Sovrascrivo tutto il nome per eliminare il nome dell'eventuale processo precedente
            process[posizione].nome[i] = '\0';

        int lettera = 65 + rand() % 26;             //genero lettera del nome
        process[posizione].nome[0] = (char)lettera; // la scrivo nel nome
        process[posizione].tipo = (char)tipo;       // scrivo il tipo
        lettera = 100 + rand() % 901;               // il genoma va da 100 a mille
        process[posizione].genoma = lettera;
        process[posizione].cont_rifiuti = 0;
        process[posizione].status = 0;
        process[posizione].myPID = procesPid;

        if (flag > 0)
        { //flag mi serve per sapere se mia ha chiamato acc. birth cosi non modifico niente dal momento che reinizializzo  le variabili

            if (process[posizione].genoma > max_gen) // salvo genoma più grande
            {
                max_gen = process[posizione].genoma;
                tipo_maxGen = process[posizione].tipo;
                strcpy(nome_maxGen, process[posizione].nome);
            }
            if (strlen(process[posizione].nome) > strlen(max_nome)) // salvo nome più lungo
            {
                strcpy(max_nome, process[posizione].nome);
                gen_maxNome = process[posizione].genoma;
                tipo_maxNome = process[posizione].tipo;
            }
        }
        return procesPid;
    }
    }

    return procesPid;
}

int morte()
{ // AVVIA LA MORTE
    pid_t PID = fork();

    switch (PID)
    {
    case -1:
        printf("Errore creazione morte! \n");
        exit(0);
        break;
    case 0:
    {

        char *percorso = "./morte";                  //percorso codice morte
        char *sim_tim = (char *)malloc(sizeof(int)); // serve a passare come argomento sim_time
        sprintf(sim_tim, "%d", sim_time);
        char *birth = (char *)malloc(sizeof(int)); // serve a passare come argomento birth_death
        sprintf(birth, "%d", birth_death);
        if (execlp(percorso, sim_tim, birth, (char *)NULL) < 0)
        {
            printf("Creazione morte fallita \n");
        }
        break;
    }
    default:
        printf("ho creato la morte \n");
        break;
    }
    return PID;
}

static void dd_signal_handler(int sig)
{
    // recupero i vari ID
    int semID;        // SEMAFORO PARTENZA PROCESSI
    int smID;         // SEMAFORO MEMORIA CONDIVISA
    int mq_AB;        // ID coda di messaggi tra le A e i B
    int mq_gestore_B; // ID coda di messaggi tra le gestore e i B
    mq_gestore_B = msgget(KEY + 1, 0);
    //***************** OTTENGO ID OGGETTI SYSTEM V ************************
    if ((mq_AB = msgget(KEY, 0)) < 0)
    { // CREO MESSAGE QUEUE tra A e B
        printf("HANDLER1 Errno was set to ==> %s\n", strerror(errno));
        exit(-1);
    }
    if ((mq_AB = msgget(KEY, 0)) < 0)
    { // CREO MESSAGE QUEUE tra gestore e i B
        printf("HANDLER1 Errno was set to ==> %s\n", strerror(errno));
        exit(-1);
    }

    if ((semID = semget(KEY, 1, 0)) < 0)
    { // SEMAFORO PARTENZA PROCESSI
        printf("HANDLER1 Errno was set to ==> %s\n", strerror(errno));
        exit(-1);
    }
    if ((smID = semget(KEY + 1, 1, 0)) < 0)
    { // SEMAFORO MEMORIA CONDIVISA
        printf("HANDLER1 Errno was set to ==> %s\n", strerror(errno));
        exit(-1);
    }
    int memID = shmget(KEY, sizeof(struct type) * INIT_PEOPLE, 0); // ID SHARED MEMORY
    struct type *b = (struct type *)shmat(memID, NULL, 0);         // MI COLLEGO ALLA SHARED MEMORY
    if (sig == SIGUSR1)
    { // La morte ha detto di terminare un processo a caso
        printf("SIGUSR1!\n");
        int chi_muore;
        int trovata = 0;
        print_memory(b);

        while (!trovata)
        {
            chi_muore = rand() % INIT_PEOPLE;

            reserveSem(smID, 0); // entro in sezione critica per  leggere status processi
            if (b[chi_muore].status == 0)
            {
                printf("Deve essere terminato  %d  con pid: %d \n", chi_muore, b[chi_muore].myPID);
                if (kill(b[chi_muore].myPID, SIGTERM) < 0)
                {
                    printf("Il figlio non è stato terminato! Errno was set to ==> %s\n", strerror(errno));
                } //  IL PROCESSO DOVRÀ MORIRE

                waitpid(b[chi_muore].myPID, NULL, 0);
                printf("Il processo è stato terminato \n");
                relaseSem(semID, 0); // aumento il semaforo della partenza sincronizzata iniziale, altrimenti si bloccano
                nasci(chi_muore, b); // un nuovo processo nasce
                trovata = 1;
            }
            relaseSem(smID, 0);
        }
    }
    if (sig == SIGUSR2)
    {
        printf("SIGUSR2!\n");
        kill(PID_Morte, SIGTERM);
        waitpid(PID_Morte, NULL, 0);
        //  printf("Valore ultimo pid del semaforo %d \n ", semctl(smID, 0, GETPID));
        reserveSem(smID, 0); // entro in sezione critica bloccando la memoria
        for (int i = 0; i < INIT_PEOPLE; i++)
        {
            printf("termino %d \n", b[i].myPID);
            kill(b[i].myPID, SIGTERM);    // TERMINO TUTTI I PROCESSI
            waitpid(b[i].myPID, NULL, 0); // aspetto che il figlio muoia per non farlo rimanere in stato zombie
            printf(" %d e stato terminato \n", b[i].myPID);
        }
        relaseSem(smID, 0);
        printf("\n 		***	stato finale memoria *** 	\n");
        print_memory(b);
        printf("\n    *** Resoconto statistiche simulazione ***    \n");
        printf("Durante tutta la simulazione abbiamo creato %d individui di cui\n", tipoA + tipoB);
        printf("%d individui di tipo A, e ", tipoA);
        printf("%d individui di tipo B\n \n", tipoB);
        printf("L'individuo con nome più lungo è stato: \n");
        printf("Nome %s\nTipo %c\nGenoma %d\n \n", max_nome, tipo_maxNome, gen_maxNome );
        printf("L'individuo con genoma più grande è stato: \n");
        printf("Nome %s\nTipo %c\nGenoma %d\n",nome_maxGen, tipo_maxGen,max_gen);
        //************************* elimino tutti gli oggetti System V ******************

        if ((semctl(semID, 0, IPC_RMID)) < 0)
        { // RIMUOVO SEMAFORO PARTENZA
            printf("Errore chiusura semaforo\n");
            exit(-1);
        }
        if ((semctl(smID, 0, IPC_RMID)) < 0)
        { // RIMUOVO SEMAFORO SHARED MEMORY
            printf("Errore chiusura semaforo\n");
            exit(-1);
        }
        if (shmctl(memID, IPC_RMID, 0) < 0)
        { // RIMUOVO SHARED MEMORY
            printf("rimozione memoria non effettuata");
        }
        if (msgctl(mq_AB, IPC_RMID, NULL) == -1)
        { // RIMUOVO CODA MESSAGGI TRA A e B
            printf("errore rimozione queue \n");
        }
        if (msgctl(mq_gestore_B, IPC_RMID, NULL) == -1)
        { // RIMUOVO CODA MESSAGGI TRA A e B
            printf("errore rimozione queue \n");
        }
        exit(0);
    }
}

void accoppiamento_birth(struct type *b, int PID_1, int PID_2)
{

    printf("Accoppiamento!\n");
    if ((signal(SIGUSR1, dd_signal_handler_accoppiamento)) == SIG_ERR)
    { // se arriva un segnale SIGUSR1 dalla morte lo ignoro
        printf("Handler accoppiamento non ha funzionato\n");
        exit(0);
    }
    if ((signal(SIGUSR2, dd_signal_handler_accoppiamento)) == SIG_ERR)
    { // se arriva un segnale SIGUSR2 dalla morte lo ignoro
        printf("Handler accoppiamento non ha funzionato\n");
        exit(0);
    }
    int semID;                  // SEMAFORO PARTENZA PROCESSI
    int smID;                   // SEMAFORO MEMORIA CONDIVISA
    int pos_primo, pos_secondo; // Posizioni struct processi nella shared memory

    if ((semID = semget(KEY, 1, 0)) < 0)
    { // SEMAFORO PARTENZA PROCESSI (serve dopo quando nascono)
        printf("ACC_BIRTH Errno was set to ==> %s\n", strerror(errno));
        exit(-1);
    }
    if ((smID = semget(KEY + 1, 1, IPC_CREAT | 0666)) < 0)
    { // SEMAFORO MEMORIA CONDIVISA
        printf("SM ACC_BIRTH Errno was set to ==> %s\n", strerror(errno));
        exit(-1);
    }
    reserveSem(smID, 0); // devo entrare in sezione critica per aggiornare la memoria
    for (int i = 0; i < INIT_PEOPLE; i++)
    { //ricavo le posizioni dei processi che si stanno accoppiando
        if (b[i].myPID == PID_1)
            pos_primo = i;
        if (b[i].myPID == PID_2)
            pos_secondo = i;
    }

    char nome1[LUNGHEZZA_NOMI]; //  salvo i nomi dei genitori che saranno sovrascritti da nasci()
    char nome2[LUNGHEZZA_NOMI];
    strcpy(nome1, b[pos_primo].nome);
    strcpy(nome2, b[pos_secondo].nome);
    int MCD_genitori = gcd(b[pos_primo].genoma, b[pos_secondo].genoma); //mi ricavo il MCD tra i 2 pid dei genitori
    printf("-+-+-+-+-+   Ecco memoria prima dell'accopiamento   -+-+-+-+-+\n");
    print_memory(b);
    // Nascita primo figlio
    pid_t figlio1;
    flag = 0;
    figlio1 = nasci(pos_primo, b);
    if (figlio1 < 0)
    {
        printf("Errore creazione figlio \n");
        exit(-1);
    }
    flag = 1;

    if ((strlen(nome1) + strlen(nome2) + 2) < LUNGHEZZA_NOMI)
    { // se a lughezza dei nomi non supera il massimo
        strcpy(b[pos_primo].nome, nome1);
        strcat(b[pos_primo].nome, nome2);
        char k[2];
        k[0] = 'A' + (rand() % 26); // lettera causale
        k[1] = '\0';
        strcat(b[pos_primo].nome, k);
    }
    b[pos_primo].genoma = MCD_genitori + (rand() % (genes + 1)); //genoma dei processi, da x a x+genes
                                                                 // Confronto genoma e nome per salvare le statistiche della simulazione
    if (b[pos_primo].genoma > max_gen)
    {
        max_gen = b[pos_primo].genoma;
        tipo_maxGen = b[pos_primo].tipo;
        strcpy(nome_maxGen, b[pos_primo].nome);
    }
    if (strlen(b[pos_primo].nome) > strlen(max_nome))
    {
        strcpy(max_nome, b[pos_primo].nome);
        gen_maxNome = b[pos_primo].genoma;
        tipo_maxNome = b[pos_primo].tipo;
    }
    // Nascita secondo figlio
    pid_t figlio2;
    flag = 0;
    figlio2 = nasci(pos_secondo, b);
    if (figlio2 < 0)
    {
        printf("Errore creazione figlio \n");
        exit(-1);
    }
    flag = 1;

    if ((strlen(nome1) + strlen(nome2) + 2) < LUNGHEZZA_NOMI)
    { // se a lughezza dei nomi non supera il massimo
        strcpy(b[pos_secondo].nome, nome1);
        strcat(b[pos_secondo].nome, nome2);
        char k[2];
        k[0] = 'A' + (rand() % 26); // lettera causale
        k[1] = '\0';
        strcat(b[pos_secondo].nome, k);
    }

    b[pos_secondo].genoma = MCD_genitori + (rand() % (genes + 1)); //genoma dei processi, da x a x+genes
                                                                   // Confronto genoma e nome per salvare le statistiche della simulazione
    if (b[pos_secondo].genoma > max_gen)
    {
        max_gen = b[pos_secondo].genoma;
        tipo_maxGen = b[pos_secondo].tipo;
        strcpy(nome_maxGen, b[pos_secondo].nome);
    }
    if (strlen(b[pos_secondo].nome) > strlen(max_nome))
    {
        strcpy(max_nome, b[pos_secondo].nome);
        gen_maxNome = b[pos_secondo].genoma;
        tipo_maxNome = b[pos_secondo].tipo;
    }
    relaseSem(smID, 0);
    //stampo la memoria appena aggiornata
    print_memory(b);
}

int main()
{

    system("clear");
    printf("Inserire sim_time: \n");
    scanf("%d", &sim_time);
    printf("Inserire birth_death: \n");
    scanf("%d", &birth_death);
    printf("Inserire genes: \n");
    scanf("%d", &genes);
    printf("AVVIO! \n");
    pid_t procesPid;
    int semID;        // SEMAFORO PARTENZA PROCESSI
    int smID;         // SEMAFORO MEMORIA CONDIVISA
    int memID;        // ID memoria condivisa
    int mq_AB;        // ID coda di messaggi tra le A e i B
    int mq_gestore_B; // ID coda di messaggi tra le gestore e i B

    // OTTENGO ID SEMAFORI , MEMORIA CONDIVISA E CODE DI MESSAGGI
    if ((mq_AB = msgget(KEY, IPC_CREAT | 0666)) < 0)
    { // CREO MESSAGE QUEUE tra A e B
        printf("Errno was set to ==> %s\n", strerror(errno));
        exit(-1);
    }
    if ((mq_gestore_B = msgget(KEY + 1, IPC_CREAT | 0666)) < 0)
    { // CREO MESSAGE QUEUE tra gestore e i B
        printf("Errno was set to ==> %s\n", strerror(errno));
        exit(-1);
    }

    memID = shmget(KEY, sizeof(struct type) * INIT_PEOPLE, IPC_CREAT | 0666); // Ottengo ID SHARED MEMORY

    if ((semID = semget(KEY, 1, IPC_CREAT | 0666)) < 0)
    { // SEMAFORO PARTENZA PROCESSI
        printf("Errno was set to ==> %s\n", strerror(errno));
        exit(-1);
    }
    if ((smID = semget(KEY + 1, 1, IPC_CREAT | 0666)) < 0)
    { // SEMAFORO MEMORIA CONDIVISA
        printf("Errno was set to ==> %s\n", strerror(errno));
        exit(-1);
    }
    //___________Creo memoria condivisa con array di INIT_PEOPLE struct type
    struct type *b;

    if (memID < 0)
    {
        printf("Errore creazione memoria condivisa \n");
    }
    printf("memID nel padre è %d \n", memID);

    b = (struct type *)shmat(memID, NULL, 0);

    if (signal(SIGUSR1, dd_signal_handler) == SIG_ERR)
    {
        printf("Handler gestore non ha funzionato\n");
        exit(0);
    }
    if (signal(SIGUSR2, dd_signal_handler) == SIG_ERR)
    {
        printf("Handler gestore non ha funzionato\n");
        exit(0);
    }

    semAvaiable(semID, 0); // inizializzo il semaforo partenza processi
    semAvaiable(smID, 0);  // inizializzo il semaforo shared  memory

    relaseSem(smID, 0);  // metto il semaforo memoria condivisa a 1
    reserveSem(smID, 0); // metto il semaforo memoria condivisa a 0

    for (int i = 0; i < INIT_PEOPLE; i++)
    { //CREO INIT_PEOPLE processi
        int num_processo = i;
        //prima di scrivere controllo il semaforo ed entro in sezione critica
        procesPid = nasci(num_processo, b);
    }

    relaseSem(smID, 0); // ho finito di creare i processi e di scrivere tutto, quindi rilascio il semaforo
    print_memory(b);
    printf("[padre] il mio pid è %d, sto per chiamare la morte\n", getpid());

    relaseSem_perTutti(semID, 0); // QUI partono tutti i processi figli
    PID_Morte = morte();          // AVVIO LA MORTE
    printf("Sono partiti! \n");

    for (;;) //  In questo ciclo  il gestore esegue i suoi compiti durante la simulazione
    {

        if (1)
        {
        }
        struct messaggio_gestore msg1; // per sicurezza ad ogni ripetizione inizializzo il messaggio

        if (msgrcv(mq_gestore_B, &msg1, sizeof(msg1), getpid(), 0) == -1) // RICEVO MESSAGGIO
        {
            // se fallisce è solo una system call interrotta da una signal
        }
        else
        {

            int PID1 = msg1.PID_mittente; // Il PID della B
            int PID2 = msg1.PID_partner;  // Il PID della A

            //Termino i processi
            kill(msg1.PID_mittente, SIGTERM);
            if (waitpid(msg1.PID_mittente, NULL, 0) == -1) // aspetto la terminazione del figlio per non lasciare zombie
                printf("Errore wait gestore: %s\n", strerror(errno));
            kill(msg1.PID_partner, SIGTERM); // aspetto la terminazione del figlio per non lasciare zombie
            if (waitpid(msg1.PID_partner, NULL, 0) == -1)
                printf("Errore wait gestore: %s\n", strerror(errno));

            printf("B [%d] ed A [%d] si accoppiano \n", PID1, PID2);
            accoppiamento_birth(b, PID2, PID1);
        }

        if (signal(SIGUSR1, dd_signal_handler) == SIG_ERR) // imposto di nuovo l'handler
        {
            printf("Handler gestore non ha funzionato\n");
            exit(0);
        }
        if (signal(SIGUSR2, dd_signal_handler) == SIG_ERR)
        {
            printf("Handler gestore non ha funzionato\n");
            exit(0);
        }

        if (signal2 == 1) // se signal2== 1 vuol dire che ho ricevuto  SIGUSR2 mentre ero in una funzione
        {
            printf("Rimando SIGUSR2 che avevo ricevuto durante altre operazioni \n");
            signal2 = 0;
            kill(getpid(), SIGUSR2); // mi mando  SIGUSR2
        }
        if (signal2 != 1)
        {
            if (signal1 == 1) // se signal1== 1 vuol dire che ho ricevuto  SIGUSR1 mentre ero in una funzione
            {
                printf("Rimando SIGUSR1 che avevo ricevuto durante altre operazioni\n");
                signal1 = 0;
                kill(getpid(), SIGUSR1); // mi mando  SIGUSR1
            }
        }
    }

    exit(-1);
}
