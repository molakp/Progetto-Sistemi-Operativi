#include "common.c"

static void dd_signal_handler(int sig) // handler per SIGTERM e SIGUSR1
{
	if (sig == SIGTERM)
	{
		exit(EXIT_SUCCESS);
	}
	if (sig == SIGUSR1)
	{
		printf(" [%d] è stato terminato  \n", getpid());

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
	if (signal(SIGUSR1, dd_signal_handler) == SIG_ERR) // imposto l'handler per SIGUSR1
	{												   // hanlder segnali dal gestore
		printf("Handler gestore non ha funzionato\n");
		exit(0);
	}
	int memID = shmget(KEY, sizeof(struct type), 0); // ID SHARED MEMORY
	int smID = semget(KEY + 1, 1, 0);				 // SEMAFORO SHARED MEMORY
	int mq_AB;										 // ID coda di messaggi tra le A e i B

	// eseguo i vari controlli sugli ID letti dal figlio
	if ((mq_AB = msgget(KEY, 0)) < 0)
	{ // mi collego alla message queue tra A e B
		printf("Errno was set to ==> %s\n", strerror(errno));
		exit(-1);
	}
	// Vari controlli di correttezza degli ID
	if (memID < 0)
	{
		printf("Errore memoria figlio! \n");
	}
	if (smID < 0)
	{
		printf("Errore semaforo shared memory figlio \n");
	}
	// fine controlli

	int mio_numero = atoi(argv[0]);						   // Converto la posizione  del processon shared memory  da string a int
	struct type *b = (struct type *)shmat(memID, NULL, 0); // MI COLLEGO ALLA SHARED MEMORY

	int genoma = 0; // salvo le informazioni che mi servono e accedo alla memoria una sola volta

	reserveSem(smID, 0); // entro in sezione critica per prende le info
	genoma = b[mio_numero].genoma;
	relaseSem(smID, 0); // esco dalla sezione critica

	struct messaggioA_B msg1; // struct per la coda di messaggi tra A e B
	int target = 0;			  // parametro che adatta il comportamento delle A al numero di richieste rifiutate
	int mcdMigliore = -1;	 //mcd migliore tra la a e tutti i processi b

	for (;;)
	{
		if ((msgrcv(mq_AB, &msg1, sizeof(msg1), getpid(), 0)) == -1)
		{ // RICEVO MESSAGGIO
			printf("Ricezione fallita dalla A");
		}
		else
		{
			reserveSem(semID, 0);			  // attendo che il gestore mi faccia partire
			relaseSem(semID, 0);			  // aumento il semaforo per non bloccarmi nelle successive esecuzioni del ciclo for
			int mittente = msg1.PID_mittente; // salvo  il PID del B mittente
			int MCD = gcd(genoma, msg1.genoma_mittente);
			//reserveSem(smID,0); //se lo metto si blocca e non so perche
			for (int i = 0; i < INIT_PEOPLE; i++)
			{ // ciclo per trovare l'mcd migliore
				if (i == mio_numero)
				{
					i++;
				}
				if (i < INIT_PEOPLE)
				{
					if (gcd(b[mio_numero].genoma, b[i].genoma) > mcdMigliore)
						mcdMigliore = gcd(b[mio_numero].genoma, b[i].genoma);
				}
			}
			//relaseSem(smID,0);//se lo metto si blocca e non so perche
			if ((msg1.genoma_mittente % genoma == 0) || (MCD >= (mcdMigliore - target)))
			{ // se genoma multiplo accetto subito
				msg1.mtype = mittente;
				msg1.genoma_mittente = genoma;
				msg1.PID_mittente = (int)getpid();
				msg1.risposta = 1; // rispondo SI
				printf("[ %d ] Ho detto si al messagio di %d ", getpid(), mittente);
				if (msgsnd(mq_AB, &msg1, sizeof(msg1), 0) == -1) // MANDO MESSAGGIO
					printf("Invio fallito\n");
				for (;;)
				{
					// ciclo per aspettare la SIGTERM del gestore
				}
			}
			else
			{
				target++; // mi adatto perche non mi ha accettato
				msg1.mtype = mittente;
				msg1.genoma_mittente = genoma;
				msg1.PID_mittente = (int)getpid();
				msg1.risposta = 0;   // rispondo NO
				reserveSem(smID, 0); // entro in sezione critica per prende le info
				b[mio_numero].cont_rifiuti++;
				relaseSem(smID, 0);								 // esco dalla sezione critica
				if (msgsnd(mq_AB, &msg1, sizeof(msg1), 0) == -1) // MANDO MESSAGGIO
					printf("Invio fallito\n");
			}
		}
	}

	exit(0);
}
