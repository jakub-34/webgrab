# webgrab
Podporované příkazy:
* Vypnutí - zajistí ukončení hlavní služby poté, co běžící vlákna ukončí svou práci. Zatím nezpracované požadavky ve frontě budou zahozeny.
* Stažení souboru z URL - vloží do fronty URL souboru, který pak nějaké pracovní vlákno bude stahovat.

./webgrab # spustím službu, další příkazy už vstupují na stdin

> download www.neco.cz/soubor

> download www.necojineho.cz/soubor2

> quit
