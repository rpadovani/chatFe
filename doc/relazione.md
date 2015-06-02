/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/

# Il progetto ChatFe

## Storia del progetto

### La genesi

Ho iniziato a lavorare al progetto dal pomeriggio in cui ho ricevuto la consegna. Il primo passaggio è stato leggere varie volte la consegna, per poi scrivere in pseudocodice la struttura generale del server, in modo da evidenziare da subito la struttura che avrei voluto prendesse.
In particolare mi sono concentrato sul server, lasciando in un secondo momento il client. Oltre ad avere un file sorgente diverso per ogni thread che sarei andato a creare (*main*, *dispatcher* e *worker*) ho deciso di creare un file sorgente per gestire gli utenti, in modo da avere *race conditions* solo all'interno di quel file, e non dover gestire l'accesso da multiple posizioni del programma.
Una volta deciso questa struttura di base nel server, ho scritto del pseudocodice per gestire i punti principali del programma (dove e come lanciare i vari thread).
Dal pseudocodice ho iniziato poi a scrivere le basi del codice vero e proprio: il mio primo obiettivo era creare un programma in grado di essere eseguito, che contenesse almeno un header, per poter poi scrivere il *Makefile*, cosa che non avevo mai fatto senza usare *build system* (come *cmake* o *qmake*).
Ho quindi creato un programma davvero basilare, che si occupava di forkarsi e lanciare un thread in background (thread che stampava un messaggio e si chiudeva).
Con questo programma di base sono poi riuscito a lavorare sul *Makefile*: per quanto la documentazione in rete sia ricca, è stata comunque una cosa che mi ha portato via vario tempo: inoltre ho dovuto migliorarlo lungo tutto lo sviluppo del programma.
Nel Makefile ho impostato come flag per `gcc` sia `-Wall`, come richiesto dalla consenga, che `-Werror`, che trasforma ogni warning in un errore. In questo modo mi sono forzato a correggere i warning man mano che si presentavano, e a non lasciarli in sospeso da correggere in un secondo momento.
In ogni caso, una volta creato il Makefile lo sviluppo è stato più facile, poiché è si è integrato con il mio editor (Atom) e potevo compilare i sorgenti direttamente dall'editor, che fornisce funzioni di debug e correzione molto utili.

### Il server

Una volta creato il makefile la mia attenzione si è concentrata sul server - in un primo momento sul thread main, per poter gestire le eventuali eccezioni e il lancio di worker per ogni client connesso. In un secondo momento sull'implementazione di quella parte di worker che si occupa della ricezione.
Per testare le varie funzioni che scrivevo usavo *telnet*, per verificare che il server ricevesse e processasse i vari messaggi secondo la loro tipologia.
In questo periodo dello sviluppo il server era stato studiato per ricevere i messaggi da *telnet*, c'era quindi del codice in più per gestire i byte di controllo che invia telnet.

### Il client

Una volta creato il thread worker e il suo gestore era giunta l'ora di iniziare ad inviare al server i comandi così come sarebbero stati inviati dal programma definitivo. Ho quindi iniziato a scrivere il client, soffermandomi in particolare sul thread writer (quello che si occupa di comunicare con il server) e implementando solo basilarmente il thread *listener* (quello che si occupa di ricevere dal server), giusto per registrare eventuali risposte del server ai comandi di servizio (*#ls*, *#logout* e registrazione).

### Il thread dispatcher

Una volta deciso come scambiarsi i dati tra server e client il processo di sviluppo è andato avanti contemporaneamente sia per il client che per il server. Al server mancava il thread dispatcher, mentre il client doveva essere perfezionato in modo da leggere e scrivere solo i bytes che il server si aspettava, ed evitare di mandare tutti i messaggi non conformi.

### Finalizzazione e pulizia

