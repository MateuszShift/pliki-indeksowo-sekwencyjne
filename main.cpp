#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ostream>
using namespace std;

#define COEFFICIENT_OF_BLOCKING 4
//#define ENTRIES 1000
#define MAX_INDEX_BLOCK_RECORDS 2

string file;
int totalRecords = 0;
int totalOverflowRecords = 0;
int totalMainRecords = 0;
float alpha = 0.5;
//int recordCount = 0; 
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
    //int entryCount;
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
    //index.entryCount = 0;
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

int countIndexEntriesInBlock(Index &index){
    int count = 0;
    for (int i = 0; i < MAX_INDEX_BLOCK_RECORDS; ++i) {
        if (index.entries[i].key != 0) {
            count++;
        }
    }
    return count;
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
    //recordCount = 0;
    //for(int i = 0; i < COEFFICIENT_OF_BLOCKING; i++){
    //    if(page.records[i].key != 0){
    //        recordCount++;
    //    }
    //}
    return page;
}

Page loadOverflowPage(int pageNumber){
    string dataFileName = file + "Overflow.dat";
    Page page;
    ifstream file(dataFileName, ios::binary);
    file.seekg(pageNumber * sizeof(Page), ios::beg); 
    file.read(reinterpret_cast<char *>(&page), sizeof(Page)); 
    file.close();
    //recordCount = 0;
    //for(int i = 0; i < COEFFICIENT_OF_BLOCKING; i++){
    //    if(page.records[i].key != 0){
    //        recordCount++;
    //    }
    //}
    return page;
}

int totalIndexBlocks() {
    string indexFileName = file + "Index.dat";
    std::ifstream indexFile(indexFileName, std::ios::binary | std::ios::ate); 
    if (!indexFile.is_open()) {
        return 0;
    }

    std::streamsize fileSize = indexFile.tellg();
    indexFile.close();

    return fileSize / sizeof(Index);
}


void addIndexEntry(Index &index, int key, int pagePointer) {
    index.entries[countIndexEntriesInBlock(index)].key = key; //tutaj bylo wczesniej index.entryCount
    index.entries[countIndexEntriesInBlock(index)-1].pagePointer = pagePointer; //tutaj bylo wczesniej index.entryCount
    //index.entryCount++;
}

int savePage(Page &page, int pageNumber){
    string dataFileName = file + ".dat";
    std::fstream file(dataFileName, std::ios::binary | std::ios::in | std::ios::out);

    if (pageNumber == -1) {
        file.seekp(0, std::ios::end);
        file.write(reinterpret_cast<const char *>(&page), sizeof(Page)); 
        int newPageNumber = (file.tellp() / sizeof(Page)) - 1;
        file.close();
        return newPageNumber;
    } else {
        file.seekp(pageNumber * sizeof(Page), std::ios::beg);
        file.write(reinterpret_cast<const char *>(&page), sizeof(Page)); 
        file.close();
        return pageNumber;
    }
}

