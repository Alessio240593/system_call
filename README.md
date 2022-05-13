![alt text](https://github.com/adam-p/markdown-here/raw/master/src/common/images/icon48.png "Logo Title Text 1")

# Syscall-project 
[countdown](https://free.timeanddate.com/countdown/i8aabgkf/n2177/cf101/cm0/cu4/ct0/cs0/ca0/co0/cr0/ss0/cac000/cpc000/pcd8873c/tcfff/fs400/szw2251/szh950/iso2022-06-08T00:00:00/bas2)
---
### _to do:_
#### `CLIENT`

◦ prepara i quattro messaggi per l’invio,
◦ quando il semaforo consente al Client_i di proseguire esso invia il primo messaggio a
FIFO1, il seconda a FIFO2, il terza a MsgQueue ed il quarto a ShdMem (frecce nere in
Figura 1); all’interno dei messaggi, Client_i invia anche il proprio PID ed il nome del
file “sendme_” (con percorso completo),
◦ chiude il file,
◦ termina.

1. (Client_0) si mette in attesa sulla MsgQueue di un messaggio da parte del server che lo
informa che tutti i file di output sono stati creati dal server stesso e che il server ha concluso.

1. una volta ricevuto tale messaggio Client_0 sblocca i segnali SIGINT e SIGUSR1 e si rimette
in attesa di ricevere uno dei due segnali.
---


#### `SERVER`

- si mette in ricezione ciclicamente su ciascuno dei quattro canali (frecce nere in Figura 1)

1. Alla ricezione dei messaggi dai vari canali esegue le seguenti operazioni:
 memorizza il PID del processo mittente, il nome del file con percorso completo ed il pezzo
di file trasmesso.

1. una volta ricevute tutte e quattro le parti di un file le riunisce nell’ordine corretto e le salva
in un file di testo in cui ognuna delle quattro parti e’ separata dalla successiva da una riga
bianca (carattere newline) ed ha l’intestazione “[Parte j, del file NOMEFILE, spedita dal
processo PID tramite CANALE]” (vedere esempio sotto), dove j è un numero da 1 a 4 in
base alla parte del file, NOMEFILE è il nome del file di origine compreso di percorso
completo, PID è il PID del processo mittente e CANALE è il canale di comunicazione (uno
tra FIFO1, FIFO2, MsgQueue e ShdMem). Il file verrà chiamato con lo stesso nome (e
percorso) del file originale ma con l’aggiunta del postfisso “_out”.

1. quando ha ricevuto e salvato tutti i file invia un messaggio di terminazione sulla coda di
messaggi, in modo che possa essere riconosciuto da Client_0 come messaggio di
conclusione lavori.

1. si rimette in attesa su FIFO 1 di un nuovo valore n.

---
![alt_text](https://upload.wikimedia.org/wikipedia/commons/d/dd/Linux_logo.jpg)

---

### _to check:_
####  `CLIENT`
 


---

#### `SERVER`







