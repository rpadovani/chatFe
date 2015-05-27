/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#ifndef THREAD_LISTENER
#define THREAD_LISTENER

/*
    La funzione si occupa di rimanere in attesa di messaggi dal server
    (la cui connessione è stata stabilita nel main_client) e di stamparli
    a schermo.
    L'unica cosa particolare che fa è vederee se il server ha inviato il
    messaggio di logout e spegnere conseguentemente il client
 */
void *thread_listener(void *connessione);

#endif
