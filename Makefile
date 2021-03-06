# Directory che contiene la definizione di tutte le funzioni
IDIRSERVER=src/server/include
IDIRCLIENT=src/client/include
# Directory dove vengono inseriti i file compilati
ODIRSERVER=obj/server
ODIRCLIENT=obj/client

# Librerie da includere nella build
LIBS=-lpthread

# VFlags di GCC per il server e per il client
CFLAGSSERVER=-I$(IDIRSERVER) -Wall -Werror
CFLAGSCLIENT=-I$(IDIRCLIENT) -Wall -Werror

# Lista delle dipendenze da buildare prese dalla directory include
# (prima vengono listate e poi viene preposta ai nomi la directory che li
# contiene)
_DEPSSERVER = thread_main.h gestione_utenti.h hash.h common.h lista.h thread_worker.h thread_dispatcher.h log.h
DEPSSERVER = $(patsubst %,$(IDIRSERVER)/%,$(_DEPSSERVER))

_DEPSCLIENT = main_client.h thread_listener.h thread_writer.h
DEPSCLIENT = $(patsubst %,$(IDIRCLIENT)/%,$(_DEPSCLIENT))

# Lista dei file che verranno prodotti nella directory dei file compilati
# (prima vengono listati poi viene aggiunta la directory)
_OBJSERVER = main_server.o thread_main.o gestione_utenti.o hash.o lista.o thread_worker.o thread_dispatcher.o log.o
OBJSERVER = $(patsubst %,$(ODIRSERVER)/%,$(_OBJSERVER))

_OBJCLIENT = main_client.o thread_listener.o thread_writer.o
OBJCLIENT = $(patsubst %,$(ODIRCLIENT)/%,$(_OBJCLIENT))

# Specifichiamo i comandi
.PHONY: clean
.PHONY: install
.PHONY: chat
.PHONY: server
.PHONY: client
.PHONY: create-folders

# Un po' di magia: creiamo una macro sui file definiti DEPS (cioè i nostri file
# .h) che specifica che ogni file oggetto .o dipende dai file .c e dal file .h
# con lo stesso nome
# La seconda riga indica che per compilare i file .o deve prima compilare i file
# .c usando il compilatore definito in $(CC)
# -o $@ indica il nome del file .o, cioè quello presente prima dei due punti
# all'inizio della riga precedente
# Questo deve avvenire per ogni file elencato in $< $(CFLAGS)
#
# Riassumendo assegna ad ogni file presente nella directory include il proprio
# file di output (sia per la parte server che per la parte client)
$(ODIRSERVER)/%.o: src/server/%.c $(DEPSSERVER)
	@gcc -c -o $@ $< $(CFLAGSSERVER)

$(ODIRCLIENT)/%.o: src/client/%.c $(DEPSCLIENT)
	@gcc -c -o $@ $< $(CFLAGSCLIENT)

# Il comando chat, che essendo il primo è quello di default, si occupa di
# compilare i file sia di server che di client
# Il primo comando si occupa di creare il server, il secondo il client, usando
# le dipendenze indicate in $LIBS
chat: create-folders server client

create-folders:
	@mkdir -p $(ODIRSERVER)
	@mkdir -p $(ODIRCLIENT)
	@mkdir -p bin

server: $(OBJSERVER)
	@gcc -o $(ODIRSERVER)/chat-server $^ $(CFLAGSSERVER) $(LIBS)
	@echo 'Server pronto'

client: $(OBJCLIENT)
	@gcc -o $(ODIRCLIENT)/chat-client $^ $(CFLAGSCLIENT) $(LIBS)
	@echo 'Client pronto'
	@echo 'Ricordati di eseguire make install per spostare gli eseguibili in bin/'

# Copiamo i file eseguibli nella cartella bin
install:
	@cp $(ODIRSERVER)/chat-server bin/chat-server
	@cp $(ODIRCLIENT)/chat-client bin/chat-client
	@echo 'Gli eseguibiili sono in bin/'

# Rimuoviamo i file compilati
clean:
	@rm -f $(ODIRSERVER)/*.o
	@rm -f $(ODIRCLIENT)/*.o
	@rm -f bin/*
	@echo 'Tutto pulito :-)'
