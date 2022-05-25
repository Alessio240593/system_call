![alt text](https://github.com/adam-p/markdown-here/raw/master/src/common/images/icon48.png "Logo Title Text 1")

# Syscall-project 
[countdown](https://free.timeanddate.com/countdown/i8aabgkf/n2177/cf101/cm0/cu4/ct0/cs0/ca0/co0/cr0/ss0/cac000/cpc000/pcd8873c/tcfff/fs400/szw2251/szh950/iso2022-06-08T00:00:00/bas2)
---
### _to do:_
#### `CLIENT`
---


#### `SERVER`

1. una volta ricevute tutte e quattro le parti di un file le riunisce nell’ordine corretto e le salva
in un file di testo in cui ognuna delle quattro parti e’ separata dalla successiva da una riga
bianca (carattere newline) ed ha l’intestazione “[Parte j, del file NOMEFILE, spedita dal
processo PID tramite CANALE]” (vedere esempio sotto), dove j è un numero da 1 a 4 in
base alla parte del file, NOMEFILE è il nome del file di origine compreso di percorso
completo, PID è il PID del processo mittente e CANALE è il canale di comunicazione (uno
tra FIFO_1, FIFO_2, MsgQueue e ShdMem). Il file verrà chiamato con lo stesso nome (e
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
1. IL client i non scrive parts (è sempre NULL) 
1. Da un certo punto in poi client stampa fifo1.name = " " in teoria sistemato
2. controllare la funione int_dirlist, deve lavorare con al più 100 file 
3. Completare client
4. sistemare makefile per adattarci a replit
5. Refactoring

---

#### `SERVER`
1. non termina il ciclo di ricezione messaggi 
2. Completare server
3. Refactoring






