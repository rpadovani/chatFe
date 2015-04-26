#ifndef THREAD_MAIN_H
#define THREAD_MAIN_H

/*
    Il thread main si occupa di gestire tutto il server, così come descritto
    nella consegna.

    Prende in input due array di caratteri che indicano, rispettivamente, il
    file che funge da database utenti e il file che si utilizza come log

    Ritorna -1 in caso di fallimento
*/
int thread_main(char *file_utenti, char *file_log);

#endif
