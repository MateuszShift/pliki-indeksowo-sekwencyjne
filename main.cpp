#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ostream>
using namespace std;

#define COEFFICIENT_OF_BLOCKING 2
#define ENTRIES 1000
#define MAX_INDEX_BLOCK_RECORDS 2

string file;
int totalRecords = 0;
int totalOverflowRecords = 0;
float alpha = 0.5;
int recordCount = 0;

//struktury
struct Record {
    int key;          
    char licensePlate[9];
    int overflowPointer = -1; 
};

struct Page{
    Record records[COEFFICIENT_OF_BLOCKING];
};

struct IndexEntry {
    int key;       
    int pagePointer;
};

struct Index {
    int entryCount;
    IndexEntry entries[MAX_INDEX_BLOCK_RECORDS];
};
//wyswietlanie menu
void mainMenu(){
    cout << "===============================================" << endl;
    cout << "1.Wyszukaj rekord i odczytaj" << endl;
    cout << "2.Dodaj rekord" << endl;
    cout << "3.Usuń rekord" << endl;
    cout << "4.Reorganizuj" << endl;
    cout << "5.Wyświetl wszystkie rekordy" << endl;
    cout << "6.Wyświetl indeksy plikow" << endl;
    cout << "7.Wyswietl plik nadmiarowy" << endl;
    cout << "8.Zakończ program" << endl;
    cout << "===============================================" << endl;
}
void choosePath(){
    cout << "===============================================" << endl;
    cout << "1.Wczytaj dane z przygotowanego pliku" << endl;
    cout << "2.Przejdź do pustego programu" << endl;
    cout << "3.Zakończ program" << endl;
    cout << "===============================================" << endl;
}
Index createEmptyIndex() {
    Index index;
    index.entryCount = 0;
    memset(&index.entries, 0, sizeof(index.entries));
    return index;
}

int findIndexEntry(Index &index, int pagePointer) {
    for (int i = 0; i < index.entryCount; ++i) {
        if (index.entries[i].pagePointer == pagePointer) {
            return i; // Zwracamy indeks wpisu w bloku indeksowym
        }
    }

    // Jeśli nie znaleziono pasującego wpisu
    throw runtime_error("Nie znaleziono wpisu w indeksie dla podanej strony.");
}

Index loadIndex(int blockIndex) {
    string indexFileName = file + "Index.dat";
    ifstream indexFile(indexFileName, ios::binary);

    indexFile.seekg(blockIndex * sizeof(Index), ios::beg);
    Index index;
    indexFile.read(reinterpret_cast<char *>(&index), sizeof(Index));
    indexFile.close();

    return index;
}


int saveIndex(Index &index, int blockIndex) {
    string indexFileName = file + "Index.dat";
    fstream indexFile(indexFileName, ios::binary | ios::in | ios::out);

    if (blockIndex == -1) {
        indexFile.seekp(0, ios::end);
        int newBlockIndex = indexFile.tellp() / sizeof(Index);
        indexFile.write(reinterpret_cast<const char *>(&index), sizeof(Index));
        indexFile.close();
        return newBlockIndex;
    } else {
        indexFile.seekp(blockIndex * sizeof(Index), ios::beg);
        indexFile.write(reinterpret_cast<const char *>(&index), sizeof(Index));
        indexFile.close();
        return blockIndex;
    }
}


int countRecords(Page &page){
    int count = 0;
    for (int i = 0; i < COEFFICIENT_OF_BLOCKING; ++i) {
        if (page.records[i].key != 0) {
            count++;
        }
    }
    return count;
}

Page loadPage(int pageNumber){
    string dataFileName = file + ".dat";
    Page page;
    ifstream file(dataFileName, ios::binary);
    file.seekg(pageNumber * sizeof(Page), ios::beg); 
    file.read(reinterpret_cast<char *>(&page), sizeof(Page)); 
    file.close();
    recordCount = 0;
    for(int i = 0; i < COEFFICIENT_OF_BLOCKING; i++){
        if(page.records[i].key != 0){
            recordCount++;
        }
    }
    return page;
}

int totalIndexBlocks() {
    string indexFileName = file + "Index.dat";
    std::ifstream indexFile(indexFileName, std::ios::binary | std::ios::ate); // Otwórz plik i przejdź na koniec
    if (!indexFile.is_open()) {
        return 0; // Jeśli plik nie istnieje lub nie można go otworzyć
    }

    std::streamsize fileSize = indexFile.tellg(); // Pobierz rozmiar pliku
    indexFile.close();

    // Oblicz liczbę bloków na podstawie rozmiaru pliku i wielkości jednego bloku
    return fileSize / sizeof(Index);
}