int saveOverflowPage(Page &overflowPage, int pageNumber){
    string dataFileName = file + "Overflow.dat";
    std::fstream file(dataFileName, std::ios::binary | std::ios::in | std::ios::out);

    if (pageNumber == -1) {
        file.seekp(0, std::ios::end);
        file.write(reinterpret_cast<const char *>(&overflowPage), sizeof(Page)); 
        int newPageNumber = (file.tellp() / sizeof(Page)) - 1;
        file.close();
        return newPageNumber;
    } else {
        file.seekp(pageNumber * sizeof(Page), std::ios::beg);
        file.write(reinterpret_cast<const char *>(&overflowPage), sizeof(Page)); 
        file.close();
        return pageNumber;
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

int findPage(int key) {
    Index nextIndex;
    Index currentIndex;

    currentIndex = loadIndex(0);
    for(int b = 0; b < totalIndexBlocks(); b++){
        for (int i = 0; i < countIndexEntriesInBlock(currentIndex); ++i) { //tutaj bylo wczesniej currentIndex.entryCount
            if (i == MAX_INDEX_BLOCK_RECORDS-1) { //jesli jestesmy na ostatnim rekordzie w bloku
                if (b == totalIndexBlocks()-1){ //jesli jestesmy na ostatnim bloku
                    return currentIndex.entries[i].pagePointer;
                }
                nextIndex = loadIndex(b+1);
                if (key >= currentIndex.entries[i].key && key < nextIndex.entries[0].key){ //sprawdzenie pomiedzy blokami czy klucz jest wiekszy niz klucz w obecnym rekordzie i mniejszy niz klucz w nastepnym bloku
                    return currentIndex.entries[i].pagePointer;                    
                }
                currentIndex = nextIndex;
            } 
            else {
                if (i == countIndexEntriesInBlock(currentIndex)-1) { //tutaj bylo wczesniej currentIndex.entryCount-1 , jesli jestesmy na ostatnim rekordzie w bloku
                    return currentIndex.entries[i].pagePointer;
                } else if (key >= currentIndex.entries[i].key && key < currentIndex.entries[i+1].key){ //jesli klucz jest wiekszy niz klucz w obecnym rekordzie i mniejszy niz klucz w nastepnym rekordzie
                    return currentIndex.entries[i].pagePointer;                    
                }
            }
        }
    }
    return -1;
}


int mergeAndSaveOverflowPage(Page &firstPage, int firstPageIndex, Page &secondPage, int secondPageIndex){
    if(firstPageIndex != secondPageIndex){
        saveOverflowPage(firstPage, firstPageIndex);
        saveOverflowPage(secondPage, secondPageIndex);
        return 1;
    }
    else{
        int firstCounter = countRecords(firstPage);
        int secondCounter = countRecords(secondPage);

        if (firstCounter == secondCounter+1)
        {
            secondPage.records[firstCounter-1] = firstPage.records[firstCounter-1];
            saveOverflowPage(secondPage, secondPageIndex);
            return 1;
        }
    }
    return 0;
}

int countNumberOfMainPages(){
    string dataFileName = file + ".dat";
    ifstream file(dataFileName, ios::binary);
    file.seekg(0, ios::end);
    int numberOfPages = file.tellg() / sizeof(Page);
    file.close();
    return numberOfPages;
}

void showAllData(){
    string dataFileName = file + ".dat";
    string overflowFileName = file + "Overflow.dat";
    ifstream file(dataFileName, ios::binary);
    ifstream overflowFile(overflowFileName, ios::binary);
    Page page;



    for(int i = 0; i < countNumberOfMainPages(); i++){
        page = loadPage(i);
        cout << "Strona " << i << endl;
        for(int j = 0; j < COEFFICIENT_OF_BLOCKING; j++){
            if(page.records[j].key !=0){
                cout << "Klucz: " << page.records[j].key << " Numer rejestracyjny: " << page.records[j].licensePlate << " Wskaznik nadmiarowy: " << page.records[j].overflowPointer << endl;
                //tutaj jeszcze obsluga overflow czyli pewnie while
                //obsluga wypisywania rekordow z pliku nadmiarowego od wskaznika
                if(page.records[j].overflowPointer !=-1){
                    int overflowPageIndex = page.records[j].overflowPointer/(COEFFICIENT_OF_BLOCKING);
                    int overflowRecordIndex = page.records[j].overflowPointer%(COEFFICIENT_OF_BLOCKING);
                    Page overflowPage = loadOverflowPage(overflowPageIndex);
                    Record overflowRecord = overflowPage.records[overflowRecordIndex];
                    cout << "       Klucz: " << overflowRecord.key << " Numer rejestracyjny: " << overflowRecord.licensePlate << " Wskaznik nadmiarowy: " << overflowRecord.overflowPointer << endl;
                    while(overflowRecord.overflowPointer != -1){
                        overflowPageIndex = overflowRecord.overflowPointer/(COEFFICIENT_OF_BLOCKING);
                        overflowRecordIndex = overflowRecord.overflowPointer%(COEFFICIENT_OF_BLOCKING);
                        overflowPage = loadOverflowPage(overflowPageIndex);
                        overflowRecord = overflowPage.records[overflowRecordIndex];
                        cout << "       Klucz: " << overflowRecord.key << " Numer rejestracyjny: " << overflowRecord.licensePlate << " Wskaznik nadmiarowy: " << overflowRecord.overflowPointer << endl;
                    }
                }
            }
            else{
                cout << "---Miejsce puste---" << endl;
            }
            
        }
    }
    file.close();
    overflowFile.close();
}



int addToOverflow(Record &newRecord, Page &mainPage, Record &mainPageRecord, int mainPageIndex){

    if(totalOverflowRecords == 0){ //dodanie pierwszego rekordu do pliku nadmiarowego
        Page overflowPage = createEmptyPage();
        overflowPage.records[0] = newRecord;
        mainPageRecord.overflowPointer = 0;
        savePage(mainPage, mainPageIndex);
        saveOverflowPage(overflowPage, 0);
        totalOverflowRecords++;
        return 0;
    }

    int overflowPageIndex = (totalOverflowRecords-1)/(COEFFICIENT_OF_BLOCKING); //indeks strony w pliku nadmiarowym
    int overflowRecordIndex = (totalOverflowRecords-1)%(COEFFICIENT_OF_BLOCKING); //indeks rekordu na stronie w pliku nadmiarowym

    Page overflowPage;

    if(overflowRecordIndex == COEFFICIENT_OF_BLOCKING-1){ //jesli nie ma miejsca na stronie to tworzymy nowa
        overflowRecordIndex = 0;
        overflowPage = createEmptyPage();
        overflowPage.records[overflowRecordIndex] = newRecord;
        overflowPageIndex++;
    }
    else{ //jesli nie ostatni rekord na stronie
        overflowRecordIndex++;
        overflowPage = loadOverflowPage(overflowPageIndex);
        overflowPage.records[overflowRecordIndex] = newRecord;
    }

    Record addedRecord = overflowPage.records[overflowRecordIndex]; 

    int overflowRecordNumber = COEFFICIENT_OF_BLOCKING*overflowPageIndex+overflowRecordIndex; //numer rekordu w pliku nadmiarowym

    if(mainPageRecord.overflowPointer == -1){ //jesli nie ma wskaznika na rekord w pliku nadmiarowym
        mainPageRecord.overflowPointer = overflowRecordNumber;
        savePage(mainPage, mainPageIndex);
        saveOverflowPage(overflowPage, overflowPageIndex);
        totalOverflowRecords++;
        totalRecords++;
        return 0;
    }
    else{ //jesli jest wskaznik na rekord w pliku nadmiarowym
        int prevOvfRecordNumber = mainPageRecord.overflowPointer; 
        int nextOvfRecordNumber = mainPageRecord.overflowPointer;
        int iteration = 0;

        while(true)
        {
            int nextOvfPageIndex = nextOvfRecordNumber/(COEFFICIENT_OF_BLOCKING); //indeks strony w pliku nadmiarowym
            int nextOvfRecordIndex = nextOvfRecordNumber%(COEFFICIENT_OF_BLOCKING); //indeks rekordu na stronie w pliku nadmiarowym

            Page nextOvfPage = loadOverflowPage(nextOvfPageIndex);
            Record nextOvfRecord = nextOvfPage.records[nextOvfRecordIndex]; 

            if(nextOvfRecord.key == addedRecord.key){
                //moment kiedy znalezlismy duplikat, wyjscie bez zapisu
                cout << "Taki rekord juz istnieje!! Nie dodano ponownie!" << endl;
                return 1;
            }
            else if (nextOvfRecord.key > addedRecord.key){
                addedRecord.overflowPointer = COEFFICIENT_OF_BLOCKING*nextOvfPageIndex+nextOvfRecordIndex;
                if (iteration == 0) 
                {
                    // Zmiana przypisania na main page
                    mainPageRecord.overflowPointer = overflowRecordNumber;
                    //zmiana przypisania overflowPointer na overflow page
                    //nextOvfRecord.overflowPointer = nextOvfRecordNumber;
                    overflowPage.records[overflowRecordIndex] = addedRecord;
                    savePage(mainPage,mainPageIndex);
                    saveOverflowPage(overflowPage, overflowPageIndex);
                    totalOverflowRecords++;
                    totalRecords++;
                    return 2;
                }
                else 
                {
                    // Element pomiedzy dwoma na liscie
                    int prevOvfPageIndex = (prevOvfRecordNumber)/(COEFFICIENT_OF_BLOCKING);
                    int prevOvfRecordIndex = (prevOvfRecordNumber)%(COEFFICIENT_OF_BLOCKING);

                    if (nextOvfPageIndex != prevOvfPageIndex) //tutaj dostaniemy sie gdy rekordy sa na roznych stronach
                    {
                        Page prevOvfPage = loadOverflowPage(prevOvfPageIndex);
                        Record prevOvfRecord = prevOvfPage.records[prevOvfRecordIndex]; 

                        addedRecord.overflowPointer = nextOvfRecordNumber;
                        overflowPage.records[overflowRecordIndex] = addedRecord;
                        prevOvfRecord.overflowPointer = overflowRecordNumber;   
                        prevOvfPage.records[prevOvfRecordIndex] = prevOvfRecord;                     

                        saveOverflowPage(prevOvfPage, prevOvfPageIndex);
                    }
                    else //tutaj dostaniemy sie gdy rekordy sa na tej samej stronie
                    {
                        Record prevOvfRecord = nextOvfPage.records[prevOvfRecordIndex]; 

                        addedRecord.overflowPointer = nextOvfRecordNumber;
                        overflowPage.records[overflowRecordIndex] = addedRecord;
                        prevOvfRecord.overflowPointer = overflowRecordNumber;
                        nextOvfPage.records[prevOvfRecordIndex] = prevOvfRecord;                     
                    }
                    mergeAndSaveOverflowPage(overflowPage, overflowPageIndex, nextOvfPage, nextOvfPageIndex);
                    totalOverflowRecords++;
                    totalRecords++;
                    return 4;
                }
            }
            else
            {
                if(nextOvfRecord.overflowPointer == -1) 
                {
                    // Ostatni element na liscie
                    nextOvfRecord.overflowPointer = overflowRecordNumber; //ustawienie wskaznika na nowy rekord (dokladniej jego indeks w pliku overflow)
                    nextOvfPage.records[nextOvfRecordIndex] = nextOvfRecord;
                    mergeAndSaveOverflowPage(overflowPage, overflowPageIndex, nextOvfPage, nextOvfPageIndex);
                    totalOverflowRecords++;
                    totalRecords++;
                    return 3;
                }
                else 
                {
                    prevOvfRecordNumber = nextOvfRecordNumber;
                    nextOvfRecordNumber = nextOvfRecord.overflowPointer;
                }
            }
            iteration++;
        }
    }
    return 0;
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
        totalMainRecords += 2;
        return 0;
    }

    int pageIndex = findPage(newRecord.key);

    Page page = loadPage(pageIndex);

    for(int i = 0; i < countRecords(page);i++){
        if(newRecord.key == page.records[i].key){
            cout << "Taki rekord juz istnieje" << endl;
            return 0;
        }
        if(i == countRecords(page)-1){ //dodanie rekordu na koniec strony
            if(i < COEFFICIENT_OF_BLOCKING-1){ //jesli jest miejsce na stronie
                page.records[i+1] = newRecord;
                savePage(page, pageIndex);
                totalRecords++;
                totalMainRecords++;
                cout << "dodano rekord na koniec strony " << pageIndex << endl;
                return 0;
            }
            else{ //jesli nie ma miejsca na stronie
                cout << "bedziemy dodawac rekord do strony " << pageIndex << "w miejscu nadmiarowym po rekordzie ostatnim " <<  page.records[i].key << endl;    
                addToOverflow(newRecord, page, page.records[i], pageIndex);
                return 0;    
            }
        }
        else{ //dodanie rekordu w srodku strony
            if(newRecord.key > page.records[i].key && newRecord.key < page.records[i+1].key){
                cout << "bedziemy dodawac rekord do strony " << pageIndex << "w miejscu nadmiarowym pomiędzy rekordami " <<  page.records[i].key << " i " << page.records[i+1].key << endl;
                addToOverflow(newRecord, page, page.records[i], pageIndex);
                return 0;
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

        //jeszcze dodac odczyt z jednego pliku spreparowanego
        //wczytaj dane z pliku testowego
        //przejsc do mozliwosci dodawania itp.

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
                showAllData();
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
