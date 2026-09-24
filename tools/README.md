# Wykresy pokrycia tagów RFID

Skrypt `plot_tag_coverage.py` tworzy:

- wykres słupkowy liczby anten, które wykryły tag w każdej próbie, osobno
  dla każdej półki, z dwoma rzędami po 10 słupków,
- mapę wykryć z osobną kolumną paneli dla każdej półki: górny panel
  pokazuje górny rząd paczek, a dolny panel dolny rząd; w każdym panelu
  wiersze odpowiadają antenom, a 10 kolumn konkretnym tagom,
- osobny wykres dla każdej anteny pokazujący wartości `1` lub `0` dla
  wszystkich tagów na każdej półce jako dwa rzędy po 10 pól; w każdym
  polu znajduje się numer taga i wynik wykrycia.

Paczki na półce są pokazane jedna nad drugą. Numery rosną od lewej do
prawej: na pierwszej półce górny rząd to 50–59, a dolny 60–69;
na kolejnych półkach obowiązuje ten sam układ.

Pomarańczowe pole na mapie oznacza, że antena wykryła tag w każdej próbie
całego testu. Szare pole oznacza brak odczytu w co najmniej jednej próbie.
Tag nieobecny w CSV jest uwzględniany z wartością `0`.

Jeżeli dla taga `antenna_count` wynosi `0`, jego kolumna otrzymuje dodatkowo
czerwone, kreskowane tło, a numer taga na osi jest czerwony i pogrubiony.
Oznacza to, że żadna antena nie wykryła go niezawodnie we wszystkich próbach.

Skrypt nie otwiera portu COM i nie koliduje z Serial Monitorem PlatformIO.

## Konfiguracja półek

Na początku skryptu znajdują się ustawienia:

```python
FIRST_TAG_NUMBER = 50
TAGS_PER_ROW = 10
ROWS_PER_SHELF = 2
TAGS_PER_SHELF = TAGS_PER_ROW * ROWS_PER_SHELF
SHELF_COUNT = 5
```

Obecna konfiguracja obejmuje 100 opakowań i tagi 50–149. Po dodaniu szóstej
półki wystarczy ustawić `SHELF_COUNT = 6`; wszystkie wykresy automatycznie
rozszerzą się wtedy do 120 opakowań i zakresu 50–169.

## Instalacja

