# CBO1-NX

Port di KisakBlack (reimplementazione open source di Call of Duty: Black Ops)
su Nintendo Switch, con devkitPro e libnx.

## Il problema centrale

I fastfile `.ff` contengono struct serializzate a 32 bit con puntatori da
4 byte: un binario a 64 bit non può leggerli. `tools/ffconv/convert.cpp` li
transcodifica offline nel layout LP64, appiattendo i puntatori in una tabella
di rilocazione (formato KBZ1). Su console `src/nx/nx_kbz.cpp` fa solo
alloca-copia-rilocazione-registrazione, senza parsing per asset.

## Regola numero uno

`src/database/db_load.cpp` è la fonte autorevole. Ogni `Load_<T>` va
rispecchiata esattamente, nell'ordine esatto.

## Regole imparate a caro prezzo

- I commenti `sizeof=0x...` negli header sono dimensioni x86. Ricalcola
  sempre per LP64, padding di allineamento compreso: `itemDef_s` è
  `__declspec(align(8))` e ha 4 byte di coda che `Load_Stream` consuma.
- **L'ordine conta.** I caricatori leggono TUTTO il blocco fisso, poi
  risolvono i riferimenti. Invertire i due produce un file che sembra
  corretto — `consumed` può persino tornare — ma è corrotto.
- Allineamenti delle `AllocLoad_`: `raw_byte` 1, `XBlendInfo` 2,
  `FxElemVisStateSample` 4, `GfxPackedVertex0` 16, `itemDef_t` 8.
  Sbagliarli disallinea il cursore del blocco 4.
- Gli indici negativi nel codice decompilato sono campi adiacenti della
  struct, non errori.
- Alcuni loader testano contro `-1`/`-2`, altri solo contro zero. Rispecchia
  quello che fa il loader, non quello che sembra sensato.
- **Non tutto finisce nel blocco 4.** Ogni `Load_<T>Ptr` fa
  `DB_PushStreamPos(0)`, alloca lì la struct dell'asset e `DB_PopStreamPos`
  riavvolge il blocco 0 (`db_stream.cpp:69`): e' memoria di scarto, riusata
  dall'asset successivo. Solo cio' che `Load_<T>` referenzia sotto il suo
  `DB_PushStreamPos(4)` fa avanzare il cursore del blocco 4. Lo stesso vale
  per `Load_GfxTextureLoad`, che ci mette dentro l'intera `GfxImageLoadDef`,
  pixel compresi.
- **`DB_ConvertOffsetToAlias` non e' `DB_ConvertOffsetToPointer`.** Il primo
  *dereferenzia*: l'offset nomina uno slot da 4 byte nel blocco 4 il cui
  contenuto e' il puntatore all'asset. Riempiono quegli slot `XAsset[i].header`
  (a `assetArray + i*8 + 4`) e qualunque campo puntatore di una struct che
  vive nel blocco 4. E' la forma usata da tutti gli handle di asset.
- **Il tag `-2` costa 4 byte nel blocco 4**, `-1` no: `-2` chiama
  `DB_InsertPointer`, che riserva lo slot alias. Dimenticarli manda il
  cursore corto di poco e i riferimenti dedup cadono dentro l'allocazione
  precedente invece che sul suo inizio -- e' il sintomo da cercare.

## Build e test

Dalla shell MSYS2 UCRT64 (`D:\Msys2\ucrt64.exe`), non da devkitPro:

    cd /c/dev/CBO1-NX/tools/ffconv
    g++ -O2 -std=c++17 -Wall convert.cpp -o convert.exe -lz
    FFDBG=1 ./convert.exe "/d/Call of Duty Black Ops/zone/Common/code_post_gfx_mp.ff" /d/kbz/code_post_gfx_mp.kbz 2>&1 | tail -20

`git` non è disponibile in UCRT64: usa la shell devkitPro.

## Stato

`code_post_gfx_mp` (664 asset, 20 tipi) e `Italian/it_code_post_gfx_mp`
(8194 asset) si convertono per intero, con `block4 x86 cursor == blockSize[4]`
e zero riferimenti irrisolti. Quella riga `MATCH` e' l'autocontrollo: se dice
`MISMATCH`, la differenza fra i due numeri e' esattamente quanto si e'
sbagliato nelle riserve del blocco 4.

Strumenti di diagnosi, tutti dietro variabile d'ambiente:

- `FFDBG=1` — una riga per asset piu', in fondo, i conteggi di risoluzione.
  Per ogni riferimento irrisolto stampa in quale allocazione e' caduto e a
  quale riga di `convert.cpp` quell'allocazione e' stata riservata.
- `FFB4=1` — ogni singola riserva del blocco 4 con offset, dimensione e riga.
  Serve a isolare una deriva di pochi byte confrontando l'offset atteso da un
  riferimento con quello emesso.
- `FFMTRACE=1` — il nome di finestra di ogni menuDef e itemDef man mano che
  vengono letti: finche' il cursore e' allineato sono tutti identificatori
  leggibili, appena si disallinea diventano `<noname>`.