Finite di sviluppare le funzioni di base del programma si è trattato solamente di inserire le funzioni mancanti di supporto (per esempio scrivere la funzione che salva l'hash table sul file alla chiusura del server) e controllare che la qualità e lo stile del codice fossero costanti.
In particolare mi sono trovato a riscrivere alcuni if/else per diminuire di un tab l'indentazione di varie parti del codice. Ho infatti sostituito in vari punti del codice strutture del tipo

```
if (ok == TRUE) {
	// Lungo listato di codice
	printf(TRUE);
	return 0;
} else {
	return -1;
}
```

con una forma molto più leggibile del tipo

```
if (ok != TRUE) {
	return -1;
}

// Lungo listato di codice
printf(TRUE);
return 0;
```

### Ulteriori letture

Un'interessante e dettagliata lettura sull'evoluzione e le tempistiche di sviluppo del codice si può avere guardando la [history](https://github.com/rpadovani/chatFe/commits/master) dei commit su GitHub.

## Struttura del codice

### I file

Il codice (presente nella directory `src/`) è diviso in due directory principali, quella riservata al client e quella riservata al server.
Entrambe le directory poi hanno una sola altra directory al loro interno, chiamata `include`, che contiene gli header di tutti i file.

Il client è organizzato in maniera molto semplice. C'è un file principale, `main_client.c`, che si occupa di verificare gli argomenti presi in input dal client, aprire una connessione con il server, effettuare il login (o la registrazione) e, se tutto è andato bene, di lanciare i due thread che si occupano di dialogare con il server:
 - `thread_listener.c` si occupa di aspettare le comunicazioni dal server e printarle in `stdout`. L'unica cosa particolare è che nel caso arrivi un messaggio speciale (`0007#logout`) dal server si occupa di terminare il client
 - `thread_writer.c` si occupa di prendere da `stdin` il messaggio dell'utente, verificare la sua validità, strutturarlo nel formato scelto per comunicare con il server ed inviarlo al server.

Il server invece è leggermente più complicato: ci sono due file `lista.c` e `hash.c`, che sono quelli forniti con la consegna. Di questi file è stato creado l'header inserito in `include/`. Forniscono funzioni per gestire le liste e le hash table.
I file `log.c` e `gestione_utent.c` forniscono funzioni di supporto per, rispettivamente, scrivere sul file di log e per gestire gli utenti. `gestione_utenti.c` in particolare è l'unico file del programma che può scrivere sulla hash table durante l'esecuzione e sul file sul quale vengono salvati gli utenti. In queto modo la mutua esclusione per questi due elementi deve essere gestita solo all'interno di questo file.
Il file `main_server.c` non si occupa di fare molto: si forka per lanciare in background i thread che si occupano del server.

Il cuore del programma sta nei tre file che si occupano dei rispettivi thread.
Il `thread_main.c` si occupa di rimanere in attesa di nuove richieste di connessione e di lanciare un thread worker per ogni nuova connessione.
Il `thread_worker.c` si occupa di ricevere i messaggi da ogni client connesso, e di lanciare le funzioni richieste nel caso di azioni speciali (*#ls*, *#logout*  e registrazione) o di depositare il messaggio nel `thread_dispatcher.c`.
Il `thread_dispatcher.c` a sua volta si occupa di gestire un buffer circolare per inviare i messaggi che ivi sono depositati ai destinatari preposti.

### Protocollo di comunicazione

#### Server - client

Per quanto riguarda i messaggi inviati dal server al client la struttura è basilare. La comunicazione inizia con un int (quindi 4 byte) sul nel quale viene inserito il numero di caratteri che seguono questo int. In questo modo il client si occupa di leggere 4 byte, e poi di leggere n byte così come indicati nei primi 4 byte.

La struttura è quindi:`%04i%s`

C'è un unico messaggio speciale, ed è il messaggio `0007#logout`, che indica al client che il server remoto è stato spento e che quindi deve disconnettersi.

#### Client - server

I messaggi tra client e server hanno una struttura simile, anche se più complicata.
Il messaggio inizia con un char che rappresenta il tipo di messaggio inviato (registrazione, messaggio singolo, broadcast, etc.)
Dopodiché segue un intero che indica la lunghezza del campo successivo, che è il campo destinatario, e poi un altro int che indica la lunghezza dell'ultimo campo, il campo messaggio.

Il campo mittente è inutile da inserire, perché il thread worker che riceve il messaggio sa già qual'è lo username di chi invia il messaggio, essendo ogni worker dedicato a una client solo.

Il campo destinatario può essere nullo, nel caso dei messaggi di servizio (registrazione, login, logout). In tal caso l'int che descrive la lunghezza è comunque presente, ma settato a 0.

La struttura è quindi: `%c%04i%s%04i%s`

**Sia nella comunicazione con il server, che con il client, non viene inserito il terminatore di stringa.** Infatti, sapendo la lunghezza in byte del messaggio, sarebbe un byte inviato in più inutilmente.

## Difficoltà incontrate

In generale non ho trovato particolari difficoltà, oltre a quelle che normalmente si trovano lavorando in C (memoria non allocata od allocata nelle dimensioni sbagliate, riferimento al valore dei puntatori invece che al valore della cella puntata etc.). Difficoltà che sono man mano diminuite più prendevo confidenza con il codice.

Un particolare su cui ho perso abbastanza tempo è stata la terminazione del server. Una volta intercettato il segnale e posto la variabile globale `go` a 0 mi sono accorto che le funzioni nel codice che attendevano un evento (quali `accept`, `read`, i lock sui mutex) non venivano interrotti. Leggendo il manuale di `signal()` mi sono accorto che avrei dovuto usare `sig_action()`, che permette di non settare la flag `SA_RESTART` (presente di default in `signal()`). Se `SA_RESTART` non è settata le funzioni di cui sopra si interrompono con un errore. Il problema è che bisogna installare un signal handler per ogni thread che si è aperto.
Per semplificare quindi il codice ho deciso di installare un solo signal handler, che si occupassse di terminare il resto del codice facendo delle finte chiamate che sbloccahino il resto, per esempio creado una nuova connessione per risvegliare l'accept. Non è sicuramente la soluzione più pulita, ma è quella più facile da implementare.

## Constatazioni sulla qualità del codice

### Il multithreading

Una delle difficoltà maggiori nello sviluppare questo codice è stato implementare i controlli sulle *race conditions*. Eseguendo il tutto in locale (dove le performance sono elevate) mi è stato impossibile ricreare situazioni nelle quali si verificassero malfunzionamenti dovuti a processi in concorrenza per la stessa risorsa. L'implementazione delle mutex è quindi puramente *teorica*, e segue quanto studiato durante il corso.

### Esperienza personale

Non usando da lungo tempo il C ho notato che all'inizio ero decisamente lento a scrivere codice e facevo errori abbastanza banali. Con il passare del tempo invece mi sono trovato più a mio agio.

Anche l'impostazione del programma soffre di contaminazioni che ho da altri linguaggi - la separazione delle funzioni in file per tipologia di funzione vuole ricordare vagamente l'astrazioni per classi del **paradigma OOP**, in particolare Qt/C++.

Una cosa che ho trovato fondamentale è stato impormi fin da subito delle regole ben precise per quanto riguardava lo stile del codice - *soft wrap* a 80 char, *snake_case* per il nome delle variabili, dichiarazione di tutte le funzioni nell'header, 4 spazi per l'indentazione.

Inoltre, l'utilizzo di strumenti di supporto allo sviluppo quali *git* per il versioning del codice e *gdb*, *strace* e *vagrant* per il debug hanno rivestito un ruolo di primaria importanza nell'evidenziare lacune nella logica e nell'implementazione del codice.

### Limiti e lacune

Il programma di per sé ha evidenti limiti e lacune - prima fra tutte, una quasi totale mancanza di controllo degli errori e nessuna forma di resilienza. Una stringa inviata male da parte di un client (o un generico errore di comunicazione) danneggia in modo pressoché irreparabile il flusso di lavoro del thread worker, costringendo a un riavvio del client.

Per quanto riguarda la sicurezza è evidente che non c'è nessun controllo per la sanitazzione dei dati, cosa che permetterebbe ad un eventuale malintenzionato di prendere il controllo del processo.

Molti casi d'uso non sono stati considerati, ad esempio se il server viene lanciato più volte su porte diverse ma con gli stessi file di log e degli utenti si verificano conflitti.
