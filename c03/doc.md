# c03

prace se sitemi jako se soubory = tak to stejne unix dela se vsim
open, write, read, close - pro nas prilis jednoduche
open, write, read, close - pro nas prilis jednoduche
BSD socket - na miru sitim, ale stale jako soubor

TCP a UDP zajistuji ruzny sluzby
1:N server - client

TCP
server                                              | client
socket()                                            | socket()
bind(ip, port)                                      | connect(ip,port)
listen()                                            |
=== hotove nastaveni ===
accept()                                            | recv()
send()                                              | send()
recv()                                              |
=== konec appky ===
close()                                             | close()

UDP
server                                              | client
socket()                                            | socket()
bind()                                              |
===
sendto()                                            | sendto()
recvfrom()                                          | recvfrom()
===
close()                                             | close()
