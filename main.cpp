#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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

//dodawanie rekordu

//usuwanie rekordu

//reorganizacja

//wyswietlanie rekordow

//wyswietlanie indeksow

//wyswietlanie pliku nadmiarowego

//tworzenie plikow
void createFiles(){
    string dataFileName = file + ".dat";
    string overflowFileName = file + "Overflow.dat";
    string indexFileName = file + "Index.idx";

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

void manageChoice(){ //uzupelniac o wywolania funkcji
    int choice;
    
    while(choice != 8){
        mainMenu();
        cout << "Wybierz opcje: ";
        cin >> choice;
        switch(choice){
            case 1:
                //wyszukaj rekord i odczytaj
                break;
            case 2:
                //dodaj rekord
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
    choosePath(); //wybór ścieżki programu
    cout << "Wybierz ścieżkę: ";
    cin >> choicePath;
    if(manageProgramInput(choicePath) == 3){ //dodanie rzeczy z pliku lub przejscie do pustego programu tylko tworząc pliki 
        return 0;
    }
    manageChoice();

    return 0;
}