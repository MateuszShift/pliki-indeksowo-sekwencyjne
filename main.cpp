#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ostream>
using namespace std;

#define COEFFICIENT_OF_BLOCKING 4
#define ENTRIES 1000

string file;



//struktury
struct Record {
    int key;          
    char licensePlate[9];
};

struct Page{
    int recordCount;
    Record records[COEFFICIENT_OF_BLOCKING];
    int overFlowPointer;
};

struct IndexEntry {
    int key;       
    int pagePointer;
};

struct Index {
    int entryCount;
    IndexEntry entries[1000];
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

//funkcje//
//dodawanie rekordu//

//4. jesli na stronie jest miejsce to dodaj rekord
    //wstawienie rekordu na odpowiednie miejsce na stronie
    //zapisanie strony w pliku
    //wyswietlenie informacji o dodaniu rekordu
//5. obsluga przepelnienia
    //jezeli overflow to -1 to znaczy ze nie ma strony nadmiarowej
        //tworzenie strony nadmiarowej pustej 
        //dodanie rekordu na stronie nadmiarowej
        //zapisanie strony nadmiarowej w pliku
        //zaktualizuj wskaznik przepelnienia
    //jezeli juz istnieje to dodaj do strony nadmiarowej
//6.zapisanie zaktualizowanej strony glownej
//7. sprawdzenie czy potrzebna jest reorganizacja
    //jesli tak to reorganizacja

//przejdzmy do pisania funkcji dodania rekordu

Index createEmptyIndex() {
    Index index;
    index.entryCount = 0;
    std::memset(index.entries, 0, sizeof(index.entries));
    return index;
}

Index loadIndex(){
    string indexFileName = file + "Index.dat";
    Index index;
    ifstream indexFile(indexFileName, ios::binary);
    
    indexFile.seekg(0, std::ios::end);
    std::streamsize fileSize = indexFile.tellg();
    indexFile.seekg(0, std::ios::beg);

    if (fileSize == 0) {
        //std::cout << "Plik indeksowy jest pusty, tworzenie pustego indeksu." << std::endl;
        index = createEmptyIndex();
    }
    indexFile.read(reinterpret_cast<char *>(&index.entryCount), sizeof(index.entryCount));
    
    indexFile.read(reinterpret_cast<char *>(index.entries), index.entryCount * sizeof(IndexEntry));
    
    

    indexFile.close();
    return index;
}

void saveIndex(Index &index) {
    string indexFileName = file + "Index.dat";
    ofstream indexFile(indexFileName, ios::binary | ios::out);
    indexFile.write(reinterpret_cast<const char *>(&index.entryCount), sizeof(index.entryCount));
    indexFile.write(reinterpret_cast<const char *>(index.entries), index.entryCount * sizeof(IndexEntry));

    indexFile.close();
}

int findPage(int key, Index &index){
    
    for (int i = 0; i < index.entryCount; ++i) {
        
        if (i == index.entryCount - 1 || key < index.entries[i + 1].key) {
            return index.entries[i].pagePointer;
        }
    }
}

void addIndexEntry(Index &index, int key, int pageIndex){
    index.entries[index.entryCount].key = key;
    index.entries[index.entryCount].pagePointer = pageIndex;
    index.entryCount++;
}

void savePage(Page &page, int pageNumber){
    
    string dataFileName = file + ".dat";
    std::fstream file(dataFileName, std::ios::binary | std::ios::in | std::ios::out); //otwarcie pliku do odczytu i zapisu w trybie binarnym

    if (pageNumber == -1) {
        file.seekp(0, std::ios::end);
    } else {
        file.seekp(pageNumber * sizeof(Page), std::ios::beg);
    }

    file.write(reinterpret_cast<const char *>(&page), sizeof(Page)); 
    
    file.close();

}

void insertNewRecord(Page &page, Record &newRecord){
    int position = page.recordCount;
    while(position > 0 && page.records[position - 1].key > newRecord.key){
        page.records[position] = page.records[position - 1];
        position--;
    }
    page.records[position] = newRecord;
    page.recordCount++;
}

Page createEmptyPage(){
    Page page;
    memset(&page,0,sizeof(Page));
    page.recordCount = 0;
    page.overFlowPointer = -1;

    return page;
}

Page loadPage(int pageNumber){
    string dataFileName = file + ".dat";
    Page page;
    ifstream file(dataFileName, ios::binary);
    file.seekg(pageNumber * sizeof(Page), ios::beg);
    file.read(reinterpret_cast<char *>(&page), sizeof(Page)); 
    file.close();
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

Page loadOverflowPage(int overflowPointer){
    string overflowFileName = file + "Overflow.dat";
    Page page;
    ifstream file(overflowFileName, ios::binary);
    file.seekg(overflowPointer * sizeof(Page), ios::beg);
    file.read(reinterpret_cast<char *>(&page), sizeof(Page)); 
    file.close();
    return page;
}

int countRecordsInOverflow(int overflowPointer){
    Page page = loadOverflowPage(overflowPointer);
    return page.recordCount;
}

bool checkIfKeyAlreadyExists(Record &newRecord){
    Index index = loadIndex();
    for (int i = 0; i < index.entryCount; ++i) {
        Page page = loadPage(index.entries[i].pagePointer);
        for(int j = 0; j < page.recordCount; ++j){
            if(page.records[j].key == newRecord.key){
                return true;
            }
        }
        int overflowPointer = page.overFlowPointer;
        while(overflowPointer != -1){
            Page overflowPage = loadOverflowPage(overflowPointer);
            for(int j = 0; j < overflowPage.recordCount; ++j){
                if(overflowPage.records[j].key == newRecord.key){
                    return true;
                }
            }
            overflowPointer = overflowPage.overFlowPointer;
        }
    }

}

int addRecord(Record &newRecord){

    if(checkIfKeyAlreadyExists(newRecord)){
        cout << "Rekord o podanym kluczu juz istnieje" << endl;
        return 1;
    }

    Index index = loadIndex();
    bool indexFileChanged = false;

    if(index.entryCount == 0){
        Page mainPage = createEmptyPage();
        insertNewRecord(mainPage, newRecord);
        savePage(mainPage, 0);
        addIndexEntry(index, newRecord.key, 0);
        indexFileChanged = true;
        if (indexFileChanged) {
            saveIndex(index);
        }
        return 0;
    }
    //krok 2
    int pageIndex = findPage(newRecord.key, index);

    //krok 3
    Page mainPage = loadPage(pageIndex);
    
    //krok 4
    if(mainPage.recordCount < COEFFICIENT_OF_BLOCKING){
        insertNewRecord(mainPage, newRecord);
        savePage(mainPage, pageIndex);
        if(newRecord.key < index.entries[pageIndex].key){
            index.entries[pageIndex].key = newRecord.key;
            indexFileChanged = true;
            if (indexFileChanged) {
                saveIndex(index);
            }
        }
        return 0;
    }
    //krok 5
    else{
        Page overflowPage;
        if(mainPage.overFlowPointer == -1){
            overflowPage = createEmptyPage();
            insertNewRecord(overflowPage, newRecord);  
            int overflowPageIndex = saveOverflowPage(overflowPage, -1);
            mainPage.overFlowPointer = overflowPageIndex;
            savePage(mainPage, pageIndex);
        }
        else{
            overflowPage = loadOverflowPage(mainPage.overFlowPointer);
            insertNewRecord(overflowPage, newRecord);
            saveOverflowPage(overflowPage, mainPage.overFlowPointer);
        }
    }
    //krok 6
    if(countRecordsInOverflow(mainPage.overFlowPointer) >= COEFFICIENT_OF_BLOCKING/2){
        //reorganize();
        //indexFileChanged = true;
        cout << "Reorganizacja" << endl;
    }
    if (indexFileChanged) {
        saveIndex(index);
    }
    return 0;
}

//usuwanie rekordu

//reorganizacja

//wyswietlanie rekordow

void showDataFile(){
    string dataFileName = file + ".dat";
    ifstream file(dataFileName, ios::binary);
    Page page;
    int i = 0;
    while(file.read(reinterpret_cast<char *>(&page), sizeof(Page))){
        cout << "Strona nr " << i << endl;
        cout << "Liczba rekordów: " << page.recordCount << endl;
        for(int j = 0; j < page.recordCount; ++j){
            cout << "Klucz: " << page.records[j].key << " Numer rejestracyjny: " << page.records[j].licensePlate << endl;
        }
        i++;
    }
    file.close();   
}

//wyswietlanie indeksow

void showIndexFile(){
    Index index = loadIndex();
    cout << "Plik indeksowy wygląda następująco:" << endl;
    for (int i = 0; i < index.entryCount; ++i) {
        cout << "Klucz: " << index.entries[i].key << " Strona: " << index.entries[i].pagePointer << endl;
    }
}
//wyswietlanie pliku nadmiarowego

void showOverflowFile(){
    string overflowFileName = file + "Overflow.dat";
    ifstream file(overflowFileName, ios::binary);
    Page page;
    int i = 0;
    while(file.read(reinterpret_cast<char *>(&page), sizeof(Page))){
        cout << "Strona nadmiarowa nr " << i << endl;
        cout << "Liczba rekordów: " << page.recordCount << endl;
        for(int j = 0; j < page.recordCount; ++j){
            cout << "Klucz: " << page.records[j].key << " Numer rejestracyjny: " << page.records[j].licensePlate << endl;
        }
        i++;
    }
    file.close();
}

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
                showDataFile();
                break;
            case 6:
                //wyswietl indeksy plikow
                showIndexFile();
                break;
            case 7:
                //wyswietl plik nadmiarowy
                showOverflowFile();
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


//na konsy zapytac sie o: czy tworzymy strukture indeksu w pamieci czy zapisujemy na dysku i z dysku odczyt (pewnie druga opcja)?
//jak obslugiwac overflow czy ma to byc podzial na strony czy caly plik wspolny dla wszystkich rekordow?
//jak usprawnic sprawdzanie czy klucz juz istnieje? powiedziec co mam i zapytac sie czy moze jakas tablica albo cos takiego?
//