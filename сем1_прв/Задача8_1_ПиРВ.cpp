#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <clocale>

using namespace std;
class RecordBook {
private:
    string nomerZachetki;      
    vector<double> ocenki;      

public:
    RecordBook(const string& nomer) : nomerZachetki(nomer) {}

    void dobavitOcenku(double ocenka) {
        if (ocenka >= 0.0 && ocenka <= 5.0) {
            ocenki.push_back(ocenka);
        }
    }

    double sredniyBall() const {
        if (ocenki.empty()) return 0.0;

        double sum = 0.0;
        for (double o : ocenki) {
            sum += o;
        }
        return sum / ocenki.size();
    }

    void vyvodOcenok() const {
        cout << "Зачётка #" << nomerZachetki << ": ";
        for (double o : ocenki) {
            cout << o << " ";
        }
        cout << endl;
    }
};

class Student {
private:
    string imya;
    RecordBook zachetka;        

public:
    Student(const string& imya, const string& nomerZachetki)
        : imya(imya), zachetka(nomerZachetki) {}

    void dobavitOcenku(double ocenka) {
        zachetka.dobavitOcenku(ocenka);
    }

    double sredniyBall() const {
        return zachetka.sredniyBall();
    }

    void vyvodInfo() const {
        cout << "\nСтудент: " << imya << endl;
        cout << "Средний балл: " << fixed << setprecision(2) << sredniyBall() << endl;
        zachetka.vyvodOcenok();
    }
};

int main() {
    setlocale(LC_ALL, "Russian");

    cout << " КОМПОЗИЦИЯ STUDENT + RECORDBOOK \n";
    Student s1("Иван Петров", "12345");
    Student s2("Анна Сидорова", "67890");

    s1.dobavitOcenku(4.5);
    s1.dobavitOcenku(5.0);
    s1.dobavitOcenku(3.5);

    s2.dobavitOcenku(4.0);
    s2.dobavitOcenku(4.5);
    s2.dobavitOcenku(5.0);

    s1.vyvodInfo();
    s2.vyvodInfo();

    return 0;
}
