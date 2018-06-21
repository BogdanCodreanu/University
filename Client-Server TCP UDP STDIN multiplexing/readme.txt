Serverul asteapta mesaje pe 3 canale (tcp, udp, stdin) + nr clienti activi.

Cand vine ceva pe canalul tcp, inseamna ca este o conexiune noua.

Cand vine ceva pe canalul unui client, ori este un mesaj legat de IBANK, urmand
sa fie interpretat de catre functia "mesajTCP", ori este o deconectare brusca
a clientului.

Cand vine ceva pe canulul udp, inseamna ca este un mesaj catre serviciul UNLOCK,
care urmeaza sa fie interpretat de catre functia "mesajUDP".

Cand vine ceva pe canulul stdin, si acel ceva este mesajul "quit", atunci se
trimite tuturor clientilor faptul ca trebuie sa se inchide. (acestia asculta pe tcp)

Clientii asculta pe canalele tcp, udp si stdin.

Cand vine un mesaj din stdin, el este verificat si trimis pe tcp sau udp (sau nicaieri
in cazul unui login cand este deja logat).

Mesajele care vin pe tcp sau udp sunt verificate daca au proprietati speciale, cum ar fi
faptul ca serverul s-a inchis si acum trebuei inchis direct clientul, sau ca serviciul
IBANK acum cere un raspuns cu y/n, si orice mesaj de la stdin va fi redirectionat direct
catre server, pe canalul tcp (nu mai verifica daca e logat sau ce fel de comanda e)