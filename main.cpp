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
float alpha = 0.5;
int totalReads = 0;
int totalWrites = 0;
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
    memset(&index.entries, 0, sizeof(index.entries));
    return index;
}


Index loadIndex(int blockIndex, string &fileName) {
    string indexFileName = fileName + "Index.dat";
    ifstream indexFile(indexFileName, ios::binary);

    indexFile.seekg(blockIndex * sizeof(Index), ios::beg);
    Index index;
    indexFile.read(reinterpret_cast<char *>(&index), sizeof(Index));
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

    if (blockIndex == -1) {
        indexFile.seekp(0, ios::end);
        int newBlockIndex = indexFile.tellp() / sizeof(Index);
        indexFile.write(reinterpret_cast<const char *>(&index), sizeof(Index));
        indexFile.close();
        totalWrites++;
        return newBlockIndex;
    } else {
        indexFile.seekp(blockIndex * sizeof(Index), ios::beg);
        indexFile.write(reinterpret_cast<const char *>(&index), sizeof(Index));
        indexFile.close();
        totalWrites++;
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
}

int savePage(Page &page, int pageNumber, string &fileName){
    string dataFileName = fileName + ".dat";
    std::fstream dataFile(dataFileName, std::ios::binary | std::ios::in | std::ios::out);

    if (pageNumber == -1) {
        dataFile.seekp(0, std::ios::end);
        dataFile.write(reinterpret_cast<const char *>(&page), sizeof(Page)); 
        int newPageNumber = (dataFile.tellp() / sizeof(Page)) - 1;
        dataFile.close();
        totalWrites++;
        return newPageNumber;
    } else {
        dataFile.seekp(pageNumber * sizeof(Page), std::ios::beg);
        dataFile.write(reinterpret_cast<const char *>(&page), sizeof(Page)); 
        dataFile.close();
        totalWrites++;
        return pageNumber;
    }
}

int saveOverflowPage(Page &overflowPage, int pageNumber, string &fileName){
    string dataFileName = fileName + "Overflow.dat";
    std::fstream overflowFile(dataFileName, std::ios::binary | std::ios::in | std::ios::out);

    buffNumber = -1;
    if (pageNumber == -1) {
        overflowFile.seekp(0, std::ios::end);
        overflowFile.write(reinterpret_cast<const char *>(&overflowPage), sizeof(Page)); 
        int newPageNumber = (overflowFile.tellp() / sizeof(Page)) - 1;
        overflowFile.close();
        totalWrites++;
        return newPageNumber;
    } else {
        overflowFile.seekp(pageNumber * sizeof(Page), std::ios::beg);
        overflowFile.write(reinterpret_cast<const char *>(&overflowPage), sizeof(Page)); 
        overflowFile.close();
        totalWrites++;
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

    currentIndex = loadIndex(0,file);
    for(int b = 0; b < totalIndexBlocks(file); b++){
        for (int i = 0; i < countIndexEntriesInBlock(currentIndex); ++i) { //tutaj bylo wczesniej currentIndex.entryCount
            if (i == MAX_INDEX_BLOCK_RECORDS-1) { //jesli jestesmy na ostatnim rekordzie w bloku
                if (b == totalIndexBlocks(file)-1){ //jesli jestesmy na ostatnim bloku
                    return currentIndex.entries[i].pagePointer;
                }
                nextIndex = loadIndex(b+1, file);
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

    for(int i = 0; i < countNumberOfMainPages(file); i++){
        page = loadPage(i, file);
        cout << "Strona " << i << endl;
        for(int j = 0; j < COEFFICIENT_OF_BLOCKING; j++){
            if(page.records[j].key !=0){
                cout << "Klucz: " << page.records[j].key << " Numer: " << page.records[j].licensePlate << " Wskaznik nadmiarowy: " << page.records[j].overflowPointer << endl;
                if(page.records[j].overflowPointer !=-1){
                    int overflowPageIndex = page.records[j].overflowPointer/(COEFFICIENT_OF_BLOCKING);
                    int overflowRecordIndex = page.records[j].overflowPointer%(COEFFICIENT_OF_BLOCKING);
                    Page overflowPage = loadOverflowPage(overflowPageIndex,0, file);
                    Record overflowRecord = overflowPage.records[overflowRecordIndex];
                    cout << "       Klucz: " << overflowRecord.key << " Numer: " << overflowRecord.licensePlate << " Wskaznik nadmiarowy: " << overflowRecord.overflowPointer << endl;
                    while(overflowRecord.overflowPointer != -1){
                        overflowPageIndex = overflowRecord.overflowPointer/(COEFFICIENT_OF_BLOCKING);
                        overflowRecordIndex = overflowRecord.overflowPointer%(COEFFICIENT_OF_BLOCKING);
                        overflowPage = loadOverflowPage(overflowPageIndex,0, file);
                        overflowRecord = overflowPage.records[overflowRecordIndex];
                        cout << "       Klucz: " << overflowRecord.key << " Numer: " << overflowRecord.licensePlate << " Wskaznik nadmiarowy: " << overflowRecord.overflowPointer << endl;
                    }
                }
            }
            else{
                cout << "---Miejsce puste---" << endl;
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
    // Save current index and data page na koncu plikow
    // Save current data page na koncu plikow
    // Save current index page na koncu plikow
    

}

void addRecordToPage(Record &record, string &fileName){
    static int newIndexRecNum = 0;
    static int newDataRecNum = 0;

    if(record.key == -1)
    {
        newIndexRecNum = 0;
        newIndexPageNum = 0;
        newDataRecNum = 0;
        newDataPageNum = 0;

        // Create empty files
        createFiles(fileName);
        
        // Create new pages
        newDataPage = createEmptyPage();
        newIndexPage = createEmptyIndex();

        // Write record to index(0) and data(0) and link index(0) to page 0
        addIndexEntry(newIndexPage, record.key, 0);

        //insert record to data page
        newDataPage.records[0] = record;

        //set pointer to -1
        newDataPage.records[0].overflowPointer = -1;

        newDataRecNum++;
        newIndexRecNum++;
    }
    else 
    {
        if (newIndexRecNum <= (int)(alpha*COEFFICIENT_OF_BLOCKING))
        {
            // Add record here data(newDataRec)
            newDataPage.records[newDataRecNum] = record;
            //set pointer to -1
            newDataPage.records[newDataRecNum].overflowPointer = -1;

            newDataRecNum++;
        }
        else
        {
            // Save current data page
            savePage(newDataPage, newDataPageNum, fileName);

            newDataPageNum++;
            // Add new data page(newDataPage)
            newDataPage = createEmptyPage();

            // Check if index page must be created
            if (newIndexRecNum > MAX_INDEX_BLOCK_RECORDS)
            {
                // Save current index page 
                saveIndex(newIndexPage, newIndexPageNum, fileName);

                // Create new index page
                newIndexPage = createEmptyIndex();

                newIndexPageNum++;
                newIndexRecNum = 0;
            }    
            // Add new index record index(newIndexRec) and link to data newDataPage
            addIndexEntry(newIndexPage, record.key, newDataPageNum);

            newIndexRecNum++;
            
            newDataRecNum = 0;
            // Add record to data(newDataRec)
            newDataPage.records[newDataRecNum] = record;
            //set pointer to -1
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

    for(int i = 0; i < countNumberOfMainPages(file); i++){
        page = loadPage(i, file);
        cout << "Strona " << i << endl;
        for(int j = 0; j < COEFFICIENT_OF_BLOCKING; j++){
            if(page.records[j].key !=0){
                cout << "Klucz: " << page.records[j].key << " Numer: " << page.records[j].licensePlate << " Wskaznik nadmiarowy: " << page.records[j].overflowPointer << endl;
                // Przepisz do nowej struktury
                addRecordToPage(page.records[j], tempFileName);
                if(page.records[j].overflowPointer !=-1){
                    int overflowPageIndex = page.records[j].overflowPointer/(COEFFICIENT_OF_BLOCKING);
                    int overflowRecordIndex = page.records[j].overflowPointer%(COEFFICIENT_OF_BLOCKING);
                    Page overflowPage = loadOverflowPage(overflowPageIndex,0, file);
                    Record overflowRecord = overflowPage.records[overflowRecordIndex];
                    cout << "       Klucz: " << overflowRecord.key << " Numer: " << overflowRecord.licensePlate << " Wskaznik nadmiarowy: " << overflowRecord.overflowPointer << endl;
                    // Przepisz do nowego pilu
                    addRecordToPage(overflowRecord, tempFileName);
                    if(overflowRecord.overflowPointer != -1)
                    {
                        while(overflowRecord.overflowPointer != -1){
                            overflowPageIndex = overflowRecord.overflowPointer/(COEFFICIENT_OF_BLOCKING);
                            overflowRecordIndex = overflowRecord.overflowPointer%(COEFFICIENT_OF_BLOCKING);
                            overflowPage = loadOverflowPage(overflowPageIndex,0, file);
                            overflowRecord = overflowPage.records[overflowRecordIndex];
                            cout << "       Klucz: " << overflowRecord.key << " Numer: " << overflowRecord.licensePlate << " Wskaznik nadmiarowy: " << overflowRecord.overflowPointer << endl;
                            // Przepisz do nowego pilu
                            addRecordToPage(overflowRecord, tempFileName);
                        }
                    }
                }
            }
            else{
                cout << "---Miejsce puste---" << endl;
            }   
        }
    }

    flushNewPages(tempFileName);

    //skasowanie i zmiana nazwy pliku na file




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
    //
    //Page nextOvfPage;
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
            
            //nextOvfPage = loadOverflowPage(nextOvfPageIndex,1, file); //tutaj dostaniemy sie gdy rekordy sa na roznych stronach
            Page nextOvfPage = loadOverflowPage(nextOvfPageIndex,1, file); //to poprawic jak sie da
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
                    savePage(mainPage,mainPageIndex, file);
                    saveOverflowPage(overflowPage, overflowPageIndex, file);
                    totalOverflowRecords++;
                    totalRecords++;
                    return 2;
                }
                else 
                {
                    // Element pomiedzy dwoma na liscie
                    int prevOvfPageIndex = (prevOvfRecordNumber)/(COEFFICIENT_OF_BLOCKING);
                    int prevOvfRecordIndex = (prevOvfRecordNumber)%(COEFFICIENT_OF_BLOCKING);
                    //Page prevOvfPage;
                    if (nextOvfPageIndex != prevOvfPageIndex) //tutaj dostaniemy sie gdy rekordy sa na roznych stronach
                    {
                        //prevOvfPage = loadOverflowPage(prevOvfPageIndex,1, file);
                        Page prevOvfPage = loadOverflowPage(prevOvfPageIndex,1, file);
                        Record prevOvfRecord = prevOvfPage.records[prevOvfRecordIndex]; 
                        //
                        if(prevOvfPageIndex == overflowPageIndex){
                            addedRecord.overflowPointer = nextOvfRecordNumber;
                            overflowPage.records[overflowRecordIndex] = addedRecord;
                            prevOvfRecord.overflowPointer = overflowRecordNumber;
                            overflowPage.records[prevOvfRecordIndex] = prevOvfRecord;
                        }
                        //
                        else{
                        addedRecord.overflowPointer = nextOvfRecordNumber;
                        overflowPage.records[overflowRecordIndex] = addedRecord;
                        prevOvfRecord.overflowPointer = overflowRecordNumber;   
                        prevOvfPage.records[prevOvfRecordIndex] = prevOvfRecord;  
                        //overflowPage.records[prevOvfRecordIndex] = prevOvfRecord;                   

                        saveOverflowPage(prevOvfPage, prevOvfPageIndex, file);
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
                    //mergeAndSaveOverflowPage(overflowPage, overflowPageIndex, prevOvfPage, prevOvfPageIndex);
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
    totalReads = 0;
    totalWrites = 0;

    //Dodanie pierwszego rekordu i rekordu Dummy
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
            cout << "Taki rekord juz istnieje" << endl;
            return 0;
        }
        if(i == countRecords(page)-1){ //dodanie rekordu na koniec strony
            if(i < COEFFICIENT_OF_BLOCKING-1){ //jesli jest miejsce na stronie
                page.records[i+1] = newRecord;
                savePage(page, pageIndex, file);
                totalRecords++;
                totalMainRecords++;
                cout << "dodano rekord na koniec strony " << pageIndex << endl;
                return 0;
            }
            else{ //jesli nie ma miejsca na stronie
                cout << "bedziemy dodawac rekord do strony " << pageIndex << "w miejscu nadmiarowym po rekordzie ostatnim " <<  page.records[i].key << endl;    
                addToOverflow(newRecord, page, page.records[i], pageIndex);
                //sprawdz czy ilosc rekordow overflow nie przekroczyla 50% ilosci rekordow w pliku
                if(totalOverflowRecords >= 0.5*totalMainRecords){
                    reorganise(file);
                }
                return 0;    
            }
        }
        else{ //dodanie rekordu w srodku strony
            if(newRecord.key > page.records[i].key && newRecord.key < page.records[i+1].key){
                cout << "bedziemy dodawac rekord do strony " << pageIndex << "w miejscu nadmiarowym pomiędzy rekordami " <<  page.records[i].key << " i " << page.records[i+1].key << endl;
                addToOverflow(newRecord, page, page.records[i], pageIndex);
                //sprawdz czy ilosc rekordow overflow nie przekroczyla 50% ilosci rekordow w pliku
                if(totalOverflowRecords >= 0.5*totalMainRecords){
                    reorganise(file);
                }
                return 0;
            }
        }        
    }    
    
    return 0;
}



//tworzenie plikow
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

int manageProgramInput(int choice){ //tutaj dopisac obsluge wyboru w sensei tworzenie plikow albo odpowiednie parsowanie pliku testowego 
    
    if(choice == 1){


        cout << "Podaj nazwe pliku: ";
        cin >> file;
        createFiles(file);

        //jeszcze dodac odczyt z jednego pliku spreparowanego
        //wczytaj dane z pliku testowego
        //przejsc do mozliwosci dodawania itp.

        return 1;
    }else if(choice == 2){
        cout << "Podaj nazwe pliku: ";
        cin >> file;
        createFiles(file);
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
    int searchedKey;
    
    while(choice != 8){
        mainMenu();
        cout << "Wybierz opcje: ";
        cin >> choice;
        switch(choice){
            case 1:
                //wyszukaj rekord i odczytaj
                cout << "Podaj klucz rekordu do wyszukania: ";
                cin >> searchedKey;
                findRecordByKey(searchedKey);
                cout << "ilosc odczytow: " << totalReads << " ilosc zapisow: " << totalWrites << endl;
                break;
            case 2:
                //czesc dodawania rekordu
                newRecord = getNewRecordData();
                addRecord(newRecord);
                cout << "ilosc odczytow: " << totalReads << " ilosc zapisow: " << totalWrites << endl;
                break;
            case 3:
                //usun rekord
                break;
            case 4:
                //reorganizuj
                reorganise(file);
                break;
            case 5:
                //wyswietl wszystkie rekordy
                showAllData(file);
                //rewriteAllData(file);
                break;
            case 6:
                //wyswietl indeksy plikow
                showIndexFile(file);
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



//dodac usprawnienie w dodawaniu do overflow zeby tak czesto nie musiec odczytywac tej samej strony jezeli 
//usprawnic wyswietlanie danych z rekordu ze jezeli jestesmy na tej samej stronie to nie trzeba jej odczytywac ponownie

//pisanie reorganizacji

//pododawac elementy dodawania rekordu do ilosci rekordow, zerowanei tych zmienych itp (totalRecords, totalMainRecords, totalOverflowRecords)
//po reorganizacji totalMainRecord = totalRecords

