# ⚔️ Empire Rising — Simulation Engine

**Empire Rising** este un simulator tactic de strategie și management de imperiu bazat pe text, dezvoltat integral în **C++ modern (C++20)**. Programul orchestrează interacțiunea complexă dintre forța militară, stabilitatea economică și controlul teritorial într-o simulare dinamică ce evoluează zi de zi. Jucătorul trebuie să recruteze trupe din diverse categorii tactice, să gestioneze aurul imperiului, să exploreze o hartă bidimensională generată dinamic cu relief variat și să unifice regiunile prin asedii, în timp ce gestionează riscul permanent de revoltă al cetăților cucerite.

---

## 🏛️ Pilonii Jocului

*   **Comandă Militară:** Gestionarea unei armate polimorfice cu ierarhii de nivel, clase tactice speciale și costuri zilnice de întreținere (*upkeep*).
*   **Sistem Economic:** Administrarea fluxului de aur al imperiului, colectarea de taxe corelate cu stabilitatea provinciilor și riscuri de faliment.
*   **Lume Vie:** O hartă de $30 \times 20$ generată pseudo-aleatoriu cu forme de relief (munți, păduri, ape, câmpii) și drumuri pavate dinamic care conectează cetățile.
*   **Dinamică Teritorială:** Regiuni guvernate de facțiuni ce pot pendula între loialitate deplină și răzmerițe violente (sistem de rebeliune).

---

## 🛠️ Detalii de Implementare Tehnică (Tema 1 & Tema 2)

Proiectul a fost structurat după cele mai riguroase standarde ale Programării Orientate pe Obiecte, asigurând o decuplare curată prin separarea codului în module declarative (**fișiere `.h`**) și logica executabilă (**fișiere `.cpp`**).

### 1. Moștenire, Polimorfism și Design Patterns (Tema 2)
*   **Ierarhie Polimorfică Extinsă:** Clasa de bază abstractă `Unit` coordonează 5 clase derivate specializate: `Infantry`, `Archer`, `Cavalry`, `GarrisonGuard` și `Hero`. Fiecare clasă își calculează dinamic atacul și atributele.
*   **Interfața Non-Virtuală (NVI):** Implementată riguros în sistemul de afișare. Operatorul `<<` apelează o metodă publică non-virtuală (`display`), care la rândul ei deleagă execuția către metoda protejată virtuală `print()`, garantând un flux de control sigur și extensibil.
*   **Constructori Virtuali (`clone`):** Pentru a permite copierea sigură a obiectelor stocate polimorfic, s-a implementat metoda virtuală `std::unique_ptr<Unit> clone() const` în întreaga ierarhie de trupe.
*   **Downcasting Securizat (`dynamic_cast`):** Filtrarea și identificarea trupelor în cadrul algoritmilor STL se realizează prin `dynamic_cast` aplicat pe pointerii bruți extrași din smart pointeri (`u.get()`), asigurând execuția de logică specifică fiecărei subclase.

### 2. Gestiunea Memoriei și Regula celor 5 (Tema 1 & 2)
*   **Smart Pointers:** Excluderea totală a pointerilor simpli (`raw pointers`) în favoarea `std::unique_ptr` și `std::make_unique` pentru eliminarea completă a scurgerilor de memorie (*memory leaks*).
*   **Idiomul Copy-and-Swap (Rule of 5):** Clasa `ArmyManager` încapsulează un vector polimorfic și gestionează ciclul de viață al resurselor prin definirea explicită a *Constructorului de copiere* (bazat pe clonare), *Move Constructor*, *Move Assignment*, *Destructor* și a unui operator de atribuire robust ce utilizează o funcție de `swap` optimizată ADL.

### 3. Ierarhie Proprie de Excepții (Tema 2)
Controlul fluxului și prevenirea stărilor invalide în runtime se realizează printr-o ierarhie dedicată de excepții, derivată din `std::runtime_error`:
*   `InsufficientGoldException` – Declanșată la tentative de recrutare sau upgrade-uri fără fonduri.
*   `PopulationLimitException` – Aruncată când se atinge capacitatea maximă a unei garnizoane.
*   `CombatException` – Gestionează anomaliile survenite în timpul simulării bătăliilor.
*   `InvalidMovementException` – Previne mutările ilegale pe hartă sau pe tipuri de relief impracticabile.

### 4. Compunerea Claselor și Utilizarea STL Modern (Tema 1)
*   **Arhitectură prin Compunere:** Clasa principală `Simulation` coordonează întregul motor prin compunerea claselor de sine stătătoare: `WorldMap`, `Player`, `WorldClock` și `Logger`. Totodată, o `Zone` compune un obiect `City` și un `ArmyManager`.
*   **Containere și Algoritmi:** Utilizarea extensivă a containerelor `std::vector` și `std::map`. Căutările și filtrările de trupe folosesc algoritmul modern `std::find_if` cu predicate Lambda, iar curățarea unităților eliminate se face eficient via `std::erase_if` (C++20).
*   **Mecanisme Statice:** Urmărirea statisticilor globale se face prin atribute și metode `static` (ex: contorul global de unități create, jurnalul de loguri, generatorul de numere aleatoare `RandomGen`).
*   **Const-Corectitudine:** Folosirea strictă a modificatorului `const` pentru metodele inspectoare și a atributului `[[nodiscard]]` pentru a asigura un cod predictibil și optimizat la compilare.