W katalogu głównym projektu:

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r tools\requirements.txt
```

## Użycie

Otwórz `tools\plot_tag_coverage.py` i wklej tabelę z Serial Monitora do
zmiennej `CSV_DATA` na początku pliku:

```python
CSV_DATA = """
epc,antenna_count,A1,A2,A5
000000000000000000000050,2,1,0,1
000000000000000000000051,1,0,1,0
000000000000000000000070,3,1,1,1
000000000000000000000090,2,1,1,0
"""
```

Kolumny `A1`, `A2`, `A5` odpowiadają antenom wykrytym przez ESP32.
Wartość `1` oznacza wykrycie EPC przez antenę we wszystkich próbach.
Jeżeli antena pominęła tag choć raz, otrzymuje `0`. `antenna_count` jest
sumą wartości `1` w danym wierszu.

Po zmianie kryterium trzeba wkleić CSV z nowego testu. Dane utworzone przez
starszy firmware oznaczały wykrycie przynajmniej raz i nie nadają się do
oceny niezawodności.

Możesz również wkleić cały fragment razem z liniami `TAG_CSV_BEGIN` oraz
`TAG_CSV_END` — zostaną automatycznie pominięte.

Następnie uruchom:

```powershell
.\.venv\Scripts\python.exe tools\plot_tag_coverage.py
```

Wykresy otworzą się w oknach i zostaną zapisane jako:

- `tools\output\tag_coverage.png`,
- `tools\output\antenna_tag_matrix.png`,
- `tools\output\antennas\antenna_A01_tags.png` itd.

Jeżeli nie chcesz otwierać okien, ustaw `SHOW_CHART = False` na początku
skryptu.

Wklejone wcześniej dane w starym, dwukolumnowym formacie nadal wygenerują
wykres słupkowy, ale nie zawierają informacji potrzebnych do utworzenia mapy
konkretnych anten.

## Wersja dla dowolnych EPC, bez przypisania do półek

Osobny skrypt `plot_tag_coverage_any_epc.py` przyjmuje wszystkie EPC w formacie
szesnastkowym (również z literami A–F), bez zakresu numerów, wymaganego prefiksu
ani przypisania do półek. Korzysta z tych samych zależności co pierwsza wersja.

Wklej CSV z zakończonego testu do `CSV_DATA` w nowym skrypcie i ustaw liczbę
włożonych tagów:

```python
EXPECTED_TAG_COUNT = 60
CSV_DATA = """
epc,antenna_count,A1,A2,A5
E2801160600002051A2B3C4D,2,1,0,1
300833B2DDD9014000000001,1,0,1,0
ABCD1234567890ABCDEF0001,0,0,0,0
"""
```

Uruchom z katalogu głównego projektu:

```powershell
.\.venv\Scripts\python.exe tools\plot_tag_coverage_any_epc.py
```

Skrypt podaje dwa osobne wyniki:

- **Odczytane przynajmniej raz** — liczba unikalnych EPC obecnych w CSV, także
  z `antenna_count = 0`. Na przykład `58/60` oznacza brak 2 EPC do oczekiwanej
  liczby. Wynik `60/60` oznacza zgodność liczby EPC, a `62/60` nadmiar 2 EPC.
- **Z anteną niezawodną** — liczba EPC, dla których co najmniej jedna antena
  wykryła tag w każdej próbie (`antenna_count > 0`). Dla powyższego przykładu
  wyniki to odpowiednio `3/60` oraz `2/60`.

W aktualnym firmware wiersz z `antenna_count = 0` oznacza tag odczytany podczas
testu, lecz bez anteny, która wykryła go we wszystkich próbach. Nie oznacza
taga nigdy niewykrytego. Sam CSV nie pozwala ustalić, czy taki tag był obecny
w każdej próbie, ale wykrywały go różne anteny. Starsze eksporty zachowują
swoją pierwotną semantykę; niezawodność oceniaj na danych z aktualnego firmware.

Pełne EPC są porównywane bez rozróżniania wielkości liter, z zachowaniem zer
wiodących. Powtórzony identyczny wiersz nie zwiększa liczby tagów; sprzeczne
wiersze dla tego samego EPC powodują błąd. Jeśli wkleisz kilka tabel z nagłówkami,
skrypt użyje tylko ostatniej. Pusta tabela z samym nagłówkiem oznacza wynik `0/60`.

Wynik odpowiada liczbie fizycznych tagów tylko wtedy, gdy każdy ma inne EPC
i w zasięgu nie ma obcych tagów. Bez listy oczekiwanych EPC nie można wskazać,
których tagów brakuje, ani ustalić ich położenia.

Wyniki są zapisywane w osobnym katalogu `tools\output\any_epc\`:

- `tag_coverage.png` — porównanie z oczekiwaną liczbą oraz liczba niezawodnych
  anten dla każdego odczytanego EPC,
- `antenna_tag_matrix.png` — mapa anten i EPC,
- `antenna_tag_counts.png` — porównanie liczby EPC niezawodnie odczytywanych
  przez poszczególne anteny,
- `tag_epc_mapping.csv` — numery porządkowe, pełne EPC i dane anten; przy
  otwieraniu w arkuszu kalkulacyjnym importuj kolumnę `epc` jako tekst,
  aby zachować zera wiodące.

Numery na wykresach wynikają z tekstowego sortowania EPC i nie oznaczają
pozycji na półkach. Po zmianie zbioru EPC ich numery mogą się zmienić.
`TAGS_PER_PANEL = 20` ustawia wyłącznie liczbę słupków na panelu.
`SHOW_CHART = False` wyłącza otwieranie okien. Przy starym formacie CSV bez
kolumn anten powstają tylko `tag_coverage.png` i `tag_epc_mapping.csv`.
