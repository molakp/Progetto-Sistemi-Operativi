# Operating Systems Project #
This repository contains the source code and documentation for an Operating Systems project developed as a part of a university course.The project simulates agents interacting and mating, creating offspring based on their DNA.

 The project was authored by Silvestro Stefano Frisullo.

## Files
The project consists of 7 files, including 5 .c files, a makefile, and a header file. The files are:

gestore.c
a.c
b.c
morte.c
common.c (which contains utility methods used by the other files)

## Usage ##
To execute the simulation, run the command make in the terminal to automatically compile all source files and launch the gestore process. To set the number of processes to be created and remain constant throughout the simulation, modify the value of INIT_PEOPLE in the header file. This value must be greater than 2 to ensure proper functionality of the project. The variables birth_death (which determines the interval at which to randomly terminate a process), sim_time (which determines the duration of the simulation), and genes (used in genome calculation) are prompted for input upon execution.

## Functionality ##
The simulation begins with the gestore process, which creates semaphores to coordinate the simulation, a shared memory (whose access is regulated by a semaphore) visible to all processes, an array of structs of size INIT_PEOPLE, and two message queues, one used for communication between the a and b processes and another used by the gestore and b processes. The newly created process, through an execlp() executed by the nasci function, executes its own code depending on its type and passes its position in shared memory as a parameter. The gestore then creates the morte process, which executes a fork() and an execlp() with birth_death and sim_time parameters. The morte uses the time() function to start a timer that keeps track of the time elapsed since the start of the simulation. Every birth_death seconds, the morte sends a SIGUSR1 signal to the gestore, which then proceeds to terminate a randomly selected process with a status of 0 through the dd_signal_handler handler. The morte is also responsible for ending the simulation after sim_time seconds by sending a SIGUSR2 signal to the gestore, which will terminate all active processes, including the morte itself.

After the creation of morte, the gestore synchronously starts all initially created processes using a semaphore. The main task of the new a and b processes is to choose each other and mate based on the criteria specified in the project. To begin the search for the a process in shared memory, the b process sets its status to 1 to signal to the gestore that it cannot be terminated. It then selects the a process with the highest genome among those with a status of 0. Once the best a process is chosen, b "locks" it by setting its status to 1, to prevent it from being terminated by the gestore or contacted by other b processes. b then sends a message to a with its genome and PID and waits for a response. In case of a negative response, b sets the statuses to 0 and searches for a new a, while a lowers its target to facilitate future mating. In case of a positive response, b sends a message to the gestore with its PID and the PID of a and waits to be terminated.

The gestore, after receiving the PIDs of the newly mated processes, calls the accoppiamento_birth function, passing the two PIDs as parameters. This function then immerses two new processes into the population at the same position in shared memory. These new processes inherit characteristics from their parent processes that just terminated, including a name that is the concatenation of the parents' names with an additional random letter between A and Z. After sim_time seconds, the gestore terminates the simulation, deleting all System V objects previously created and all child processes, and showing the user a summary of the simulation.




## INTRODUZIONE ## 
Autore: _Silvestro Stefano Frisullo_

In questo progetto utilizziamo 7 file, tra cui  5 file .c, un makefile ed  un file header.
Abbiamo: 
1. gestore.c 
2. a.c 
3. b.c  
4. morte.c

È presente anche un file _common.c_ che contiene metodi di ausilio a tutti i restanti file.
## AVVIO ##
Per avviare la simulazione digitare il comando _make_ da terminale così da compilare automaticamente tutti i file sorgenti e lanciare il gestore.
Per impostare il numero di processi che si vuole immettere nella popolazione e che resterà tale per tutta la durata del procedimento, basta modificare il valore INIT_PEOPLE nell’ header, il quale deve essere >2 per garantire la funzionalità del progetto.
Le variabili _birth_death_ (la quale stabilisce ogni quanti secondi terminare casualmente un processo),  _sim_time_ (che stabilisce la durata della simulazione) e _genes_ (utile nel calcolo dei genomi), vengono richieste all’utente al momento dell’avvio.

## FUNZIONAMENTO ## 
Il tutto ha  inizio dal gestore che costituisce il nostro processo padre e che darà vita a tutti gli altri processi.
 Esso crea i semafori che serviranno a coordinare lo svolgimento della simulazione, una memoria condivisa (il cui accesso è regolato da un semaforo) visibile a tutti in cui viene inizializzato un array di struct di dimensione _INIT_PEOPLE_ e due code di messaggi, una che verrà utilizzata per le comunicazioni fra i processi  A e B ed un’altra che sarà usata dal gestore  e i processi B.
Il processo appena nato, tramite una **execlp()** eseguita dalla funzione nasci , esegue il proprio codice a seconda del tipo e  gli si passa come parametro la sua posizione in memoria condivisa.
Il gestore prosegue creando la morte con la funzione morte(), la quale non fa altro che eseguire una **fork()** ed una execlp() con parametri birth_death e  sim_time. 
La morte fa uso  della funzione **time()** per avviare un timer che terrà conto del tempo trascorso dall’inizio della simulazione ed in modo ciclico ogni birth_death secondi,  tramite una signal SIGUSR1 indirizzata al gestore, comunicherà che dovrà essere terminato un processo con status a 0 scelto casualmente.
Sarà poi il gestore in seguito alla ricezione della signal a procedere con la terminazione di un processo, tramite l’ handler dd_signal_handler. 
La morte ha anche il compito di porre fine alla simulazione che avverrà dopo sim-time secondi tramite l’invio di una SIGUSR2 al gestore, il quale agirà affinché tutti i processi attivi al momento vengano terminati, compresa la morte stessa.

Dopo la creazione della morte il gestore, tramite l’ausilio di un apposito semaforo, farà partire in modo sincronizzato tutti i processi creati inizialmente, dopodiché non  dovrà fare altro che restare in attesa di un messaggio da una B.
 Il compito principale dei nuovi processi A e B è esclusivamente quello di scegliersi a vicenda e accoppiarsi sulla base dei criteri specificati nella consegna del progetto.
Per iniziare la ricerca della A in memoria centrale, la B imposta il proprio status a 1 per segnalare al gestore che non può essere terminata, dopodiché sceglierà la A con genoma più alto tra quelle con status a 0.
Una volta scelta la A migliore, B la “blocca” impostandone lo status a 1, in modo da evitare che la stessa venga terminata dal gestore o contattata da altre B.
A questo punto B invia alla A un messaggio con il proprio genoma e PID e resta in attesa di una sua risposta.
In caso di risposta negativa, B imposta gli status a 0 e si mette alla ricerca di una nuova A, mentre la A abbassa il proprio target per facilitare un futuro accoppiamento.
In caso di risposta positiva B manda un messaggio al gestore con il proprio PID e il PID della A e attende di essere terminato.

Il gestore, ricevuti i PID dei processi appena accoppiati, richiama la funzione _accoppiamento_birth_ passandole come parametro i due PID, la quale a sua volta dovrà immettere nella popolazione e alla stessa posizione in memoria condivisa altri due processi con caratteristiche ereditate dai processi genitori appena terminati, tra cui ad esempio il nome che dovrà essere la concatenazione dei nomi dei genitori con in più un’altra lettera a caso tra A-Z.
Dopo *sim_time* secondi il gestore termina la simulazione, provvedendo all’eliminazione di tutti gli oggetti System V precedentemente creati e di tutti i processi figli, mostrando all’utente un resoconto della simulazione.