### Folosiți template-ul corespunzător grupei voastre!

| Laborant  | Link template                                |
|-----------|----------------------------------------------|
| Dragoș B  | https://github.com/Ionnier/oop-template      |
| Tiberiu M | https://github.com/MaximTiberiu/oop-template |
| Marius MC | https://github.com/mcmarius/oop-template     |

### Important!

Aveți voie cu cod generat de modele de limbaj la care nu ați contribuit semnificativ doar dacă documentați riguros acest proces.
Codul generat pus "ca să fie"/pe care nu îl înțelegeți se punctează doar pentru puncte bonus, doar în contextul
în care oferă funcționalități ajutătoare și doar dacă are sens.

Codul din proiect trebuie să poată fi ușor de înțeles și de modificat de către altcineva. Pentru detalii, veniți la ore.

O cerință nu se consideră îndeplinită dacă este realizată doar prin cod generat.

- **Fără cod de umplutură/fără sens!**
- **Fără copy-paste!**
- **Fără variabile globale!**
- **Fără atribute publice!**
- **Pentru T2 și T3, fără date în cod!** Datele vor fi citite din fișier, aveți exemple destule.
- **Obligatoriu** fișiere cu date mai multe din care să citiți, obligatoriu cu biblioteci externe: fișiere (local sau server) sau baze de date
- obligatoriu (TBD) să integrați cel puțin două biblioteci externe pe lângă cele pentru stocare

### Tema 0

- [ ] Nume proiect (poate fi schimbat ulterior)
- [ ] Scurtă descriere a temei alese, ce v-ați propus să implementați

## Tema 1

#### Cerințe
- [ ] definirea a minim **3-4 clase** folosind compunere cu clasele definite de voi; moștenirile nu se iau în considerare aici
- [ ] constructori de inițializare cu parametri pentru fiecare clasă
- [ ] pentru o aceeași (singură) clasă: constructor de copiere, `operator=` de copiere, destructor
<!-- - [ ] pentru o altă clasă: constructor de mutare, `operator=` de mutare, destructor -->
<!-- - [ ] pentru o altă clasă: toate cele 5 funcții membru speciale -->
- [ ] `operator<<` pentru **toate** clasele pentru afișare (`std::ostream`) folosind compunere de apeluri cu `operator<<`
- [ ] cât mai multe `const` (unde este cazul) și funcții `private`
- [ ] implementarea a minim 3 funcții membru publice pentru funcționalități netriviale specifice temei alese, dintre care cel puțin 1-2 funcții mai complexe
  - nu doar citiri/afișări sau adăugat/șters elemente într-un/dintr-un vector
- [ ] scenariu de utilizare **cu sens** a claselor definite:
  - crearea de obiecte și apelarea tuturor funcțiilor membru publice în main
  - vor fi adăugate în fișierul `tastatura.txt` DOAR exemple de date de intrare de la tastatură (dacă există); dacă aveți nevoie de date din fișiere, creați alte fișiere separat
- [ ] minim 52-60% din codul propriu să fie C++, `.gitattributes` configurat corect
- [ ] tag de `git`: de exemplu `v0.1`
- [ ] serviciu de integrare continuă (CI) cu **toate bifele**; exemplu: GitHub Actions
- [ ] code review #1 2 proiecte

## Tema 2

#### Cerințe
- [ ] separarea codului din clase în `.h` (sau `.hpp`) și `.cpp`
- [ ] moșteniri:
  - minim o clasă de bază și **3 clase derivate** din aceeași ierarhie; cele 3 derivate moștenesc aceeași clasă de bază
  - ierarhia trebuie să fie cu bază proprie, nu derivată dintr-o clasă predefinită
  - [ ] funcții virtuale (pure) apelate prin pointeri de bază din clasa care conține atributul de tip pointer de bază
    - minim o funcție virtuală va fi **specifică temei** (i.e. nu simple citiri/afișări sau preluate din biblioteci i.e. draw/update/render)
    - constructori virtuali (clone): sunt necesari, dar nu se consideră funcții specifice temei
    - afișare virtuală, interfață non-virtuală
  - [ ] apelarea constructorului din clasa de bază din constructori din derivate
  - [ ] clasă cu atribut de tip pointer la o clasă de bază cu derivate; aici apelați funcțiile virtuale prin pointer de bază, eventual prin interfața non-virtuală din bază
    - [ ] suprascris cc/op= pentru copieri/atribuiri corecte, copy and swap
    - [ ] `dynamic_cast`/`std::dynamic_pointer_cast` pentru downcast cu sens
    - [ ] smart pointers (recomandat, opțional)