void addIndexEntry(Index &index, int key, int pagePointer) {
    index.entries[index.entryCount].key = key;
    index.entries[index.entryCount].pagePointer = pagePointer;
    index.entryCount++;
}

int findIndexEntryIndex(Index &index, int pageIndex) {
    for (int i = 0; i < index.entryCount; ++i) {
        if (index.entries[i].pagePointer == pageIndex) {
            return i; // Zwraca indeks odnośnika do strony w bloku indeksowym
        }
    }
    return -1; // Jeśli nie znaleziono, co w praktyce nie powinno się zdarzyć
}

int savePage(Page &page, int pageNumber){
    string dataFileName = file + ".dat";
    std::fstream file(dataFileName, std::ios::binary | std::ios::in | std::ios::out);

    if (pageNumber == -1) {
        file.seekp(0, std::ios::end); // Przesuń wskaźnik na koniec pliku
        file.write(reinterpret_cast<const char *>(&page), sizeof(Page)); 
        int newPageNumber = (file.tellp() / sizeof(Page)) - 1; // Oblicz nowy numer strony
        file.close();
        return newPageNumber;
    } else {
        file.seekp(pageNumber * sizeof(Page), std::ios::beg); // Przesuń wskaźnik na konkretną stronę
        file.write(reinterpret_cast<const char *>(&page), sizeof(Page)); 
        file.close();
        return pageNumber; // Zwraca istniejący numer strony
    }
}


Page createEmptyPage(){
    Page page;
    memset(&page,0,sizeof(Page));
    return page;
}

int saveOverflowPage(Page &page, int pageNumber){
    string overflowFileName = file + "Overflow.dat";
    std::fstream file(overflowFileName, std::ios::binary | std::ios::in | std::ios::out);
    int pageIndex;
    if (pageNumber == -1) {
        file.seekp(0, std::ios::end);
        pageIndex = file.tellp() / sizeof(Page);  
    } else {
        file.seekp(pageNumber * sizeof(Page), std::ios::beg);
        pageIndex = pageNumber;
    }
    file.write(reinterpret_cast<const char *>(&page), sizeof(Page));
    file.close();
    return pageIndex;
}

Record loadRecordFromFile(int currentIndex){
    string dataFileName = file + ".dat";
    ifstream file(dataFileName, ios::binary);
    Record record;
    file.seekg(currentIndex * sizeof(Record), ios::beg);
    file.read(reinterpret_cast<char *>(&record), sizeof(Record));
    file.close();
    return record;
}

Record loadOverflowRecord(int currentIndex){
    return loadRecordFromFile(currentIndex);
}

Record createDummyRecord(){
    Record dummyRecord;
    dummyRecord.key = -1;
    char licensePlate[9] = "DUMMMMMY";
    strcpy(dummyRecord.licensePlate, licensePlate);
    dummyRecord.licensePlate[8] = '\0';
    dummyRecord.overflowPointer = -1;
    return dummyRecord;
}

bool isRecordUnique(int key, Page page){
    for(int i = 0; i < COEFFICIENT_OF_BLOCKING; i++){//spr dla glownej strony
        if(page.records[i].key == key){
            return true;
        }
    }
    //spr dla nadmiarow 
    int indexTemp = -1;
    for(int i = 0; i < COEFFICIENT_OF_BLOCKING; i++){
        if(page.records[i].overflowPointer != -1 && page.records[i].overflowPointer != 0){//jezeli bledy to sprawdzic warunek z 0
            indexTemp = page.records[i].overflowPointer;
            while(indexTemp != -1){
                Record overflowRecord = loadOverflowRecord(indexTemp);
                if(overflowRecord.key == key){
                    return true;
                }
                indexTemp = overflowRecord.overflowPointer;
            }
            
        }
    }
    return false;
    

}

int insertNewRecordOnPage(Page &page, Record &newRecord){
    for(int i = COEFFICIENT_OF_BLOCKING - 1; i >= 0; i--){
        if(i==0 || (newRecord.key > page.records[i-1].key && page.records[i-1].key != 0)){
            page.records[i] = newRecord;
            return 0;
        }else{
            page.records[i] = page.records[i-1];
        }
    }
}

