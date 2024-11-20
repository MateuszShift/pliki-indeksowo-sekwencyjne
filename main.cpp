#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Record {
private:
    int key;                  
    std::string licensePlate;    

public:
    //konstruktory
    Record() : key(0), licensePlate("") {}
    Record(int key, const std::string& plate)
        : key(key), licensePlate(plate){}

    //gettery
    int getKey() const { return key; }
    std::string getLicensePlate() const { return licensePlate; }

    //settery
    void setKey(int saidKey) { key = saidKey; }
    void setLicensePlate(const std::string& plate) { licensePlate = plate; }

    void printRecord() const {
        std::cout << "Key: " << key
                  << ", License Plate: " << licensePlate
                  << std::endl;
    }

    //operatory
    bool operator<(const Record& other) const {
        return this->key < other.key;
    }
    bool operator==(const Record& other) const {
        return this->key == other.key;
    }
};




int main(){

    Record r1(1, "AB123CD");
    Record r2(2, "CD123EF");

    r1.printRecord();
    r2.printRecord();

    if (r1 < r2) {
        std::cout << "r1 < r2" << std::endl;
    } else {
        std::cout << "r1 >= r2" << std::endl;
    }
    
    
    return 0;
}