- [ ] excepții
  - [ ] ierarhie proprie cu baza `std::exception` sau derivată din `std::exception`; minim **3** clase pentru erori specifice distincte
    - clasele de excepții trebuie să trateze categorii de erori distincte (exemplu de erori echivalente: citire fișiere cu diverse extensii)
  - [ ] utilizare cu sens: de exemplu, `throw` în constructor (sau funcție care întoarce un obiect), `try`/`catch` în `main`
  - această ierarhie va fi complet independentă de ierarhia cu funcții virtuale
- [ ] funcții și atribute `static`
- [ ] STL
- [ ] cât mai multe `const`
- [ ] funcții *de nivel înalt*, de eliminat cât mai mulți getters/setters/funcții low-level
- [ ] minim 75-78% din codul propriu să fie C++
- [ ] la sfârșit: commit separat cu adăugarea unei noi clase derivate fără a modifica restul codului, **pe lângă cele 3 derivate deja adăugate** din aceeași ierarhie
  - noua derivată nu poate fi una existentă care a fost ștearsă și adăugată din nou
  - noua derivată va fi integrată în codul existent (adică va fi folosită, nu adăugată doar ca să fie)
- [ ] tag de `git` pe commit cu **toate bifele**: de exemplu `v0.2`
- [ ] code review #2 2 proiecte

## Tema 3

#### Cerințe
- [ ] 2 șabloane de proiectare (design patterns)
- [ ] o clasă șablon cu sens; minim **2 instanțieri**
  - [ ] preferabil și o funcție șablon (template) cu sens; minim 2 instanțieri
- [ ] minim 80-90% din codul propriu să fie C++
<!-- - [ ] o specializare pe funcție/clasă șablon -->
- [ ] tag de `git` pe commit cu **toate bifele**: de exemplu `v0.3` sau `v1.0`
- [ ] code review #3 2 proiecte

## Instrucțiuni de compilare

Proiectul este configurat cu CMake.

Instrucțiuni pentru terminal:

1. Pasul de configurare
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
# sau ./scripts/cmake.sh configure
```

Sau pe Windows cu GCC folosind Git Bash:
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G Ninja
# sau ./scripts/cmake.sh configure -g Ninja
```

Pentru a configura cu ASan, avem opțiunea `-DUSE_ASAN=ON` (nu merge pe Windows cu GCC):
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DUSE_ASAN=ON
# sau ./scripts/cmake.sh configure -e "-DUSE_ASAN=ON"
```


La acest pas putem cere să generăm fișiere de proiect pentru diverse medii de lucru.


2. Pasul de compilare
```sh
cmake --build build --config Debug --parallel 6
# sau ./scripts/cmake.sh build
```

Cu opțiunea `parallel` specificăm numărul de fișiere compilate în paralel.


3. Pasul de instalare (opțional)
```sh
cmake --install build --config Debug --prefix install_dir
# sau ./scripts/cmake.sh install
```

Vezi și [`scripts/cmake.sh`](scripts/cmake.sh).

Observație: folderele `build/` și `install_dir/` sunt adăugate în fișierul `.gitignore` deoarece
conțin fișiere generate și nu ne ajută să le versionăm.


## Instrucțiuni pentru a rula executabilul

Există mai multe variante:

1. Din directorul de build (implicit `build`). Executabilul se află la locația `./build/oop` după ce a fost rulat pasul de compilare al proiectului (`./scripts/cmake.sh build` - pasul 2 de mai sus).

```sh
./build/oop
```

2. Din directorul `install_dir`. Executabilul se află la locația `./install_dir/bin/oop` după ce a fost rulat pasul de instalare (`./scripts/cmake.sh install` - pasul 3 de mai sus).

```sh
./install_dir/bin/oop
```

3. Rularea programului folosind Valgrind se poate face executând script-ul `./scripts/run_valgrind.sh` din rădăcina proiectului. Pe Windows acest script se poate rula folosind WSL (Windows Subsystem for Linux). Valgrind se poate rula în modul interactiv folosind: `RUN_INTERACTIVE=true ./scripts/run_valgrind.sh`

Implicit, nu se rulează interactiv, iar datele pentru `std::cin` sunt preluate din fișierul `tastatura.txt`.

```sh
RUN_INTERACTIVE=true ./scripts/run_valgrind.sh
# sau
./scripts/run_valgrind.sh
```

4. Pentru a rula executabilul folosind ASan, este nevoie ca la pasul de configurare (vezi mai sus) să fie activat acest sanitizer. Ar trebui să meargă pe macOS și Linux. Pentru Windows, ar merge doar cu MSVC (nerecomandat).

Comanda este aceeași ca la pasul 1 sau 2. Nu merge combinat cu Valgrind.

```sh
./build/oop
# sau
./install_dir/bin/oop
```

## License

The project is licensed under [AGPLv3](LICENSE).

The [template repository](https://github.com/mcmarius/oop-template) itself is licensed under [Unlicense](LICENSE.template).

## Resurse

Raylib - Utilizată pentru nucleul grafic al jocului, gestionarea ferestrei și randarea 2D.

raylib-cpp - Un wrapper C++ object-oriented peste Raylib, care a facilitat utilizarea conceptelor de POO (constructori/destructori) în context grafic.
