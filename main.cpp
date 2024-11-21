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
    IndexEntry entries[ENTRIES];
    int entryCount;
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

////funkcja testowa
void readPage(const string &fileName, int pageNumber) {
    Page page;
    ifstream file(fileName, ios::binary);
    
    file.seekg(pageNumber * sizeof(Page), ios::beg);
    file.read(reinterpret_cast<char *>(&page), sizeof(Page));
    file.close();

    cout << "Liczba rekordów: " << page.recordCount << endl;
    for (int i = 0; i < page.recordCount; ++i) {
        cout << "Rekord " << i + 1 << ": Klucz = " << page.records[i].key
             << ", Tablica rejestracyjna = " << page.records[i].licensePlate << endl;
    }
    cout << "Wskaźnik przepełnienia: " << page.overFlowPointer << endl;
}


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
    std::fstream file(dataFileName, std::ios::binary);

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

int addRecord(Index &index, Record &newRecord){
    string dataFileName = file + ".dat";
    string overflowFileName = file + "Overflow.dat";
    string indexFileName = file + "Index.dat";


    if(index.entryCount == 0){ //jesli indeks jest pusty to 
        Page mainPage = createEmptyPage();
        insertNewRecord(mainPage, newRecord);
        savePage(mainPage, 0);
        addIndexEntry(index, newRecord.key, 0);
        return 0;
        //readPage(dataFileName, 0);
    }
    //krok 2
    int pageIndex = findPage(newRecord.key, index);

    //krok 3
    Page mainPage = loadPage(pageIndex);
    
    //krok 4
    if(mainPage.recordCount < COEFFICIENT_OF_BLOCKING){
        insertNewRecord(mainPage, newRecord);
        savePage(mainPage, pageIndex);
        return 0;
    }
    //krok 5 w ktorym sprawdzamy czy jest strona nadmiarowa

    
    
    return 0;
}





//usuwanie rekordu

//reorganizacja

//wyswietlanie rekordow

//wyswietlanie indeksow

//wyswietlanie pliku nadmiarowego

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

void initializeEmptyIndex(Index &index){
    index.entryCount = 0;
}

int manageProgramInput(int choice, Index index){ //tutaj dopisac obsluge wyboru w sensei tworzenie plikow albo odpowiednie parsowanie pliku testowego 
    
    if(choice == 1){
        cout << "Podaj nazwe pliku: ";
        cin >> file;
        createFiles();
        string indexFileName = file + "Index.dat";
        ifstream indexFile(indexFileName, ios::binary);
        if (indexFile.is_open()) {
            indexFile.read(reinterpret_cast<char *>(&index), sizeof(Index));
            indexFile.close();
        } else {
            initializeEmptyIndex(index); // Zainicjalizuj pusty indeks
        }

        

        //wczytaj dane z pliku testowego

        return 1;
    }else if(choice == 2){
        cout << "Podaj nazwe pliku: ";
        cin >> file;
        createFiles();
        initializeEmptyIndex(index);
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
    return newRecord;
}

void manageChoice(Index &index){ //uzupelniac o wywolania funkcji
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
                addRecord(index, newRecord);
                break;
            case 3:
                //usun rekord
                break;
            case 4:
                //reorganizuj
                break;
            case 5:
                //wyswietl wszystkie rekordy
                break;
            case 6:
                //wyswietl indeksy plikow
                break;
            case 7:
                //wyswietl plik nadmiarowy
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
    Index index; //struktura indeksu
    choosePath(); //wybór ścieżki programu
    cout << "Wybierz ścieżkę: ";
    cin >> choicePath;
    if(manageProgramInput(choicePath, index) == 3){ //dodanie rzeczy z pliku lub przejscie do pustego programu tylko tworząc pliki 
        return 0;
    }
    manageChoice(index);

    return 0;
}