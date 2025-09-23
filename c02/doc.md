# C 02

## ISO OSI

APL             L7  Aplikacni               HTTP/S
PRES            L6  Prezentacni | sifrovani, komprese | JSON, XML, ... | BER, DER (bite/data encoding rule)
SESSION         L5  Relacni     | vydrzi i vypadky (email client session)
TRANS           L4  Transportni Firewall    TCP/UDP, port
NET             L3  Sitova      Router      IP (v4, v6)
DL DATA LINK    L2  Linkova     Switch      MAC
PHY             L1  Fyzicka     Hub         Rozhrani

## TCP/IP

Aplikacni = L5,L6,L7
Transportni = L4
Internetova = L3
Vrstva sitoveho pristupu = L1,L2

## co delame ted

netcat - neparalelni spojeni = peer2peer
        nejjednoduci spojeni co existuje vlastne

'netstat -tpln' nebo 'netstat -tupln' = jaky procesy existujou a nekde poslouchaji

> nc -s 147.228.67.253 -l -p 10000

wireshark
    na pozorovani trafficku, jeste si to musim nejak setupnout hej
tcpdump
    to samy v CLI

## DNS

Prevod z domeny na ip treba.

A               DNS -> IPv4
AAAA            DNS -> IPv6
CNAME           DNS -> DNS      Hierarchicky oddelovac - asi jako rename??
PTR             IP -> DNS       To dela nekdo jinej, nez to ostatni nahore
MX              DNS -> DNS      Alias, tykajici se emailu
SOA, TXT, ...

jak zjistit dns info?

nslookup - kde najdu treba seznam.cz?
> nslookup seznam.cz
> host seznam.cz
to samy, ale pro emaily

dig seznam.cz
primy prepis komunikace

## dalsi

ss, nebo ss -l -4
taky ukaze vsechno

lsof, nebo lsof -i :22
supermocny, musi se docela dost filtrovat

## TELNET

existuje, nesifrovany predchudce SSH
> telnet

## https pozadavky

wget
    stahovatko, nic jinyho

curl
    chytrejsi

## windows

powershell:
    ipconfig
    ipconfig /all
    ping
        ale pingne jen 4x
    ping -t
    tracert
    netstat
    route PRINT
        smerovaci tabulka
    netsh
        svycarsky nozik na vsechno mozny
    netsh interface ipv4 show config
        = ipconfig /all
    netsh interface ipv4 add address name="Ethernet" 18.0.0.45 255.0.0.0
        pridani + odpojeni od site, kvuli mismas
    netsh interface ipv4 set address name="Ethernet" source=dhcp
        zase se pripojime na sit
    nslookup
    ssh
    Get-NetIPAddress | ft IPAddress // ta pipe je pro hezci vystup
        = ip -a
    Resolve-DnsName -Name "seznam.cz"
    Clear-DnsClientCache
    Get-NetTCPConnection
        = netstat
    Test-NetConnection "www.seznam.cz"
