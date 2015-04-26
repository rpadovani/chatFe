#ifndef MAIN_SERVER_H
#define MAIN_SERVER_H

/*
    La variabile globale go controlla il flusso di esecuzione del programma.
    Fintanto che è diversa da zero, le varie operazioni continuano
 */
extern int go;

/*
    Le due variabili seguenti mantengono il riferimo al nome dei file degli
    utenti e del file di log
 */
extern char *file_log;
extern char *file_utenti;

#endif
