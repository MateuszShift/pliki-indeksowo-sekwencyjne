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

    //if (blockIndex == -1) {
    //    indexFile.seekp(0, ios::end);
    //    int newBlockIndex = indexFile.tellp() / sizeof(Index);
    //    indexFile.write(reinterpret_cast<const char *>(&index), sizeof(Index));
    //    indexFile.close();
    //    totalWrites++;
    //    return newBlockIndex;
    //} else {
        indexFile.seekp(blockIndex * sizeof(Index), ios::beg);
        indexFile.write(reinterpret_cast<const char *>(&index), sizeof(Index));
        indexFile.close();
        totalWrites++;
        return blockIndex;
    //}
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

    for(int i = 0; i < countNumberOfMainPages(fileName); i++){
        page = loadPage(i, fileName);
        cout << "Strona " << i << endl;
        for(int j = 0; j < COEFFICIENT_OF_BLOCKING; j++){
            if(page.records[j].key !=0){
                cout << "Klucz: " << page.records[j].key << " Numer: " << page.records[j].licensePlate << " Wskaznik nadmiarowy: " << page.records[j].overflowPointer << endl;
                if(page.records[j].overflowPointer !=-1){
                    int overflowPageIndex = page.records[j].overflowPointer/(COEFFICIENT_OF_BLOCKING);
                    int overflowRecordIndex = page.records[j].overflowPointer%(COEFFICIENT_OF_BLOCKING);
                    Page overflowPage = loadOverflowPage(overflowPageIndex,0, fileName);
                    Record overflowRecord = overflowPage.records[overflowRecordIndex];
                    cout << "       Klucz: " << overflowRecord.key << " Numer: " << overflowRecord.licensePlate << " Wskaznik nadmiarowy: " << overflowRecord.overflowPointer << endl;
                    while(overflowRecord.overflowPointer != -1){
                        overflowPageIndex = overflowRecord.overflowPointer/(COEFFICIENT_OF_BLOCKING);
                        overflowRecordIndex = overflowRecord.overflowPointer%(COEFFICIENT_OF_BLOCKING);
                        overflowPage = loadOverflowPage(overflowPageIndex,0, fileName);
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
    saveIndex(newIndexPage, newIndexPageNum, fileName);
    savePage(newDataPage, newDataPageNum, fileName);
}

void addRecordToPage(Record &record, string &fileName){
    static int newIndexRecNum = 0;
    static int newDataRecNum = 0;

    int coefForNewPage = COEFFICIENT_OF_BLOCKING*alpha;
    
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
        if (newDataRecNum < coefForNewPage)
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
            if (newIndexRecNum >= MAX_INDEX_BLOCK_RECORDS)
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

    for(int i = 0; i < countNumberOfMainPages(fileName); i++){
        page = loadPage(i, fileName);
        //cout << "Strona " << i << endl;
        for(int j = 0; j < COEFFICIENT_OF_BLOCKING; j++){
            if(page.records[j].key !=0){
                //cout << "Klucz: " << page.records[j].key << " Numer: " << page.records[j].licensePlate << " Wskaznik nadmiarowy: " << page.records[j].overflowPointer << endl;
                // Przepisz do nowej struktury
                addRecordToPage(page.records[j], tempFileName);
                if(page.records[j].overflowPointer !=-1){
                    int overflowPageIndex = page.records[j].overflowPointer/(COEFFICIENT_OF_BLOCKING);
                    int overflowRecordIndex = page.records[j].overflowPointer%(COEFFICIENT_OF_BLOCKING);
                    Page overflowPage = loadOverflowPage(overflowPageIndex,0, file);
                    Record overflowRecord = overflowPage.records[overflowRecordIndex];
                    //cout << "       Klucz: " << overflowRecord.key << " Numer: " << overflowRecord.licensePlate << " Wskaznik nadmiarowy: " << overflowRecord.overflowPointer << endl;
                    // Przepisz do nowego pilu
                    addRecordToPage(overflowRecord, tempFileName);
                    if(overflowRecord.overflowPointer != -1)
                    {
                        while(overflowRecord.overflowPointer != -1){
                            overflowPageIndex = overflowRecord.overflowPointer/(COEFFICIENT_OF_BLOCKING);
                            overflowRecordIndex = overflowRecord.overflowPointer%(COEFFICIENT_OF_BLOCKING);
                            overflowPage = loadOverflowPage(overflowPageIndex,0, file);
                            overflowRecord = overflowPage.records[overflowRecordIndex];
                            //cout << "       Klucz: " << overflowRecord.key << " Numer: " << overflowRecord.licensePlate << " Wskaznik nadmiarowy: " << overflowRecord.overflowPointer << endl;
                            // Przepisz do nowego pilu
                            addRecordToPage(overflowRecord, tempFileName);
                        }
                    }
                }
            }
            //else{
            //    cout << "---Miejsce puste---" << endl;
            //}   
        }
    }

    flushNewPages(tempFileName);

    string tempMainFileName = tempFileName + ".dat";
    string tempOvfFileName = tempFileName + "Overflow.dat";
    string tempIndexFileName = tempFileName + "Index.dat";

    //skasowanie i zmiana nazwy pliku na file
    remove(dataFileName.c_str());
    remove(overflowFileName.c_str());
    remove(indexFileName.c_str());
    rename(tempMainFileName.c_str(), dataFileName.c_str());
    rename(tempOvfFileName.c_str(), overflowFileName.c_str());
    rename(tempIndexFileName.c_str(), indexFileName.c_str());

    //ustawienie odpowiednich ilosci rekordow
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
                cout << "TAKI REKORD JUZ ISTNIEJE W OVERFLOW!!!" << endl;
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
            cout << "TAKI REKORD JUZ ISTNIEJE !!!" << endl;
            return 0;
        }
        if(i == countRecords(page)-1){ //dodanie rekordu na koniec strony
            if(i < COEFFICIENT_OF_BLOCKING-1){ //jesli jest miejsce na stronie
                page.records[i+1] = newRecord;
                savePage(page, pageIndex, file);
                totalRecords++;
                totalMainRecords++;
                //cout << "dodano rekord na koniec strony " << pageIndex << endl;
                return 0;
            }
            else{ //jesli nie ma miejsca na stronie
                //cout << "bedziemy dodawac rekord do strony " << pageIndex << "w miejscu nadmiarowym po rekordzie ostatnim " <<  page.records[i].key << endl;    
                addToOverflow(newRecord, page, page.records[i], pageIndex);
                //sprawdz czy ilosc rekordow overflow nie przekroczyla 50% ilosci rekordow w pliku
                if(totalOverflowRecords >= 0.5*totalMainRecords){
                    reorganise(file);
                    //showAllData(file);
                }
                return 0;    
            }
        }
        else{ //dodanie rekordu w srodku strony
            if(newRecord.key > page.records[i].key && newRecord.key < page.records[i+1].key){
                //cout << "bedziemy dodawac rekord do strony " << pageIndex << "w miejscu nadmiarowym pomiędzy rekordami " <<  page.records[i].key << " i " << page.records[i+1].key << endl;
                addToOverflow(newRecord, page, page.records[i], pageIndex);
                //sprawdz czy ilosc rekordow overflow nie przekroczyla 50% ilosci rekordow w pliku
                if(totalOverflowRecords >= 0.5*totalMainRecords){
                    reorganise(file);
                    //showAllData(file);
                }
                return 0;
            }
        }        
    }    
    
    return 0;
}

void readRecords(const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "Nie można otworzyć pliku do odczytu!" << std::endl;
        return;
    }

    char operation;
    Record record;
    int recordIndex = 1;

    while (true) {
        // Odczyt znaku operacji
        if (!inFile.read(reinterpret_cast<char*>(&operation), sizeof(operation))) {
            break; // Koniec pliku
        }

        // Decyzja na podstawie znaku operacji
        if (operation == 'A') {
            // Odczyt rekordu, jeśli operacja to 'A'
            if (!inFile.read(reinterpret_cast<char*>(&record), sizeof(Record))) {
                std::cerr << "Błąd: Niekompletny rekord w pliku!" << std::endl;
                break;
            }

            addRecord(record);
        } else if (operation == 'R') {
            // Wywołanie funkcji reorganizacji, jeśli operacja to 'R'
            reorganise(file);
        }
        ++recordIndex;
    }

    inFile.close();
}


void generateRandomRecords(int amount, string &fileName) {
    srand(time(NULL));
    std::ofstream outFile(fileName, std::ios::binary);
    if (!outFile) {
        std::cerr << "Nie można otworzyć pliku do zapisu!" << std::endl;
        return;
    }

    for (int i = 0; i < amount; ++i) {
        char operation;
        Record record;

        // Ustawienie znaku operacji: 'R' co 50 rekordów, 'A' w pozostałych przypadkach
        if ((i + 1) % 50 == 0) {
            operation = 'R';
        } else {
            operation = 'A';
        }

        // Zapis znaku operacji
        outFile.write(reinterpret_cast<const char*>(&operation), sizeof(operation));

        // Jeśli operacja to 'A', zapisz również rekord
        if (operation == 'A') {
            // Losowanie klucza z zakresu 0–10000
            record.key = std::rand() % 10001; // Liczba od 0 do 10000 włącznie

            // Generowanie losowej tablicy znaków dla tablicy `licensePlate`
            // Pierwsze 3 znaki muszą być literami (od 'A' do 'Z')
            for (int j = 0; j < 3; ++j) {
                record.licensePlate[j] = 'A' + std::rand() % 26;
            }

            // Pozostałe 5 znaków może być literami lub cyframi
            for (int j = 3; j < 8; ++j) {
                int choice = std::rand() % 2; // 0 - cyfra, 1 - litera
                if (choice == 0) {
                    record.licensePlate[j] = '0' + std::rand() % 10; // Cyfra
                } else {
                    record.licensePlate[j] = 'A' + std::rand() % 26; // Litera
                }
            }
            record.licensePlate[8] = '\0'; // Dodanie znaku końca stringa

            // Ustawienie wskaźnika przepełnienia na domyślną wartość
            record.overflowPointer = -1;

            // Zapis rekordu
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

        cout << "Podaj nazwe pliku testowego (stworzy sie nowy albo zapiszemy nazwe istniejącego): ";
        cin >> generateRecordsFile;
        cout << "Podaj ilosc rekordow do wygenerowania: ";
        cin >> amount;
        //stworz dane testowe
        generateRandomRecords(amount, generateRecordsFile);

        //odczytaj dane z pliku testowego i je odpowiednio dodaj lub wykonaj reorganizacje

        readRecords(generateRecordsFile);

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

void showOverflowFile(string &fileName){
    string overflowFileName = fileName + "Overflow.dat";
    ifstream overflowFile(overflowFileName, ios::binary);
    Page page;
    for(int i = 0; i < totalOverflowRecords; i++){
        int pageIndex = i/(COEFFICIENT_OF_BLOCKING);
        int recordIndex = i%(COEFFICIENT_OF_BLOCKING);
        page = loadOverflowPageShow(pageIndex, file);
        cout << "Rekord " << i << endl;
        cout << "Klucz: " << page.records[recordIndex].key << " Numer: " << page.records[recordIndex].licensePlate << " Wskaznik nadmiarowy: " << page.records[recordIndex].overflowPointer << endl;
    }
    overflowFile.close();
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
                totalReads = 0;
                totalWrites = 0;
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
                totalReads = 0;
                totalWrites = 0;
                reorganise(file);
                cout << "ilosc odczytow: " << totalReads << " ilosc zapisow: " << totalWrites << endl;
                break;
            case 5:
                //wyswietl wszystkie rekordy
                totalReads = 0;
                totalWrites = 0;
                showAllData(file);
                //rewriteAllData(file);
                cout << "ilosc odczytow: " << totalReads << " ilosc zapisow: " << totalWrites << endl;
                cout << "ilosc rekordow: " << totalRecords << endl;
                cout << "ilosc rekordow w pliku nadmiarowym: " << totalOverflowRecords << endl;
                cout << "ilosc rekordow w pliku glownym: " << totalMainRecords << endl;
                break;
            case 6:
                //wyswietl indeksy plikow
                totalReads = 0;
                totalWrites = 0;
                showIndexFile(file);
                cout << "ilosc odczytow: " << totalReads << " ilosc zapisow: " << totalWrites << endl;
                break;
            case 7:
                //wyswietl plik nadmiarowy
                showOverflowFile(file);
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

