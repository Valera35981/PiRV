#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <clocale>

using namespace std;

class Student {
private:
    string imya;
    vector<double> ocenki;

public:
    Student(const string& imya) : imya(imya) {}

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

    string getImya() const {
        return imya;
    }

    void vyvodInfo() const {
        cout << "Студент: " << imya << ", средний балл: " << sredniyBall() << endl;
    }
};

class Group {
private:
    string nazvanie;             
    vector<Student*> studenti;       

public:
    Group(const string& nazv) : nazvanie(nazv) {}

    void dobavitStudenta(Student* student) {
        if (student != nullptr) {
            studenti.push_back(student);
            cout << "Студент " << student->getImya() << " добавлен в группу " << nazvanie << endl;
        }
    }

    void udalitStudenta(const string& imya) {
        for (size_t i = 0; i < studenti.size(); ++i) {
            if (studenti[i]->getImya() == imya) {
                cout << "Студент " << imya << " удалён из группы " << nazvanie << endl;
                studenti.erase(studenti.begin() + i);
                return;
            }
        }
        cout << "Студент " << imya << " не найден в группе" << endl;
    }

    double sredniyBallGruppy() const {
        if (studenti.empty()) return 0.0;

        double sum = 0.0;
        for (const auto* student : studenti) {
            sum += student->sredniyBall();
        }
        return sum / studenti.size();
    }

    void vyvodVsehStudentov() const {
        cout << "\nГруппа: " << nazvanie << endl;
        if (studenti.empty()) {
            cout << "  В группе нет студентов" << endl;
            return;
        }

        cout << "  Студенты:" << endl;
        for (const auto* student : studenti) {
            cout << "    - ";
            student->vyvodInfo();
        }
        cout << "  Средний балл группы: " << sredniyBallGruppy() << endl;
    }

    string getNazvanie() const {
        return nazvanie;
    }
};

int main() {
    setlocale(LC_ALL, "Russian");

    cout << " КЛАСС GROUP \n";

    Student s1("Иван Иванов");
    Student s2("Анна Сидорова");
    Student s3("Пётр Иванов");
    Student s4("Мария Смирнова");

    s1.dobavitOcenku(4.5);
    s1.dobavitOcenku(5.0);
    s1.dobavitOcenku(4.0);

    s2.dobavitOcenku(3.5);
    s2.dobavitOcenku(4.0);
    s2.dobavitOcenku(4.5);

    s3.dobavitOcenku(5.0);
    s3.dobavitOcenku(5.0);
    s3.dobavitOcenku(4.5);

    s4.dobavitOcenku(3.0);
    s4.dobavitOcenku(3.5);
    s4.dobavitOcenku(4.0);

    Group group("ИУ1-43Б");

    cout << "\n Добавление студентов \n";
    group.dobavitStudenta(&s1);
    group.dobavitStudenta(&s2);
    group.dobavitStudenta(&s3);

    group.vyvodVsehStudentov();
    cout << "\n Добавление ещё одного студента \n";
    group.dobavitStudenta(&s4);
    group.vyvodVsehStudentov();

    cout << "\n Удаление студента \n";
    group.udalitStudenta("Анна Сидорова");
    group.vyvodVsehStudentov();

    cout << "\n Попытка удалить несуществующего студента \n";
    group.udalitStudenta("Неизвестный");

    cout << "\n Студенты после удаления из группы \n";
    cout << "Студент Анна Сидорова (удалена из группы, но объект существует):" << endl;
    s2.vyvodInfo();
    return 0;
}
