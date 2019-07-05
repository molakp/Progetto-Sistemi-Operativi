#include "common.c"

static void dd_signal_handler(int sig) // metodo per la gestione dei segnali
{
	if (sig == SIGTERM)
	{
		exit(EXIT_SUCCESS);
	}
	if (sig == SIGUSR1)
	{
		printf("%d è stato ammazzato  \n", getpid());

		exit(EXIT_SUCCESS);
	}
}

int main(int argc, char *argv[])
{

	int semID = semget(KEY, 1, 0); // SEMAFORO PARTENZA PROCESSI
	if (semID < 0)
	{
		printf("Errore semaforo partenza processi nel figlio! \n");
	}
	if (signal(SIGUSR1, dd_signal_handler) == SIG_ERR) // hanlder segnali dal gestore
	{
		printf("Handler gestore non ha funzionato\n");
		exit(0);
	}
	int memID = shmget(KEY, sizeof(struct type), 0); // ID SHARED MEMORY
	int smID = semget(KEY + 1, 1, 0);				 // SEMAFORO SHARED MEMORY
	int mq_AB;										 // ID coda di messaggi tra le A e i B
	int mq_gestore_B;								 // ID coda di messaggi tra le gestore e i B

	// eseguo i vari controlli sugli ID letti dal figlio

	if ((mq_AB = msgget(KEY, 0)) < 0)
	{ // mi collego alla message queue tra A e B
		printf(" [%d] B Errno was set to ==> %s\n", getpid(), strerror(errno));
		exit(-1);
	}
	if ((mq_gestore_B = msgget(KEY + 1, 0)) < 0)
	{ // mi collego alla message queue tra B e gestore
		printf("[%d] B Errno was set to ==> %s\n", getpid(), strerror(errno));
		exit(-1);
	}
	if (memID < 0)
	{
		printf("Errore memoria figlio! \n");
	}
	if (smID < 0)
	{
		printf("Errore semaforo shared memory figlio \n");
	}
	int mio_numero = atoi(argv[0]);						   // Converto il numero del processo da string a int
	struct type *b = (struct type *)shmat(memID, NULL, 0); // MI COLLEGO ALLA SHARED MEMORY

	//DA QUI IL CODICE DEL FIGLIO
	int trovata = 0;	  // flag per segnalare che ho trovato una A
	reserveSem(semID, 0); // attendo che il gestore mi faccia partire
	for (;;)
	{
		pid_t migliore;				// PID della A migliore, ovvero quella che sarà contattata
		int genoma_migliore = 0;	//PID A migliore
		int mio_genoma = 0;			//genoma A migliore
		int num_occupati = 0;		//dice quanti processi hanno lo status a 1 quindi non possono morire
		int posizione_migliore = 0; // posizione della A che contatterò

		//*********SEZIONE CRITICA***************

		reserveSem(smID, 0);	  // prendo il semaforo per l'accesso alla shared memeory
		b[mio_numero].status = 1; // metto il mio status a 1 per non essere terminato mentre sono in sezione critica
		// con questo for trovo la A migliore tra quelle libere
		for (int i = 0; i < INIT_PEOPLE; i++)
		{
			if (b[i].status == 1) // mi conto
				num_occupati++;
			if (b[i].tipo == 'A' && b[i].status == 0 && b[i].genoma >= genoma_migliore)
			{
				migliore = b[i].myPID;		   // salvo il suo PID
				genoma_migliore = b[i].genoma; // e il suo genoma per poterla confrontare con le altre A
				posizione_migliore = i;
				trovata = 1; //indica di aver trovato una A con  le caratteristiche ricercate
			}
		}
		if (num_occupati < (INIT_PEOPLE - 1)) // garantisco che ci siano almeno 2 processi sempre disponibii a morire
		{
			if (trovata == 1)
			{									  // Solo se ho trovato una A libera proseguo
				b[posizione_migliore].status = 1; //metto lo status della A migliore a 1, entrambi non possiamo piu morire
				mio_genoma = b[mio_numero].genoma;
				relaseSem(smID, 0); //rilascio l'accesso alla memoria

				//**************FINE SEZINE CRITICA**********************

				// ho trovato la A migliore e l'ho bloccata, quindi le mando un messaggio
				struct messaggioA_B msg1;
				msg1.mtype = (long)migliore;
				msg1.genoma_mittente = mio_genoma;
				msg1.PID_mittente = (int)getpid();
				msg1.risposta = 0; // per adesso la risposta è no
				if (msgsnd(mq_AB, &msg1, sizeof(msg1), 0) == -1)
				{
					// MANDO MESSAGGIO
					printf("[%d] Invio fallito alla A [%d]. Errno was set to ==> %s\n", getpid(), b[posizione_migliore].myPID, strerror(errno));
				}

				if ((msgrcv(mq_AB, &msg1, sizeof(msg1), getpid(), 0)) == -1) // RICEVO MESSAGGIO{
					printf(" \n******************************** 	Ricezione fallita dalla A *************************** \n");
				if (msg1.risposta == 1) //ho ricevuto una risposta positiva da parte della A ricercata, avviso il gestore
				{
					struct messaggio_gestore msg_gestore;
					msg_gestore.mtype = (int)getppid();
					msg_gestore.PID_mittente = (int)getpid();
					msg_gestore.PID_partner = (int)migliore;
					if (msgsnd(mq_gestore_B, &msg_gestore, sizeof(msg_gestore), 0) == -1) // MANDO MESSAGGIO
						printf("Invio fallito al gestore\n");

					// Ho trovato una A e avvisato il gestore, la mia vita è finita
					printf("[%d] Mi sono accoppiato con %d e ora muoio \n", getpid(), migliore);
					for (;;)
					{
						//ciclo infinito aspettando la SIGTERM del gestore
					}
				}
				else
				{
					if (msg1.risposta == 0) //la A ha rifiutato il mio contatto, resetto tutti gli status a 0
					{
						reserveSem(smID, 0); // prendo il semaforo per l'accesso alla shared memeory
						b[posizione_migliore].status = 0;
						b[mio_numero].status = 0;
						b[mio_numero].cont_rifiuti++;
						relaseSem(smID, 0); //rilascio l'accesso alla memoria
					}
				}
			}
			else
			{

				b[mio_numero].status = 0;
				relaseSem(smID, 0); //rilascio l'accesso alla memoria
			}
		}
		else
		{
			b[mio_numero].status = 0;
			relaseSem(smID, 0); //rilascio l'accesso alla memoria
		}
	}
}
