#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ostream>
using namespace std;

#define COEFFICIENT_OF_BLOCKING 4
#define MAX_INDEX_BLOCK_RECORDS 10

string file;
int totalRecords = 0;
int totalOverflowRecords = 0;
int totalMainRecords = 0;
float alpha = 0.75; //to do zmiany
float beta = 0.95; //to do zmiany (wspolczynnik ilosci rekordow w pliku nadmiarowym do ilosci rekordow w pliku glownym)
int totalReads = 0;
int totalWrites = 0;

int totalReorganiseReads = 0;
int totalReorganiseWrites = 0;
int totalReorganisationsCounter = 0;
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
    IndexEntry entries[MAX_INDEX_BLOCK_RECORDS];
};

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
    memset(&index.entries, 0, sizeof(index.entries));
    return index;
}

void createFiles(string &fileName){
    string dataFileName = fileName + ".dat";
    string overflowFileName = fileName + "Overflow.dat";
    string indexFileName = fileName + "Index.dat";
    std::ofstream mainFile(dataFileName, std::ios::binary);
    std::ofstream overflowFile(overflowFileName, std::ios::binary);
    std::ofstream indexFile(indexFileName, std::ios::binary);
    mainFile.close();
    overflowFile.close();
    indexFile.close();
}

Index loadIndex(int blockIndex, string &fileName) {
    string indexFileName = fileName + "Index.dat";
    ifstream indexFile(indexFileName, ios::binary);
    indexFile.seekg(blockIndex * sizeof(Index), ios::beg);
    Index index;
    indexFile.read(reinterpret_cast<char *>(&index), sizeof(Index)); //wczytanie bloku indeksowego
    indexFile.close();
    totalReads++;
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

int saveIndex(Index &index, int blockIndex, string &fileName) {
    string indexFileName = fileName + "Index.dat";
    fstream indexFile(indexFileName, ios::binary | ios::in | ios::out);
    indexFile.seekp(blockIndex * sizeof(Index), ios::beg);
    indexFile.write(reinterpret_cast<const char *>(&index), sizeof(Index));
    indexFile.close();
    totalWrites++;
    return blockIndex;
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

Page loadPage(int pageNumber, string &fileName){
    string dataFileName = fileName + ".dat";
    Page page;
    ifstream dataFile(dataFileName, ios::binary);
    dataFile.seekg(pageNumber * sizeof(Page), ios::beg); 
    dataFile.read(reinterpret_cast<char *>(&page), sizeof(Page)); 
    dataFile.close();
    totalReads++;
    return page;
}

static int buffNumber = -1;

Page loadOverflowPage(int pageNumber, int count, string &fileName){
    static Page pageBuff; 
    Page page;
    if (buffNumber != pageNumber)
    {
        buffNumber = pageNumber;
        string dataFileName = fileName + "Overflow.dat";
        ifstream overflowFile(dataFileName, ios::binary);
        overflowFile.seekg(pageNumber * sizeof(Page), ios::beg); 
        overflowFile.read(reinterpret_cast<char *>(&page), sizeof(Page)); 
        overflowFile.close();
        totalReads += count;
        pageBuff = page;
    } else {
        page = pageBuff;
    }
    return page;
}

int totalIndexBlocks(string &fileName) {
    string indexFileName = fileName + "Index.dat";
    std::ifstream indexFile(indexFileName, std::ios::binary | std::ios::ate);
    std::streamsize fileSize = indexFile.tellg(); 
    indexFile.close();
    return fileSize / sizeof(Index);
}

void addIndexEntry(Index &index, int key, int pagePointer) {
    index.entries[countIndexEntriesInBlock(index)].key = key; 
    index.entries[countIndexEntriesInBlock(index)-1].pagePointer = pagePointer;
}

int savePage(Page &page, int pageNumber, string &fileName){
    string dataFileName = fileName + ".dat";
    std::fstream dataFile(dataFileName, std::ios::binary | std::ios::in | std::ios::out);
    dataFile.seekp(pageNumber * sizeof(Page), std::ios::beg);
    dataFile.write(reinterpret_cast<const char *>(&page), sizeof(Page)); 
    dataFile.close();
    totalWrites++;
    return pageNumber;
}

int saveOverflowPage(Page &overflowPage, int pageNumber, string &fileName){
    string dataFileName = fileName + "Overflow.dat";
    std::fstream overflowFile(dataFileName, std::ios::binary | std::ios::in | std::ios::out);

    buffNumber = -1;
    overflowFile.seekp(pageNumber * sizeof(Page), std::ios::beg);
    overflowFile.write(reinterpret_cast<const char *>(&overflowPage), sizeof(Page)); 
    overflowFile.close();
    totalWrites++;
    return pageNumber;
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

    currentIndex = loadIndex(0,file);
    for(int b = 0; b < totalIndexBlocks(file); b++){
        for (int i = 0; i < countIndexEntriesInBlock(currentIndex); ++i) {
            if (i == MAX_INDEX_BLOCK_RECORDS-1) { //jesli jestesmy na ostatnim rekordzie w bloku
                if (b == totalIndexBlocks(file)-1){ //jesli jestesmy na ostatnim bloku
                    return currentIndex.entries[i].pagePointer;
                }
                nextIndex = loadIndex(b+1, file);
                if (key >= currentIndex.entries[i].key && key < nextIndex.entries[0].key){ //sprawdzenie pomiedzy blokami czy klucz jest wiekszy niz klucz w obecnym rekordzie i mniejszy niz klucz w nastepnym bloku
                    return currentIndex.entries[i].pagePointer;                    
                }
                currentIndex = nextIndex; //to przepisanie powoduje ze w nastepnej iteracji petli for bedziemy mieli nextIndex jako currentIndex
            } 
            else {
                if (i == countIndexEntriesInBlock(currentIndex)-1) { //jesli jestesmy na ostatnim rekordzie w bloku
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
        saveOverflowPage(firstPage, firstPageIndex, file);
        saveOverflowPage(secondPage, secondPageIndex, file);
        return 1;
    }
    else{
        int firstCounter = countRecords(firstPage);
        int secondCounter = countRecords(secondPage);

        if (firstCounter == secondCounter+1)
        {
            secondPage.records[firstCounter-1] = firstPage.records[firstCounter-1]; 
            saveOverflowPage(secondPage, secondPageIndex, file);
            return 1;
        }
    }
    return 0;
}

int countNumberOfMainPages(string &fileName){
    string dataFileName = fileName + ".dat";
    ifstream dataFile(dataFileName, ios::binary);
    dataFile.seekg(0, ios::end);
    int numberOfPages = dataFile.tellg() / sizeof(Page);
    dataFile.close();
    return numberOfPages;
}

void findRecordByKey(int key){
    totalReads = 0;
    totalWrites = 0;
    int pageIndex = findPage(key);
    Page page = loadPage(pageIndex, file);
    for(int i = 0; i < countRecords(page); i++){
        if(page.records[i].key == key){
            cout << "Znaleziono rekord o kluczu: " << key  << endl;
            cout << "Dane znalezionego rekordu: " << key << " " << page.records[i].licensePlate << " " << page.records[i].overflowPointer << endl;
            return;
        }
        if(i == countRecords(page)-1){
            if(page.records[i].overflowPointer != -1){
            int overflowPageIndex = page.records[i].overflowPointer/(COEFFICIENT_OF_BLOCKING);
            int overflowRecordIndex = page.records[i].overflowPointer%(COEFFICIENT_OF_BLOCKING);
            Page overflowPage = loadOverflowPage(overflowPageIndex, 1, file);
            Record overflowRecord = overflowPage.records[overflowRecordIndex];
            if(overflowRecord.key == key){
                cout << "Znaleziono rekord o kluczu: " << key << endl;
                cout << "Dane znalezionego rekordu: " << key << " " << overflowRecord.licensePlate << " " << overflowRecord.overflowPointer << endl;
                return;
            }
            if(overflowRecord.key > key){
                cout << "Nie znaleziono rekordu o kluczu: " << key << endl;
                cout << "wychodzimy wczesniej niz na koncu listy" << endl;
                return;
            }
            while(overflowRecord.overflowPointer != -1){
                overflowPageIndex = overflowRecord.overflowPointer/(COEFFICIENT_OF_BLOCKING);
                overflowRecordIndex = overflowRecord.overflowPointer%(COEFFICIENT_OF_BLOCKING);
                overflowPage = loadOverflowPage(overflowPageIndex,1, file);
                overflowRecord = overflowPage.records[overflowRecordIndex];
                if(overflowRecord.key == key){
                    cout << "Znaleziono rekord o kluczu: " << key << endl;
                    cout << "Dane znalezionego rekordu: " << key << " " << overflowRecord.licensePlate << " " << overflowRecord.overflowPointer << endl;
                    return;
                }
                if (overflowRecord.key > key)
                {
                    cout << "Nie znaleziono rekordu o kluczu: " << key << endl;
                    cout << "wychodzimy wczesniej niz na koncu listy" << endl;
                    return;
                }
            }
            }
            cout << "Nie znaleziono rekordu o kluczu: " << key << endl;
            return;
        }
        if(page.records[i].key < key && page.records[i+1].key > key){
            if(page.records[i].overflowPointer != -1){
            int overflowPageIndex = page.records[i].overflowPointer/(COEFFICIENT_OF_BLOCKING);
            int overflowRecordIndex = page.records[i].overflowPointer%(COEFFICIENT_OF_BLOCKING);
            Page overflowPage = loadOverflowPage(overflowPageIndex, 1, file);
            Record overflowRecord = overflowPage.records[overflowRecordIndex];
            if(overflowRecord.key == key){
                cout << "Znaleziono rekord o kluczu: " << key << endl;
                cout << "Dane znalezionego rekordu: " << key << " " << overflowRecord.licensePlate << " " << overflowRecord.overflowPointer << endl;
                return;
            }
            if(overflowRecord.key > key){
                cout << "Nie znaleziono rekordu o kluczu: " << key << endl;
                cout << "wychodzimy wczesniej niz na koncu listy" << endl;
                return;
            }
            while(overflowRecord.overflowPointer != -1){
                overflowPageIndex = overflowRecord.overflowPointer/(COEFFICIENT_OF_BLOCKING);
                overflowRecordIndex = overflowRecord.overflowPointer%(COEFFICIENT_OF_BLOCKING);
                overflowPage = loadOverflowPage(overflowPageIndex,1, file);
                overflowRecord = overflowPage.records[overflowRecordIndex];
                if(overflowRecord.key == key){
                    cout << "Znaleziono rekord o kluczu: " << key << endl;
                    cout << "Dane znalezionego rekordu: " << key << " " << overflowRecord.licensePlate << " " << overflowRecord.overflowPointer << endl;
                    return;
                }
                if (overflowRecord.key > key)
                {
                    cout << "Nie znaleziono rekordu o kluczu: " << key << endl;
                    cout << "wychodzimy wczesniej niz na koncu listy" << endl;
                    return;
                }
            }
            }
            cout << "Nie znaleziono rekordu o kluczu: " << key << endl;
            return;
        }        
    }
    cout << "Nie znaleziono rekordu o kluczu: " << key << endl;
}

void showIndexFile(string &fileName){
    string indexFileName = fileName + "Index.dat";
    ifstream indexFile(indexFileName, ios::binary);
    Index index;
    for(int i = 0; i < totalIndexBlocks(file); i++){
        index = loadIndex(i, file);
        cout << "Blok indeksowy " << i << endl;
        for(int j = 0; j < MAX_INDEX_BLOCK_RECORDS; j++){
            if(index.entries[j].key != 0){
                cout << "Klucz: " << index.entries[j].key << " Wskaznik do strony: " << index.entries[j].pagePointer << endl;
            }
            else{
                cout << "---Miejsce puste---" << endl;
            }
        }
    }
    indexFile.close();
}

Page loadOverflowPageShow(int pageNumber, string &fileName){
    string dataFileName = fileName + "Overflow.dat";
    ifstream overflowFile(dataFileName, ios::binary);
    Page page;
    overflowFile.seekg(pageNumber * sizeof(Page), ios::beg); 
    overflowFile.read(reinterpret_cast<char *>(&page), sizeof(Page)); 
    overflowFile.close();
    totalReads++;
    return page;
}

void showAllData(string &fileName){
    string dataFileName = fileName + ".dat";
    string overflowFileName = fileName + "Overflow.dat";
    ifstream dataFile(dataFileName, ios::binary);
    ifstream overflowFile(overflowFileName, ios::binary);
    Page page;

    for(int i = 0; i < countNumberOfMainPages(fileName); i++){
        page = loadPage(i, fileName);
        cout << "Strona " << i << endl;
        for(int j = 0; j < COEFFICIENT_OF_BLOCKING; j++){
            if(page.records[j].key !=0){
                cout << "Klucz: " << page.records[j].key << " Numer: " << page.records[j].licensePlate << " Wskaznik nadmiarowy: " << page.records[j].overflowPointer << endl;
                if(page.records[j].overflowPointer !=-1){
                    int overflowPageIndex = page.records[j].overflowPointer/(COEFFICIENT_OF_BLOCKING);
                    int overflowRecordIndex = page.records[j].overflowPointer%(COEFFICIENT_OF_BLOCKING);
                    Page overflowPage = loadOverflowPage(overflowPageIndex,1, fileName);
                    Record overflowRecord = overflowPage.records[overflowRecordIndex];
                    cout << "           Klucz: " << overflowRecord.key << " Numer: " << overflowRecord.licensePlate << " Wskaznik nadmiarowy: " << overflowRecord.overflowPointer << endl;
                    while(overflowRecord.overflowPointer != -1){
                        overflowPageIndex = overflowRecord.overflowPointer/(COEFFICIENT_OF_BLOCKING);
                        overflowRecordIndex = overflowRecord.overflowPointer%(COEFFICIENT_OF_BLOCKING);
                        overflowPage = loadOverflowPage(overflowPageIndex,1, fileName);
                        overflowRecord = overflowPage.records[overflowRecordIndex];
                        cout << "           Klucz: " << overflowRecord.key << " Numer: " << overflowRecord.licensePlate << " Wskaznik nadmiarowy: " << overflowRecord.overflowPointer << endl;
                    }
                }
            }
            else{
                cout << "---------Miejsce puste---------" << endl;
            }   
        }
    }
    dataFile.close();
    overflowFile.close();
}

static Index newIndexPage;
static int newIndexPageNum = 0;
static Page newDataPage;
static int newDataPageNum = 0;

void flushNewPages(string &fileName){
    saveIndex(newIndexPage, newIndexPageNum, fileName);
    savePage(newDataPage, newDataPageNum, fileName);
}

void addRecordToPage(Record &record, string &fileName){
    static int newIndexRecNum = 0;
    static int newDataRecNum = 0;

    int coefForNewPage = COEFFICIENT_OF_BLOCKING*alpha;
    
    if(record.key == -1){
        newIndexRecNum = 0;
        newIndexPageNum = 0;
        newDataRecNum = 0;
        newDataPageNum = 0;
        createFiles(fileName);
        newDataPage = createEmptyPage();
        newIndexPage = createEmptyIndex();
        addIndexEntry(newIndexPage, record.key, 0);
        newDataPage.records[0] = record;
        newDataPage.records[0].overflowPointer = -1;
        newDataRecNum++;
        newIndexRecNum++;
    }
    else{
        if (newDataRecNum < coefForNewPage){ //jesli jest miejsce na stronie wedlug wspolczynnika
            newDataPage.records[newDataRecNum] = record;
            newDataPage.records[newDataRecNum].overflowPointer = -1;
            newDataRecNum++;
        }
        else{
            savePage(newDataPage, newDataPageNum, fileName);
            newDataPageNum++;
            newDataPage = createEmptyPage();
            if (newIndexRecNum >= MAX_INDEX_BLOCK_RECORDS){ //jesli blok indeksowy jest pelny
                saveIndex(newIndexPage, newIndexPageNum, fileName);
                newIndexPage = createEmptyIndex();
                newIndexPageNum++;
                newIndexRecNum = 0;
            }    
            addIndexEntry(newIndexPage, record.key, newDataPageNum);
            newIndexRecNum++;
            newDataRecNum = 0;
            newDataPage.records[newDataRecNum] = record;
            newDataPage.records[newDataRecNum].overflowPointer = -1;
            newDataRecNum++;
        }
    }
}

void reorganise(string &fileName){
    string dataFileName = fileName + ".dat";
    string overflowFileName = fileName + "Overflow.dat";
    string indexFileName = fileName + "Index.dat";
    ifstream dataFile(dataFileName, ios::binary);
    ifstream overflowFile(overflowFileName, ios::binary);
    Page page;
    string tempFileName = fileName + "Temp";

    for(int i = 0; i < countNumberOfMainPages(fileName); i++){
        page = loadPage(i, fileName);
        for(int j = 0; j < COEFFICIENT_OF_BLOCKING; j++){
            if(page.records[j].key !=0){
                addRecordToPage(page.records[j], tempFileName);
                if(page.records[j].overflowPointer !=-1){
                    int overflowPageIndex = page.records[j].overflowPointer/(COEFFICIENT_OF_BLOCKING);
                    int overflowRecordIndex = page.records[j].overflowPointer%(COEFFICIENT_OF_BLOCKING);
                    Page overflowPage = loadOverflowPage(overflowPageIndex,1, file);
                    Record overflowRecord = overflowPage.records[overflowRecordIndex];
                    addRecordToPage(overflowRecord, tempFileName);
                    if(overflowRecord.overflowPointer != -1)
                    {
                        while(overflowRecord.overflowPointer != -1){
                            overflowPageIndex = overflowRecord.overflowPointer/(COEFFICIENT_OF_BLOCKING);
                            overflowRecordIndex = overflowRecord.overflowPointer%(COEFFICIENT_OF_BLOCKING);
                            overflowPage = loadOverflowPage(overflowPageIndex,1, file);
                            overflowRecord = overflowPage.records[overflowRecordIndex];
                            addRecordToPage(overflowRecord, tempFileName);
                        }
                    }
                }
            }
        }
    }
    flushNewPages(tempFileName);

    string tempMainFileName = tempFileName + ".dat";
    string tempOvfFileName = tempFileName + "Overflow.dat";
    string tempIndexFileName = tempFileName + "Index.dat";

    remove(dataFileName.c_str());
    remove(overflowFileName.c_str());
    remove(indexFileName.c_str());
    rename(tempMainFileName.c_str(), dataFileName.c_str());
    rename(tempOvfFileName.c_str(), overflowFileName.c_str());
    rename(tempIndexFileName.c_str(), indexFileName.c_str());

    totalMainRecords = totalMainRecords + totalOverflowRecords;
    totalRecords = totalMainRecords;
    totalOverflowRecords = 0;

    dataFile.close();
    overflowFile.close();
}

int addToOverflow(Record &newRecord, Page &mainPage, Record &mainPageRecord, int mainPageIndex){

    if(totalOverflowRecords == 0){ //dodanie pierwszego rekordu do pliku nadmiarowego
        Page overflowPage = createEmptyPage();
        overflowPage.records[0] = newRecord;
        mainPageRecord.overflowPointer = 0;
        savePage(mainPage, mainPageIndex, file);
        saveOverflowPage(overflowPage, 0, file);
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
        overflowPage = loadOverflowPage(overflowPageIndex,1, file);
        overflowPage.records[overflowRecordIndex] = newRecord;
    }

    Record addedRecord = overflowPage.records[overflowRecordIndex]; 

    int overflowRecordNumber = COEFFICIENT_OF_BLOCKING*overflowPageIndex+overflowRecordIndex; //numer rekordu w pliku nadmiarowym

    if(mainPageRecord.overflowPointer == -1){ //jesli nie ma wskaznika na rekord w pliku nadmiarowym
        mainPageRecord.overflowPointer = overflowRecordNumber;
        savePage(mainPage, mainPageIndex, file);
        saveOverflowPage(overflowPage, overflowPageIndex, file);
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
            
            Page nextOvfPage = loadOverflowPage(nextOvfPageIndex,1, file);//to ogarniete buforem w funkcji loadOverflowPage
            Record nextOvfRecord = nextOvfPage.records[nextOvfRecordIndex]; 

            if(nextOvfRecord.key == addedRecord.key){
                cout << "TAKI REKORD JUZ ISTNIEJE W OVERFLOW!!!" << endl;
                return 0;
            }
            else if (nextOvfRecord.key > addedRecord.key){ //jesli rekord jest wiekszy niz dodawany
                addedRecord.overflowPointer = COEFFICIENT_OF_BLOCKING*nextOvfPageIndex+nextOvfRecordIndex;
                if (iteration == 0){ //jesli dodajemy rekord na poczatek listy
                    mainPageRecord.overflowPointer = overflowRecordNumber;
                    overflowPage.records[overflowRecordIndex] = addedRecord;
                    savePage(mainPage,mainPageIndex, file);
                    saveOverflowPage(overflowPage, overflowPageIndex, file);
                    totalOverflowRecords++;
                    totalRecords++;
                    return 0;
                }
                else{//rekord pomiedzy dwoma na liscie
                    int prevOvfPageIndex = (prevOvfRecordNumber)/(COEFFICIENT_OF_BLOCKING);
                    int prevOvfRecordIndex = (prevOvfRecordNumber)%(COEFFICIENT_OF_BLOCKING);
                    if (nextOvfPageIndex != prevOvfPageIndex){ //tutaj dostaniemy sie gdy rekordy sa na roznych stronach
                        Page prevOvfPage = loadOverflowPage(prevOvfPageIndex,1, file);
                        Record prevOvfRecord = prevOvfPage.records[prevOvfRecordIndex]; 
                        if(prevOvfPageIndex == overflowPageIndex){ //jesli rekordy sa na tej samej stronie
                            addedRecord.overflowPointer = nextOvfRecordNumber;
                            overflowPage.records[overflowRecordIndex] = addedRecord;
                            prevOvfRecord.overflowPointer = overflowRecordNumber;
                            overflowPage.records[prevOvfRecordIndex] = prevOvfRecord;
                        }
                        else{ //jesli rekordy sa na roznych stronach
                        addedRecord.overflowPointer = nextOvfRecordNumber;
                        overflowPage.records[overflowRecordIndex] = addedRecord;
                        prevOvfRecord.overflowPointer = overflowRecordNumber;   
                        prevOvfPage.records[prevOvfRecordIndex] = prevOvfRecord;               
                        saveOverflowPage(prevOvfPage, prevOvfPageIndex, file); //zapisanie strony z poprzednim rekordem
                        }
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
                    return 0;
                }
            }
            else{
                if(nextOvfRecord.overflowPointer == -1) {//ostatni element na liscie
                    nextOvfRecord.overflowPointer = overflowRecordNumber; //ustawienie wskaznika na nowy rekord (dokladniej jego indeks w pliku overflow)
                    nextOvfPage.records[nextOvfRecordIndex] = nextOvfRecord;
                    mergeAndSaveOverflowPage(overflowPage, overflowPageIndex, nextOvfPage, nextOvfPageIndex);
                    totalOverflowRecords++;
                    totalRecords++;
                    return 0;
                }
                else{ //jesli nie jest to ostatni element na liscie to przechodzimy do nastepnego
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

    if (totalRecords == 0) {
        Page page = createEmptyPage();
        Record dummyRecord = createDummyRecord();
        page.records[0] = dummyRecord;
        page.records[1] = newRecord;
        savePage(page, 0, file);
        Index currentIndex = createEmptyIndex();
        addIndexEntry(currentIndex, dummyRecord.key, 0);
        saveIndex(currentIndex, 0, file);
        totalRecords += 2;
        totalMainRecords += 2;
        return 0;
    }

    int pageIndex = findPage(newRecord.key);
    Page page = loadPage(pageIndex, file);

    for(int i = 0; i < countRecords(page);i++){
        if(newRecord.key == page.records[i].key){
            cout << "TAKI REKORD JUZ ISTNIEJE !!!" << endl;
            return 0;
        }
        if(i == countRecords(page)-1){ //dodanie rekordu na koniec strony
            if(i < COEFFICIENT_OF_BLOCKING-1){ //jesli jest miejsce na stronie
                page.records[i+1] = newRecord;
                savePage(page, pageIndex, file);
                totalRecords++;
                totalMainRecords++;
                return 0;
            }
            else{ //jesli nie ma miejsca na stronie 
                addToOverflow(newRecord, page, page.records[i], pageIndex);
                if(totalOverflowRecords >= beta*totalMainRecords){
                    //totalReads = 0;
                    //totalWrites = 0;
                    reorganise(file); //to przywrococ pozniej
                    //totalReorganiseReads += totalReads;
                    //totalReorganiseWrites += totalWrites;
                    //totalReorganisationsCounter++;
                    //cout << "Reorganizacja: " << totalReads << " odczytow, " << totalWrites << " zapisow" << endl;
                }
                return 0;    
            }
        }
        else{ //dodanie rekordu w srodku strony
            if(newRecord.key > page.records[i].key && newRecord.key < page.records[i+1].key){
                addToOverflow(newRecord, page, page.records[i], pageIndex);
                if(totalOverflowRecords >= beta*totalMainRecords){
                    //totalReads = 0;
                    //totalWrites = 0;
                    reorganise(file);
                    //totalReorganiseReads += totalReads;
                    //totalReorganiseWrites += totalWrites;
                    //totalReorganisationsCounter++;
                    //cout << "Reorganizacja: " << totalReads << " odczytow, " << totalWrites << " zapisow" << endl;
                }
                return 0;
            }
        }        
    }    
    return 0;
}

void readRecords(const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    char operation;
    Record record;
    int recordIndex = 1;
    totalReorganisationsCounter = 0;
    while (true) {
        if (!inFile.read(reinterpret_cast<char*>(&operation), sizeof(operation))) {
            break;
        }
        if (operation == 'A') {
            if (!inFile.read(reinterpret_cast<char*>(&record), sizeof(Record))) {
                std::cerr << "Błąd: Niekompletny rekord w pliku!" << std::endl;
                break;
            }
            addRecord(record);
        } else if (operation == 'R') {
            reorganise(file);
        }
        ++recordIndex;
    }
    inFile.close();

    //cout << "Łączna liczba zapisów: " << totalWrites << endl;
    //cout << "Łączna liczba odczytów: " << totalReads << endl;
    //cout << "Dla parametrów: alpha = " << alpha << ", beta = " << beta << endl;

    //cout << "łączna liczba zapisow podczas reorganizacji: " << totalReorganiseWrites << endl;
    //cout << "łączna liczba odczytow podczas reorganizacji: " << totalReorganiseReads << endl;
    //cout << "łączna liczba reorganizacji: " << totalReorganisationsCounter << endl;
}

void generateRandomRecords(int amount, string &fileName) {
    srand(time(NULL));
    std::ofstream outFile(fileName, std::ios::binary);

    for (int i = 0; i < amount; ++i) {
        char operation;
        Record record;        
        operation = 'A';
        outFile.write(reinterpret_cast<const char*>(&operation), sizeof(operation));

        if (operation == 'A') {
            record.key = std::rand() % 100001;

            for (int j = 0; j < 3; ++j) {
                record.licensePlate[j] = 'A' + std::rand() % 26;
            }
            int licensePlateLength = 7 + (std::rand() % 2);

            for (int j = 3; j < licensePlateLength; ++j) {
                int choice = std::rand() % 2;
                if (choice == 0) {
                    record.licensePlate[j] = '0' + std::rand() % 10;
                } else {
                    record.licensePlate[j] = 'A' + std::rand() % 26;
                }
            }
            record.licensePlate[licensePlateLength] = '\0';

            cout << "Wygenerowano rekord o kluczu: " << record.key << " i numerze rejestracyjnym: " << record.licensePlate << endl;
            record.overflowPointer = -1;

            outFile.write(reinterpret_cast<const char*>(&record), sizeof(record));
        }
    }

    outFile.close();
    cout << "Wygenerowano " << amount << " rekordów do pliku " << fileName << endl;
}

int manageProgramInput(int choice){ 

    string generateRecordsFile;
    int amount = 0;

    if(choice == 1){

        cout << "Podaj nazwe pliku na ktorym bedziemy przeprowadzac nasz program: ";
        cin >> file;
        createFiles(file);
        //stworz dane testowe (zakomentowac jezeli nie jest to potrzebne)
        cout << "Podaj nazwe pliku testowego (stworzy sie nowy albo zapiszemy nazwe istniejącego): ";
        cin >> generateRecordsFile;
        //cout << "Podaj ilosc rekordow do wygenerowania: ";
        //cin >> amount;
        //generateRandomRecords(amount, generateRecordsFile);

        readRecords(generateRecordsFile);

        return 1;
    }else if(choice == 2){
        cout << "Podaj nazwe pliku: ";
        cin >> file;
        createFiles(file);
        return 2;
    }else if(choice == 3){
        return 3;
    }
    return 0;
}

Record getNewRecordData() {
    Record newRecord;
    do {
        cout << "Podaj klucz (>= 0): ";
        cin >> newRecord.key;
        if (newRecord.key < 0) {
            cout << "Klucz nie może być mniejszy niż 0. Spróbuj ponownie.\n";
        }
    } while (newRecord.key < 0);
    bool validLicensePlate = false;
    do {
        cout << "Podaj numer rejestracyjny (7 lub 8 znaków): ";
        cin >> newRecord.licensePlate;
        int length = 0;
        while (newRecord.licensePlate[length] != '\0' && length < 9) {
            ++length;
        }

        if (length == 7 || length == 8) {
            validLicensePlate = true;
        } else {
            cout << "Numer rejestracyjny musi mieć dokładnie 7 lub 8 znaków. Spróbuj ponownie.\n";
        }
    } while (!validLicensePlate);

    newRecord.licensePlate[8] = '\0';

    newRecord.overflowPointer = -1;

    return newRecord;
}

int countNumberOfOverflowPages(string &fileName){
    string overflowFileName = fileName + "Overflow.dat";
    ifstream overflowFile(overflowFileName, ios::binary);
    overflowFile.seekg(0, ios::end);
    int numberOfPages = overflowFile.tellg() / sizeof(Page);
    overflowFile.close();
    return numberOfPages;
}

void showOverflowFile(string &fileName){
    string overflowFileName = fileName + "Overflow.dat";
    ifstream overflowFile(overflowFileName, ios::binary);
    Page page;
    for(int i = 0;i < countNumberOfOverflowPages(fileName); i++){
        page = loadOverflowPageShow(i, fileName);
        cout << "Strona " << i << endl;
        for(int j = 0; j < COEFFICIENT_OF_BLOCKING; j++){
            if(page.records[j].key !=0){
                cout << "Klucz: " << page.records[j].key << " Numer: " << page.records[j].licensePlate << " Wskaznik nadmiarowy: " << page.records[j].overflowPointer << endl;
            }
            else{
                cout << "---Miejsce puste---" << endl;
            }   
        }
    }
}

void manageChoice(){
    int choice;
    Record newRecord;
    int searchedKey;
    
    while(choice != 8){
        mainMenu();
        cout << "Wybierz opcje: ";
        cin >> choice;
        switch(choice){
            case 1:
                totalReads = 0;
                totalWrites = 0;
                cout << "Podaj klucz rekordu do wyszukania: ";
                cin >> searchedKey;
                if(searchedKey <= 0){
                    cout << "Klucz nie moze byc mniejszy niz 1" << endl;
                    break;
                }
                findRecordByKey(searchedKey);
                cout << "ilosc odczytow: " << totalReads << " ilosc zapisow: " << totalWrites << endl;
                break;
            case 2:
                totalReads = 0;
                totalWrites = 0;
                newRecord = getNewRecordData();
                addRecord(newRecord);
                cout << "ilosc odczytow: " << totalReads << " ilosc zapisow: " << totalWrites << endl;
                break;
            case 3:
                cout << "niestety ale nie ma tej opcji :( "<< endl;
                break;
            case 4:
                totalReads = 0;
                totalWrites = 0;
                reorganise(file);
                cout << "ilosc odczytow: " << totalReads << " ilosc zapisow: " << totalWrites << endl;
                break;
            case 5:
                totalReads = 0;
                totalWrites = 0;
                showAllData(file);
                cout << "ilosc odczytow: " << totalReads << " ilosc zapisow: " << totalWrites << endl;
                break;
            case 6:
                totalReads = 0;
                totalWrites = 0;
                showIndexFile(file);
                cout << "ilosc odczytow: " << totalReads << " ilosc zapisow: " << totalWrites << endl;
                break;
            case 7:
                cout << "Tak wygląda plik nadmiarowy: " << endl;
                showOverflowFile(file);
                break;
            case 8:
                break;
            default:
                cout << "Nie ma takiej opcji" << endl;
                break;
        }
    }
}

int main(){
    int choicePath;
    choosePath();
    cout << "Wybierz ścieżkę: ";
    cin >> choicePath;
    if(manageProgramInput(choicePath) == 3){
        return 0;
    }
    manageChoice();

    return 0;
}