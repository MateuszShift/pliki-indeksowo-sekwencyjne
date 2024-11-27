#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ostream>
using namespace std;

#define COEFFICIENT_OF_BLOCKING 4
#define ENTRIES 1000
#define MAX_INDEX_BLOCK_RECORDS 2

string file;
int totalRecords = 0;
int totalOverflowRecords = 0;
float alpha = 0.5;
int recordCount = 0; //zmienic to 

//struktury
struct Record { //struktura rekordu gdzie glownym wyznacznikiem po czym sortujemy jest klucz a licensePlate jest tylko dodatkiem
    int key;          
    char licensePlate[9];
    int overflowPointer = -1; //tutaj ten wskaznik na nadmiar
};

struct Page{
    Record records[COEFFICIENT_OF_BLOCKING];
};

struct IndexEntry {
    int key;       
    int pagePointer;
};

struct Index { //to odnosi sie do stron glownych raczej nei ma zastosowania do nadmiarowych bo by wymagalo dodatkowego pliku
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

Record createDummyRecord(){
    Record dummyRecord;
    dummyRecord.key = -1;
    char licensePlate[9] = "DUMMMMMY";
    strcpy(dummyRecord.licensePlate, licensePlate);
    dummyRecord.licensePlate[8] = '\0';
    dummyRecord.overflowPointer = -1;
    return dummyRecord;
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

int findPage(int key) {
    Index nextIndex;
    Index currentIndex;

    currentIndex = loadIndex(0);
    for(int b = 0; b < totalIndexBlocks(); b++){
        for (int i = 0; i < currentIndex.entryCount; ++i) {
            if (i == MAX_INDEX_BLOCK_RECORDS-1) {
                if (b == totalIndexBlocks()-1){
                    return currentIndex.entries[i].pagePointer;
                }
                nextIndex = loadIndex(b+1);
                if (key >= currentIndex.entries[i].key && key < nextIndex.entries[0].key){
                    return currentIndex.entries[i].pagePointer;                    
                }
                currentIndex = nextIndex;
            } 
            else {
                if (i == currentIndex.entryCount-1) {
                    return currentIndex.entries[i].pagePointer;
                } else if (key >= currentIndex.entries[i].key && key < currentIndex.entries[i+1].key){
                    return currentIndex.entries[i].pagePointer;                    
                }
            }
        }
    }
    return -1;
}


int addRecord(Record &newRecord) {

    //Dodanie pierwszego rekordu i rekordu Dummy
    if (totalRecords == 0) {
        Page page = createEmptyPage();
        Record dummyRecord = createDummyRecord();
        page.records[0] = dummyRecord;
        page.records[1] = newRecord;
        savePage(page, 0);

        Index currentIndex = createEmptyIndex();
        addIndexEntry(currentIndex, dummyRecord.key, 0);
        saveIndex(currentIndex, 0);
        totalRecords += 2;
        return 0;
    }

    int pageIndex = findPage(newRecord.key);

    Page page = loadPage(pageIndex);

    for(int i = 0; i < countRecords(page);i++){
        if(newRecord.key == page.records[i].key){
            cout << "dsad";
            return 0;
        }
        if(i == countRecords(page)-1){
            if(i < COEFFICIENT_OF_BLOCKING-1){
                page.records[i+1] = newRecord;
                //zapis strony
                return 0;
            }
            else{
                //albo otwieramy nową strone albo overflow z wartosci ostatniej

            }
        }

        else{
            if(newRecord.key > page.records[i].key && newRecord.key < page.records[i+1].key){
                //zwracamy mniejszy i obsluga overflow dla i
            }
        }        
    }    
    
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

