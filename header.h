// HEADER
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h> /* For portability */
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/resource.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#define KEY 7389
#define INIT_PEOPLE 20
#define LUNGHEZZA_NOMI 100
#define MYKEY_SM 100

struct type
{
	pid_t myPID;
	char tipo;				   // tipo del processo, A o B
	char nome[LUNGHEZZA_NOMI]; // Nome del processo
	unsigned long genoma;	  //genoma processo
	int cont_rifiuti;		   // n° di rifiuti dati (per le A) o ricevuti (per i B), mi servirà per abbassare il target successivamente
	int status;				   // booleano per disponibilità a morire o a parlare con altri processi
};

struct messaggioA_B
{
	long mtype;					   // qua va il pid del destinatario
	unsigned long genoma_mittente; // genome processo mittente
	int PID_mittente;			   //PID mittente
	int risposta;				   // Boolean risposta a proposta di accoppiamento
};

struct messaggio_gestore
{
	long mtype;		  // qua va il pid del destinatario o la modalità di accesso alla coda di messaggi nel caso del gestore
	int PID_mittente; //PID mittente
	int PID_partner;  // PID del processo con cui mi sono accoppiato
};

// SEMAFORI

union semun {
	// value for SETVAL
	int val;
	// buffer for IPC_STAT, IPC_SET
	struct semid_ds *buf;
	// array for GETALL, SETALL
	unsigned short *array;
// Linux specific part
#if defined(__linux__)
	// buffer for IPC_INFO
	struct seminfo *__buf;
#endif
};

int reserveSem(int semId, int semNum);
int relaseSem(int semId, int semNum);
int SemAvailable(int semId, int semNum);

//METODI GESTORE

void accoppiamento_birth(struct type *b, int PID1, int PID2); // passati i pid dei genitori ne crea due nuovi (ereditati) e chiude i genitori

int morte();														 // seleziona un processo da terminare casualmente, lo termina e ne crea un altro casualmente
pid_t nasci(int posizione, struct type *process);					 // fa nascere un processo alla posizione 'posizione' nell'array in memoria condivisa
void aggiorna(int ID_memoria_condivisa, struct type, int posizione); //aggiorna la tabella dei processi attivi
