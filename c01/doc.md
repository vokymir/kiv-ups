# Semestr work

Prsi. Zacit se da po cca 5. cviceni, kde bude navrh protokolu.

# prikazy

- ip address
    - vypise vsechny ip adresy, respektive cesty pryc?, vcetne localhostu
- ip link
    - to samy, ale osekany informace
- ip route
    - ne, tak tady to ukazuje vsechny cesty pryc z pc, vcetne defaultni IP adresy
- ping
    - muzem nekoho pingnout, vlastne na nej zatukat a dostat potvrzeni
- traceroute
    - vypise kudy signal projde, kdyz nekoho napr. pinguju, anebo mu proste neco posilam
    - kdyz pinguju, tak je nejaky *ttl*, (neco) to live. proste potom, co paketa/signal odejde ode me, tak po nekolika dalsich zarizenich umira. to po kolika urcuje ttl.
    - a tenhle prikaz funguje tak, ze vzdycky zvedne ttl o 1, tudiz se dostane o uroven dal (zacina na 1)
    - a je jako povinnost dat vedet, ze packet umrel zrovna u me
    - ne vsichni to delaji, ale vetsinou ano (treba datacentra to nedelaji, aby nedavali info utocnikovi. nebo treba seznam...)
- nc
    - netcat (vyber tu openBSD verzi), posloucha na nejakym portu
    - takze, kdyz das *nc -l IP PORT* tak to posloucha
    - a kdyz das to samy bez -l, tak pak muzes psat a posilat.

# pripojovani

- eryx.zcu.cz
- ares.fav.zcu.cz

# zajimavost

univerzita ma pridelene IP adresy (v4) jako 147.228.xxx.xxx
