# Náplň cvičenia
- zoznámenie sa s perifériou DMA
- prenos dát s USART a DMA

# USART a DMA

- v ukážkovom programe sa na komunikáciu využíva periféria USART spolu s DMA (Direct Memory Access)
- predvedené sú dva spôsoby prenosu dát s DMA - polling, interrupts

- podľa nastavenia makra "POLLING" na "1" alebo "0" sa bude využívať jeden z uvedených spôsobov prenosu dát (makro sa nastavuje vo vlastnosťiach projektu, project->properties)

<p align="center">
    <img src="https://github.com/VRS-Predmet/vrs_cvicenie_7/blob/master/images/nastavenie_makra.PNG" width="750">
</p>

### Polling
- pri tomto spôsobe využívania DMA je v hlavnej slučke programu potrebné volať funkciu, ktorá zistí, či boli prijaté dáta a zabezpečí ich rýchle spracovanie
- funkcia musí byť vyvolávaná periodicky a dostatočne rýchlo, aby sa predišlo strate dát

### Interrupts
- namiesto neustáleho vyvolávania funkcie na spracovanie prijatých dát sú využité 3 zdroje prerušenia - 2x od DMA, 1x od USART
- takto je funkcia na spracovanie dát vyvolaná len vtedy, ak je to potrebné
- prerušenia od DMA - HT(half transfer) a TC(transfer complete)
- prerušenie od USART - IDLE - ak sa zastaví komunikácia po zbernici, po uplinutí času potrebného na prenos jedného znaku sa vyvolá prerušenie

### Konfigurácia DMA
- prenos dát s DMA je nastavený na Rx aj Tx pričom Rx má rozdielnu konfiguráciu ako Tx
- smerovanie prenosu dát - periféria -> pamäť pri Rx, pamäť -> periféria pri Tx
- pri Rx sa ukladajú dáta do vyhradenej pamäti, do ktorej sa zapisuje "kruhovo" (circular mode) - ak sa naplní celé pamäťové miesto, ďalšie dáta sa začnú zapisovať na jeho začiatok a staré sa prepíšu
- pri Tx sa využije normálny mód (normal mode), to znamená, že keď sa dojde na koniec pamäťového miesta, prenos dáť sa ukončí

<p align="center">
    <img src="https://github.com/VRS-Predmet/vrs_cvicenie_7/blob/master/images/dma_config1.PNG" width="650">
</p>

- konfigurácia USART2 sa nelíši od konfigurácie z predchádzajúceho cvičenia
- V NVIC je potrebné povoliť prerušenia pre DMA(všetky používané kanály) aj USART2

- v ukážkovom kóde je navyše ku vygenerovanému kódu ešte doplňené povolenie konkrétnych prerušení pre DMA a USART (IDLE, HT, TC), priradenie pamäťového miesta pre príjem dát a samotné zapnutie DMA pre obsluhu USART2 Rx a Tx


# Zadanie
- Doimplemetovať chýbajúce častí šablóny programu, vďaka ktorému bude MCU komunikovať s PC prostredníctvom USART2 s využitím DMA.
- Okrem spracovania prijatých dát bude program pravidelne posielať informácie o aktuálnom vyťažení pamäte, ktorú využíva DMA pre dáta prijaté cez USART.
- USART komunikácia je obsluhovaná pomocou prerušní HT, TC, IDLE. 
- K vypracovaniu zadania nie je potrebný žiaden dodatočný HW.

### Úlohy:
 1. Stiahnúť/naklonovať vetvu "zadanie_cv7", ktorá predstavuje šablónu projektu, do ktorej je nutné vypracovať nasledujúce úlohy.
 2. Pre svoje zadanie si vtvoriť vlastný github repozitár, kam sa nahraje stiahnutá šablóná.
 
 3. V súbore "Src/usart.c" doplniť konfiguráciu DMA a USART periferie.
 
 4. V subore "Src/usart.c" implementovať funkciu pre obsluhu prijimania dát od USART s DMA - "USART2_CheckDmaReception". Funkcia musí správne obslúžiť prijímanie dáť od DMA a ich následné posielanie na spracovanie. Kontrolovať zaplnenosť DMA Rx buffera a zabrániť pretečeniu a strate prijatých dát. Buffer využívaný pre DMA USART Rx sa musí používať v normálnom režime, nie kruhovom. Veľkosť buffera je nastavena na 256 bytov. Predpokladá sa, že vysielacia strana naraz odošle maximálne 20 znakov.
 
 5. V subore "Src/main.c" implementovať funkciu "proccesDmaData", ktorá spracuje prijatá dáta. Spracovať znamená, že po prijatí súboru znakov vyhodnotí koľko z prijatých znakov boli malé písmená a koľko boli veľké písmená. Každý súbor znakov, ktorý má byť spracovaný sa musí začínať znakom "#" a končiť znakom "$". To znamená, že platný súbor znaokov, pre ktorý sa vyhodnotí počet malých a veľkých písmen bude vyzerať napríklad ako "#Platn15uborZnakov$". Pokiaľ nie je detegovaný štartovací znak "#", funkcia bude prijaté znaky ignorovať. Ak bol prijatý štartovací znak, súbor znakov bude vyhodnotený až po prijatí ukončovacieho znaku "$". Ak po prijatí štartovacieho znaku nabude počas nasledujúcich 35 znakov prijatý ukončovací znak, prijaté dáta sa zahodia a funkcia bude čakať na nový štartovací znak.
 
 6. V subore "Src/main.c" implementovať periodické odosielanie dát o aktuálnom stave DMA Rx buffera cez USART2 do PC. Formát spravý a frekvencia posielania sú špecifikované inštrukciami v komentáry vo while slučke. Implementácia periodického odoielania môže byť priamo vo while slučke poprípade si môžete vytvoriť vlastnú funkciu, ktorú budete vo while slučke volať. V tomto smere máte voľnú ruku. Podstatné je, aby odosielané dáta boli zobraziteľné v PC pomocou terminálu.
 
 7. Odovzdáva sa odkaz k vašemu repozitáru.