int findPage(int key, Index &currentIndex, Page &currentPage, int &currentIndexBlock, bool &hasSpaceOnNextPage, bool &betweenPages) {
    hasSpaceOnNextPage = false;
    betweenPages = false;

    while (true) {
        // Iteracja przez wpisy w bieżącym bloku indeksowym
        for (int i = 0; i < currentIndex.entryCount; ++i) {
            int currentPagePointer = currentIndex.entries[i].pagePointer;
            currentPage = loadPage(currentPagePointer);

            // Jeśli to ostatni wpis w bloku lub klucz jest mniejszy od następnego klucza
            if (i == currentIndex.entryCount - 1 || key < currentIndex.entries[i + 1].key) {
                // Sprawdź, czy obecna strona ma miejsce
                if (countRecords(currentPage) < COEFFICIENT_OF_BLOCKING) {
                    return currentPagePointer;
                }

                // Sprawdź, czy klucz należy pomiędzy stronami
                if (i < currentIndex.entryCount - 1) {
                    int nextPagePointer = currentIndex.entries[i + 1].pagePointer;
                    Page nextPage = loadPage(nextPagePointer);

                    if (key > currentPage.records[COEFFICIENT_OF_BLOCKING - 1].key && key < nextPage.records[0].key) {
                        if (countRecords(nextPage) < COEFFICIENT_OF_BLOCKING) {
                            hasSpaceOnNextPage = true;
                            currentPage = nextPage;
                            return nextPagePointer;
                        } else {
                            betweenPages = true;
                            return currentPagePointer;
                        }
                    }
                }

                //return currentPagePointer;
            }
        }

        // Jeśli klucz jest większy niż wszystkie w bieżącym bloku indeksowym
        if (currentIndexBlock < totalIndexBlocks() - 1) {
            ++currentIndexBlock;
            currentIndex = loadIndex(currentIndexBlock);
        } else {
            // Ostatni wpis w indeksie
            currentPage = loadPage(currentIndex.entries[currentIndex.entryCount - 1].pagePointer);
            return currentIndex.entries[currentIndex.entryCount - 1].pagePointer;
        }
    }
}


int addRecord(Record &newRecord) {
    int currentIndexBlock = 0;
    
    bool indexModified = false;

    // KROK 1: Dodanie pierwszego rekordu i dummy
    if (totalRecords == 0) {
        Page page = createEmptyPage();
        Record dummyRecord = createDummyRecord();
        page.records[0] = dummyRecord;
        page.records[1] = newRecord;
        page.records[1].overflowPointer = -1;
        savePage(page, 0);
        Index currentIndex = createEmptyIndex();
        addIndexEntry(currentIndex, newRecord.key, 0);
        saveIndex(currentIndex, currentIndexBlock);
        totalRecords += 2;
        return 0;
    }
    Index currentIndex = loadIndex(currentIndexBlock);
    // KROK 2: Znalezienie odpowiedniej strony
    bool hasSpaceOnNextPage = false;
    bool betweenPages = false;
    Page currentPage;
    int pageIndex = findPage(newRecord.key, currentIndex, currentPage, currentIndexBlock, hasSpaceOnNextPage, betweenPages);

    // KROK 3: Sprawdzenie unikalności
    if (isRecordUnique(newRecord.key, currentPage)) {
        cout << "Rekord o podanym kluczu już istnieje!!!" << endl;
        return 0;
    }

    // KROK 4: Dodanie rekordu na istniejącą stronę
    if (countRecords(currentPage) < COEFFICIENT_OF_BLOCKING) {
        insertNewRecordOnPage(currentPage, newRecord);
        savePage(currentPage, pageIndex);

        // Aktualizacja indeksu, jeśli trzeba
        int indexEntryIndex = findIndexEntryIndex(currentIndex, pageIndex);
        if (newRecord.key < currentIndex.entries[indexEntryIndex].key) {
            currentIndex.entries[indexEntryIndex].key = newRecord.key;
            saveIndex(currentIndex, currentIndexBlock);
        }

        totalRecords++;
        return 0;
    }

    // KROK 5: Dodanie nowej strony
    if (!betweenPages && currentPage.records[COEFFICIENT_OF_BLOCKING - 1].key < newRecord.key) {
        Page newPage = createEmptyPage();
        insertNewRecordOnPage(newPage, newRecord);
        int newPageIndex = savePage(newPage, -1);

        // Dodaj nowy wpis do indeksu
        if (currentIndex.entryCount < MAX_INDEX_BLOCK_RECORDS) {
            addIndexEntry(currentIndex, newRecord.key, newPageIndex);
            saveIndex(currentIndex, currentIndexBlock);
        } else {
            Index newIndexBlock = createEmptyIndex();
            addIndexEntry(newIndexBlock, newRecord.key, newPageIndex);
            saveIndex(newIndexBlock, ++currentIndexBlock);
        }

        totalRecords++;
        return 0;
    }

    // KROK 6: Dodanie do przepełnienia
    cout << "Tworzenie strony nadmiarowej..." << endl;
    //pamietac o dosc newralgicznym momencie gdy strona 0:-1,2,3,4 str 1: 5,6,7,8 |koniec bloku| str 2: 10,11,12,13 i dodajemy 9 to wtedy przeskoczy juz do str 2 i na nią bedzie wskazywal wiec to bedzie moment gdy trzeba to bedzie obsluzyc
    //wykryjemy to poprzez to ze to jest kolejny blok indeksowy a jego indeks to 0 w tablicy indeksow na danym bloku i wtedy trzeba wczytac poprzedni blok i 
    return 0;
}


//usuwanie rekordu

//reorganizacja

//wyswietlanie rekordow

//wyswietlanie pliku nadmiarowego

//wyswietlanie indeksow



//tworzenie plikow
void createFiles(){
    string dataFileName = file + ".dat";
    string overflowFileName = file + "Overflow.dat";
    string indexFileName = file + "Index.dat";

    std::ofstream mainFile(dataFileName, std::ios::binary);
    std::ofstream overflowFile(overflowFileName, std::ios::binary);
    std::ofstream indexFile(indexFileName, std::ios::binary);
    mainFile.close();
    overflowFile.close();
    indexFile.close();
}

int manageProgramInput(int choice){ //tutaj dopisac obsluge wyboru w sensei tworzenie plikow albo odpowiednie parsowanie pliku testowego 
    
    if(choice == 1){
        cout << "Podaj nazwe pliku: ";
        cin >> file;
        createFiles();

        //wczytaj dane z pliku testowego

        return 1;
    }else if(choice == 2){
        cout << "Podaj nazwe pliku: ";
        cin >> file;
        createFiles();
        return 2;
    }else if(choice == 3){ //wyjdz z programu   
        return 3;
    }
    return 0;
}

Record getNewRecordData(){
    Record newRecord;
    cout << "Podaj klucz: ";
    cin >> newRecord.key;
    cout << "Podaj numer rejestracyjny: ";
    cin >> newRecord.licensePlate;
    newRecord.licensePlate[8] = '\0';

    return newRecord;
}

void manageChoice(){ //uzupelniac o wywolania funkcji
    int choice;
    Record newRecord;
    
    while(choice != 8){
        mainMenu();
        cout << "Wybierz opcje: ";
        cin >> choice;
        switch(choice){
            case 1:
                //wyszukaj rekord i odczytaj
                break;
            case 2:
                //czesc dodawania rekordu
                newRecord = getNewRecordData();
                addRecord(newRecord);
                break;
            case 3:
                //usun rekord
                break;
            case 4:
                //reorganizuj
                break;
            case 5:
                //wyswietl wszystkie rekordy
                //showDataFile();
                break;
            case 6:
                //wyswietl indeksy plikow
                //showIndexFile();
                break;
            case 7:
                //wyswietl plik nadmiarowy
                //showOverflowFile();
                break;
            case 8:
                //zakoncz program
                break;
            default:
                cout << "Nie ma takiej opcji" << endl;
                break;
        }
    }
}

int main(){
    int choicePath;
    choosePath(); //wybór ścieżki programu
    cout << "Wybierz ścieżkę: ";
    cin >> choicePath;
    if(manageProgramInput(choicePath) == 3){ //dodanie rzeczy z pliku lub przejscie do pustego programu tylko tworząc pliki 
        return 0;
    }
    manageChoice();

    return 0;
}


//26.11 zrobic dodatkowo ten overflow i poprawnie zeby to sie spinalo wszysztko, DODAJ FUNCKJE SPRAWDZAJACA CZY REKORD MA ODPOWIEDNIE PARAMETRY JAK NIE TO WPISZ PONOWNIE
//jezeli wszystkie przypadki beda ogarniete to wtedy zrobic reorganizacje
//dodac usuwanie rekordu jak starczy czasu i bedzie ok dzialalo z reorganizacja i bede wiedzial jak to zrobic
//zmienic indeksowa strukture na dynamiczna np vector