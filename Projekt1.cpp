// ------------------------------
// Program: Prosty system biblioteczny
// Autor: Twój zespół + GitHub Copilot
// Opis: Pozwala zarządzać książkami, czytelnikami, wypożyczeniami i karami
// ------------------------------

#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <limits>
#include <algorithm>
#include <memory>
#include <fstream>
#include <cctype>

using namespace std;

// ------------------------------
// Funkcja aktualnaData
// Zwraca aktualną datę w formacie "DD.MM.RRRR" jako string.
// Używana do zapisywania dat wypożyczeń i kar.
// ------------------------------
string aktualnaData() {
    time_t t = time(0);
    tm* now = localtime(&t);
    stringstream ss;
    ss << setw(2) << setfill('0') << now->tm_mday << "."
       << setw(2) << setfill('0') << (now->tm_mon + 1) << "."
       << (now->tm_year + 1900);
    return ss.str();
}

// ------------------------------
// Funkcja formatujDate
// Zamienia czas typu time_t na string w formacie "DD.MM.RRRR".
// Używana do czytelnego wyświetlania dat.
// ------------------------------
string formatujDate(time_t czas) {
    tm* czasTM = localtime(&czas);
    stringstream ss;
    ss << put_time(czasTM, "%d.%m.%Y");
    return ss.str();
}

// ------------------------------
// Klasa Kara
// Reprezentuje karę nałożoną na czytelnika za przetrzymanie książki lub inną przewinę.
// Przechowuje kwotę, powód, datę nałożenia i status zapłaty.
// ------------------------------
class Kara {
private:
    double kwota;         // Kwota kary
    string powod;         // Powód nałożenia kary
    string dataNalozenia; // Data nałożenia kary
    bool czyZaplacona;    // Czy kara została zapłacona

public:
    // Konstruktor kary. Jeśli nie podano daty, ustawia dzisiejszą.
    Kara(double kwota = 0.0, string powod = "", string data = "", bool zaplacona = false)
        : kwota(kwota), powod(powod), dataNalozenia(data.empty() ? aktualnaData() : data),
          czyZaplacona(zaplacona) {}

    double getKwota() const { return kwota; }
    string getPowod() const { return powod; }
    string getData() const { return dataNalozenia; }
    bool isZaplacona() const { return czyZaplacona; }

    // Oznacza karę jako zapłaconą
    void zaplac() { czyZaplacona = true; }
    // Zmniejsza kwotę kary o podaną wartość (np. przy częściowej spłacie)
    void zmniejszKwote(double kwota) {
        if (kwota > 0 && kwota <= this->kwota) {
            this->kwota -= kwota;
            if (this->kwota < 0.01) {
                zaplac();
            }
        }
    }
};

// ------------------------------
// Klasa Ksiazka
// Reprezentuje książkę w katalogu biblioteki.
// Przechowuje tytuł, autora, numer i status wypożyczenia.
// ------------------------------
class Ksiazka {
private:
    string tytul;      // Tytuł książki
    string autor;      // Autor książki
    string numer;      // Numer książki 
    bool wypozyczona;  // Czy książka jest wypożyczona

public:
    // Konstruktor książki
    Ksiazka(string tytul = "", string autor = "", string numer = "", bool wypozyczona = false)
        : tytul(tytul), autor(autor), numer(numer), wypozyczona(wypozyczona) {}

    string getTytul() const { return tytul; }
    string getAutor() const { return autor; }
    string getNumer() const { return numer; }
    bool isWypozyczona() const { return wypozyczona; }

    void wypozycz() { wypozyczona = true; }
    void zwroc() { wypozyczona = false; }

    // Wyświetla informacje o książce
    void wyswietlInformacje() const {
        cout << "Tytuł: " << tytul << "\n"
             << "Autor: " << autor << "\n"
             << "Numer: " << numer << "\n"
             << "Status: " << (wypozyczona ? "Wypożyczona" : "Dostępna") << "\n\n";
    }
};

// ------------------------------
// Klasa Wypozyczenie
// Reprezentuje pojedyncze wypożyczenie książki przez czytelnika.
// Przechowuje tytuł książki, datę wypożyczenia, czas, status zwrotu i powiązane kary.
// ------------------------------
class Wypozyczenie {
private:
    string tytulKsiazki;         // Tytuł wypożyczonej książki
    string dataWypozyczenia;     // Data wypożyczenia
    time_t czasWypozyczenia;     // Czas wypożyczenia (do obliczania kar)
    bool zwrocona;               // Czy książka została zwrócona
    vector<Kara> kary;           // Lista kar związanych z tym wypożyczeniem

public:
    // Konstruktor wypożyczenia
    Wypozyczenie(string tytul = "", string data = "", time_t czas = time(0), bool zwrot = false)
        : tytulKsiazki(tytul), dataWypozyczenia(data.empty() ? aktualnaData() : data),
          czasWypozyczenia(czas), zwrocona(zwrot) {}

    string getTytul() const { return tytulKsiazki; }
    string getDataWypozyczenia() const { return dataWypozyczenia; }
    time_t getCzasWypozyczenia() const { return czasWypozyczenia; }
    bool isZwrocona() const { return zwrocona; }
    vector<Kara>& getKary() { return kary; }
    const vector<Kara>& getKary() const { return kary; }

    void oznaczJakoZwrocona() { zwrocona = true; }
    void dodajKare(const Kara& kara) { kary.push_back(kara); }

    // Oblicza liczbę dni spóźnienia względem dozwolonego czasu wypożyczenia
    int obliczDniSpoznienia(int maxDni) const {
        if (zwrocona) return 0;
        time_t teraz = time(0);
        double sekundy = difftime(teraz, czasWypozyczenia);
        int dni = static_cast<int>(sekundy / (60 * 60 * 24)) - maxDni;
        return max(0, dni);
    }

    // Oblicza wysokość kary za przetrzymanie książki powyżej 14 dni
    double obliczKareZaPrzetrzymanie() const {
        int dni = obliczDniSpoznienia(14);
        return dni * 1.0; // 1 zł za dzień powyżej 14 dni
    }

    // Wyświetla informacje o wypożyczeniu i ewentualnych karach
    void wyswietlInformacje() const {
        cout << "Książka: " << tytulKsiazki << "\n"
             << "Data wypożyczenia: " << dataWypozyczenia << "\n"
             << "Status: " << (zwrocona ? "Zwrócona" : "Wypożyczona") << "\n";
        if (!zwrocona) {
            int dniSpoznienia = obliczDniSpoznienia(14);
            if (dniSpoznienia > 0) {
                cout << "Dni spóźnienia: " << dniSpoznienia << "\n";
                cout << "Kara za przetrzymanie: " << fixed << setprecision(2) << obliczKareZaPrzetrzymanie() << " zł\n";
            }
        }
        if (!kary.empty()) {
            cout << "Kary:\n";
            for (const auto& kara : kary) {
                cout << "- " << kara.getPowod() << ": "
                     << fixed << setprecision(2) << kara.getKwota() << " zł ("
                     << (kara.isZaplacona() ? "zapłacona" : "do zapłaty") << ")\n";
            }
        }
        cout << "\n";
    }
};

// ------------------------------
// Klasa Uzytkownik
// Klasa bazowa dla wszystkich użytkowników systemu (czytelnik, bibliotekarz).
// Przechowuje login, hasło i rolę użytkownika.
// ------------------------------
class Uzytkownik {
protected:
    string login; // Login użytkownika (email)
    string haslo; // Hasło użytkownika
    string rola;  // Rola użytkownika ("czytelnik" lub "bibliotekarz")

public:
    Uzytkownik(string login = "", string haslo = "", string rola = "")
        : login(login), haslo(haslo), rola(rola) {}

    virtual ~Uzytkownik() = default;

    string getLogin() const { return login; }
    string getRola() const { return rola; }
    string getHaslo() const { return haslo; }

    bool sprawdzHaslo(const string& haslo) const {
        return this->haslo == haslo;
    }

    // Wirtualne menu użytkownika (do nadpisania w klasach pochodnych)
    virtual void wyswietlMenu(vector<Ksiazka>&) {}
    virtual void wyswietlMenu(vector<Ksiazka>&, vector<shared_ptr<Uzytkownik>>&) {}
};

// ------------------------------
// Klasa Czytelnik
// Dziedziczy po Uzytkownik. Reprezentuje czytelnika biblioteki.
// Przechowuje dane osobowe, historię wypożyczeń i saldo kar.
// ------------------------------
class Czytelnik : public Uzytkownik {
private:
    string imie;                       // Imię czytelnika
    string nazwisko;                   // Nazwisko czytelnika
    string email;                      // Email czytelnika
    string telefon;                    // Telefon czytelnika
    vector<Wypozyczenie> wypozyczenia; // Lista wypożyczeń
    double saldoKar;                   // Suma niezapłaconych kar

public:
    Czytelnik(string imie = "", string nazwisko = "", string email = "", string telefon = "",
              string login = "", string haslo = "", double saldo = 0.0)
        : Uzytkownik(login, haslo, "czytelnik"), imie(imie), nazwisko(nazwisko),
          email(email), telefon(telefon), saldoKar(saldo) {}

    string getImie() const { return imie; }
    string getNazwisko() const { return nazwisko; }
    string getEmail() const { return email; }
    string getTelefon() const { return telefon; }
    double getSaldoKar() const { return saldoKar; }
    void setSaldoKar(double saldo) { saldoKar = saldo; }
    vector<Wypozyczenie>& getWypozyczenia() { return wypozyczenia; }
    const vector<Wypozyczenie>& getWypozyczenia() const { return wypozyczenia; }

    void dodajWypozyczenie(const Wypozyczenie& wypozyczenie) {
        wypozyczenia.push_back(wypozyczenie);
    }

    // Wyświetla historię wypożyczeń czytelnika
    void wyswietlWypozyczenia() const {
        if (wypozyczenia.empty()) {
            cout << "Brak historii wypożyczeń.\n";
            return;
        }
        cout << "\n=== HISTORIA WYPOSZCZEŃ ===\n";
        for (const auto& wypozyczenie : wypozyczenia) {
            wypozyczenie.wyswietlInformacje();
        }
    }

    // Wyświetla listę kar czytelnika
    void wyswietlKary() const {
        cout << "\n=== LISTA KAR ===\n";
        cout << "Suma kar: " << fixed << setprecision(2) << saldoKar << " zł\n\n";
        if (saldoKar <= 0) {
            cout << "Brak zaległych kar.\n";
            return;
        }
        for (const auto& wypozyczenie : wypozyczenia) {
            for (const auto& kara : wypozyczenie.getKary()) {
                if (!kara.isZaplacona()) {
                    cout << "- Książka: " << wypozyczenie.getTytul() << "\n";
                    cout << "  Powód: " << kara.getPowod() << "\n";
                    cout << "  Kwota: " << fixed << setprecision(2) << kara.getKwota() << " zł\n";
                    cout << "  Data nałożenia: " << kara.getData() << "\n";
                    cout << "  Status: " << (kara.isZaplacona() ? "Zapłacona" : "Do zapłaty") << "\n\n";
                }
            }
        }
    }

    // Pozwala zapłacić karę (lub jej część)
    void zaplacKare(double kwota) {
        if (kwota <= 0 || kwota > saldoKar) {
            cout << "Nieprawidłowa kwota.\n";
            return;
        }
        double pozostalaKwota = kwota;
        for (auto& wypozyczenie : wypozyczenia) {
            for (auto& kara : wypozyczenie.getKary()) {
                if (!kara.isZaplacona() && pozostalaKwota > 0) {
                    double doZaplaty = min(kara.getKwota(), pozostalaKwota);
                    kara.zmniejszKwote(doZaplaty);
                    pozostalaKwota -= doZaplaty;
                    saldoKar -= doZaplaty;
                    if (kara.getKwota() <= 0.01) {
                        kara.zaplac();
                    }
                }
            }
        }
        cout << "Zapłacono " << fixed << setprecision(2) << kwota << " zł. Pozostałe saldo kar: " << saldoKar << " zł.\n";
    }

    // Pozwala wypożyczyć książkę z katalogu
    void wypozyczKsiazke(vector<Ksiazka>& katalog) {
        cout << "\n=== WYPOŻYCZ KSIĄŻKĘ ===\n";
        bool cosDostepne = false;
        for (const auto& ksiazka : katalog) {
            if (!ksiazka.isWypozyczona()) {
                cout << "- " << ksiazka.getTytul() << " (Autor: " << ksiazka.getAutor() << ")\n";
                cosDostepne = true;
            }
        }
        if (!cosDostepne) {
            cout << "Brak dostępnych książek do wypożyczenia.\n";
            return;
        }
        cout << "Podaj tytuł książki do wypożyczenia: ";
        string tytul;
        getline(cin >> ws, tytul);
        if (tytul.empty()) {
            cout << "Tytuł nie może być pusty!\n";
            return;
        }

        for (auto& ksiazka : katalog) {
            if (ksiazka.getTytul() == tytul && !ksiazka.isWypozyczona()) {
                ksiazka.wypozycz();
                Wypozyczenie noweWyp(ksiazka.getTytul());
                dodajWypozyczenie(noweWyp);
                cout << "Wypożyczono książkę: " << ksiazka.getTytul() << "\n";
                return;
            }
        }
        cout << "Nie znaleziono dostępnej książki o podanym tytule.\n";
    }

    // Pozwala zwrócić wypożyczoną książkę i nalicza ewentualne kary
    void zwrocKsiazke(vector<Ksiazka>& katalog) {

        cout << "\n=== ZWRÓĆ KSIĄŻKĘ ===\n";
        bool cosWypozyczone = false;
        for (const auto& wyp : wypozyczenia) {
            if (!wyp.isZwrocona()) {
                cout << "- " << wyp.getTytul() << " (wypożyczona: " << wyp.getDataWypozyczenia() << ")\n";
                cosWypozyczone = true;
            }
        }
        if (!cosWypozyczone) {
            cout << "Nie masz wypożyczonych książek.\n";
            return;
        }
        cout << "Podaj tytuł książki do zwrotu: ";
        string tytul;

        getline(cin >> ws, tytul);
        if (tytul.empty()) {
            cout << "Tytuł nie może być pusty!\n";
            return;
        }
        for (auto& wyp : wypozyczenia) {

            if (!wyp.isZwrocona() && wyp.getTytul() == tytul) {
                int dniSpoznienia = wyp.obliczDniSpoznienia(14);
                if (dniSpoznienia > 0) {
                    double kara = wyp.obliczKareZaPrzetrzymanie();
                    Kara nowaKara(kara, "Przetrzymanie powyżej 14 dni");
                    wyp.dodajKare(nowaKara);
                    saldoKar += kara;
                    cout << "Naliczono karę za przetrzymanie: " << kara << " zł\n";
                }
                int dniOdWypozyczenia = (int)((time(0) - wyp.getCzasWypozyczenia()) / (60 * 60 * 24));
                if (dniOdWypozyczenia > 30) {
                    int dniPonadMiesiac = dniOdWypozyczenia - 30;
                    double karaMiesiac = dniPonadMiesiac * 1.0;
                    Kara nowaKaraMiesiac(karaMiesiac, "Przetrzymanie powyżej miesiąca");
                    wyp.dodajKare(nowaKaraMiesiac);
                    saldoKar += karaMiesiac;
                    cout << "Naliczono dodatkową karę " << karaMiesiac << " zł za przetrzymanie powyżej miesiąca!\n";
                }
                wyp.oznaczJakoZwrocona();
                for (auto& ksiazka : katalog) {
                    if (ksiazka.getTytul() == tytul) {
                        ksiazka.zwroc();
                        break;
                    }
                }
                cout << "Książka została zwrócona.\n";
                return;
            }
        }
        cout << "Nie znaleziono wypożyczonej książki o podanym tytule.\n";
    }

    // Menu czytelnika - pozwala wybrać operacje do wykonania
    void wyswietlMenu(vector<Ksiazka>& katalog) override {
        int wybor = -1;
        do {
            cout << "\n=== MENU CZYTELNIKA (" << imie << " " << nazwisko << ") ===\n"
                 << "1. Moje wypożyczenia\n"
                 << "2. Moje kary\n"
                 << "3. Zapłać karę\n"
                 << "4. Wypożycz książkę\n"
                 << "5. Zwróć książkę\n"
                 << "0. Wyloguj\n"
                 << "Wybor: ";
            string wyborStr;
            getline(cin, wyborStr);
            try {
                wybor = stoi(wyborStr);
            } catch (...) {
                cout << "Podaj liczbę!\n";
                continue;
            }
            switch (wybor) {
                case 1: wyswietlWypozyczenia(); break;
                case 2: wyswietlKary(); break;
                case 3: {
                    if (saldoKar > 0) {
                        cout << "Podaj kwotę do zapłaty (max " << saldoKar << " zł): ";
                        string kwotaStr;
                        getline(cin, kwotaStr);
                        double kwota = 0.0;
                        try {
                            kwota = stod(kwotaStr);
                        } catch (...) {
                            cout << "Podaj poprawną liczbę!\n";
                            break;
                        }
                        zaplacKare(kwota);
                    } else {
                        cout << "Nie masz żadnych kar do zapłaty.\n";
                    }
                    break;
                }
                case 4:
                    wypozyczKsiazke(katalog);
                    break;
                case 5:
                    zwrocKsiazke(katalog);
                    break;
                case 0: cout << "Wylogowano.\n"; break;
                default: cout << "Nieprawidłowy wybór.\n";
            }
        } while (wybor != 0);
    }
    void wyswietlMenu(vector<Ksiazka>&, vector<shared_ptr<Uzytkownik>>&) override {}
};

// ------------------------------
// Klasa Bibliotekarz
// Dziedziczy po Uzytkownik. Reprezentuje bibliotekarza.
// Pozwala zarządzać katalogiem, czytelnikami i karami.
// ------------------------------
class Bibliotekarz : public Uzytkownik {
public:

    Bibliotekarz(string login = "", string haslo = "")
        : Uzytkownik(login, haslo, "bibliotekarz") {}

    // Wyświetla wszystkie książki w katalogu
    void wyswietlKsiazki(const vector<Ksiazka>& katalog) const {
        if (katalog.empty()) {
            cout << "Katalog jest pusty.\n";
            return;
        }
        cout << "\n=== KATALOG KSIĄŻEK ===\n";
        for (const auto& ksiazka : katalog) {
            ksiazka.wyswietlInformacje();
        }
    }

    // Dodaje nową książkę do katalogu
    // Numer książki jest nadawany automatycznie jako największy istniejący numer + 1
    void dodajKsiazke(vector<Ksiazka>& katalog) const {
        string tytul, autor;
        cout << "Dodawanie nowej książki:\n";
        cout << "Tytuł: ";
        getline(cin >> ws, tytul);
        if (tytul.empty()) {
            cout << "Tytuł nie może być pusty!\n";
            return;
        }

        cout << "Autor: ";
        getline(cin, autor);
        if (autor.empty()) {
            cout << "Autor nie może być pusty!\n";
            return;
        }

        // Szukamy największego numeru w katalogu (zamieniamy na liczbę)
        long long maxNumer = 0;
        for (const auto& ksiazka : katalog) {
            try {
                long long n = stoll(ksiazka.getNumer());
                if (n > maxNumer) maxNumer = n;
            } catch (...) {
                // Jeśli numer nie jest liczbą, pomijamy
            }
        }

        // Nowy numer to największy znaleziony + 1 (jako string)
        string nowyNumer = to_string(maxNumer + 1);

        // Dodajemy książkę z automatycznie nadanym numerem
        katalog.emplace_back(tytul, autor, nowyNumer);
        cout << "Książka została dodana do katalogu. Numer: " << nowyNumer << endl;
    }

    // Pozwala wyszukać książki po tytule, autorze lub numerze
    void szukajKsiazki(const vector<Ksiazka>& katalog) const {
        string fraza;
        cout << "Wpisz frazę do wyszukania (tytuł/autor/numer): ";
        getline(cin >> ws, fraza);
        if (fraza.empty()) {
            cout << "Fraza nie może być pusta!\n";
            return;
        }

        bool znaleziono = false;
        for (const auto& ksiazka : katalog) {
            if (ksiazka.getTytul().find(fraza) != string::npos ||
                ksiazka.getAutor().find(fraza) != string::npos ||
                ksiazka.getNumer().find(fraza) != string::npos) {
                ksiazka.wyswietlInformacje();
                znaleziono = true;
            }
        }

        if (!znaleziono) {
            cout << "Nie znaleziono książek pasujących do podanej frazy.\n";
        }
    }

    // Rejestruje nowego czytelnika w systemie
    void zarejestrujCzytelnika(vector<shared_ptr<Uzytkownik>>& uzytkownicy) const {
        string imie, nazwisko, email, telefon, login, haslo;
        cout << "Rejestracja nowego czytelnika:\n";
        cout << "Imię: ";
        getline(cin >> ws, imie);
        if (imie.empty()) {
            cout << "Imię nie może być puste!\n";
            return;
        }
        cout << "Nazwisko: ";
        getline(cin, nazwisko);
        if (nazwisko.empty()) {
            cout << "Nazwisko nie może być puste!\n";
            return;
        }
        cout << "Email: ";
        getline(cin, email);
        if (email.empty() || email.find('@') == string::npos) {
            cout << "Podaj poprawny email!\n";
            return;
        }
        cout << "Telefon: ";
        getline(cin, telefon);
        if (telefon.empty() || telefon.find_first_not_of("0123456789") != string::npos) {
            cout << "Podaj poprawny numer telefonu (same cyfry)!\n";
            return;
        }
        login = email;
        cout << "Hasło: ";
        getline(cin, haslo);
        if (haslo.empty()) {
            cout << "Hasło nie może być puste!\n";
            return;
        }
        auto nowyCzytelnik = make_shared<Czytelnik>(imie, nazwisko, email, telefon, login, haslo);
        uzytkownicy.push_back(nowyCzytelnik);
        cout << "Czytelnik został zarejestrowany.\n";
    }

    // Wyświetla listę wszystkich czytelników
    void listaCzytelnikow(const vector<shared_ptr<Uzytkownik>>& uzytkownicy) const {
        cout << "\n=== LISTA CZYTELNIKÓW ===\n";
        for (const auto& uzytkownik : uzytkownicy) {
            if (auto czytelnik = dynamic_cast<Czytelnik*>(uzytkownik.get())) {
                cout << czytelnik->getImie() << " " << czytelnik->getNazwisko() << "\n"
                     << "Email: " << czytelnik->getEmail() << "\n"
                     << "Telefon: " << czytelnik->getTelefon() << "\n"
                     << "Saldo kar: " << fixed << setprecision(2) << czytelnik->getSaldoKar() << " zł\n\n";
            }
        }
    }

    // Pozwala zarządzać karami wybranego czytelnika
    void zarzadzajKaramiCzytelnika(vector<shared_ptr<Uzytkownik>>& uzytkownicy) {
        string email;
        cout << "Podaj email czytelnika: ";
        getline(cin >> ws, email);
        for (auto& uzytkownik : uzytkownicy) {
            if (auto czytelnik = dynamic_cast<Czytelnik*>(uzytkownik.get())) {
                if (czytelnik->getEmail() == email) {
                    cout << "\n=== ZARZĄDZANIE KARAMI ===\n"
                         << "Czytelnik: " << czytelnik->getImie() << " " << czytelnik->getNazwisko() << "\n"
                         << "Aktualne saldo kar: " << czytelnik->getSaldoKar() << " zł\n\n";
                    int wybor = -1;
                    do {
                        cout << "1. Dodaj karę\n"
                             << "2. Zobacz kary\n"
                             << "0. Powrót\n"
                             << "Wybor: ";
                        string wyborStr;
                        getline(cin, wyborStr);
                        try {
                            wybor = stoi(wyborStr);
                        } catch (...) {
                            cout << "Podaj liczbę!\n";
                            continue;
                        }
                        switch (wybor) {
                            case 1: {
                                cout << "Kwota kary: ";
                                string kwotaStr;
                                getline(cin, kwotaStr);
                                double kwota = 0.0;
                                try {
                                    kwota = stod(kwotaStr);
                                } catch (...) {
                                    cout << "Podaj poprawną liczbę!\n";
                                    break;
                                }
                                if (kwota <= 0) {
                                    cout << "Kwota musi być dodatnia!\n";
                                    break;
                                }
                                string powod;
                                cout << "Powód: ";
                                getline(cin, powod);
                                if (powod.empty()) {
                                    cout << "Powód nie może być pusty!\n";
                                    break;
                                }
                                bool znaleziono = false;
                                for (auto& wypozyczenie : czytelnik->getWypozyczenia()) {
                                    if (!wypozyczenie.isZwrocona()) {
                                        wypozyczenie.dodajKare(Kara(kwota, powod));
                                        czytelnik->setSaldoKar(czytelnik->getSaldoKar() + kwota);
                                        znaleziono = true;
                                        break;
                                    }
                                }
                                if (!znaleziono) {
                                    Wypozyczenie noweWypozyczenie("Kara administracyjna");
                                    noweWypozyczenie.dodajKare(Kara(kwota, powod));
                                    czytelnik->dodajWypozyczenie(noweWypozyczenie);
                                    czytelnik->setSaldoKar(czytelnik->getSaldoKar() + kwota);
                                }
                                cout << "Dodano karę.\n";
                                break;
                            }
                            case 2:
                                czytelnik->wyswietlKary();
                                break;
                            case 0:
                                break;
                            default:
                                cout << "Nieprawidłowy wybór.\n";
                        }
                    } while (wybor != 0);
                    return;
                }
            }
        }
        cout << "Nie znaleziono czytelnika o podanym emailu.\n";
    }

    // Menu bibliotekarza - pozwala wybrać operacje do wykonania
    void wyswietlMenu(vector<Ksiazka>& katalog, vector<shared_ptr<Uzytkownik>>& uzytkownicy) override {
        int wybor = -1;
        do {
            cout << "\n=== MENU BIBLIOTEKARZA ===\n"
                 << "1. Przeglądaj katalog\n"
                 << "2. Dodaj książkę\n"
                 << "3. Szukaj książki\n"
                 << "4. Zarejestruj czytelnika\n"
                 << "5. Lista czytelników\n"
                 << "6. Zarządzaj karami czytelnika\n"
                 << "0. Wyloguj\n"
                 << "Wybor: ";
            string wyborStr;
            getline(cin, wyborStr);
            try {
                wybor = stoi(wyborStr);
            } catch (...) {
                cout << "Podaj liczbę!\n";
                continue;
            }
            switch (wybor) {
                case 1: wyswietlKsiazki(katalog); break;
                case 2: dodajKsiazke(katalog); break;
                case 3: szukajKsiazki(katalog); break;
                case 4: zarejestrujCzytelnika(uzytkownicy); break;
                case 5: listaCzytelnikow(uzytkownicy); break;
                case 6: zarzadzajKaramiCzytelnika(uzytkownicy); break;
                case 0: cout << "Wylogowano.\n"; break;
                default: cout << "Nieprawidłowy wybór.\n";
            }
        } while (wybor != 0);
    }
    void wyswietlMenu(vector<Ksiazka>&) override {}
};

// ------------------------------
// Klasa SystemBiblioteczny
// Główna klasa zarządzająca całą aplikacją biblioteczną.
// Przechowuje katalog książek, użytkowników i obsługuje logowanie oraz zapis/odczyt danych.
// ------------------------------
class SystemBiblioteczny {
private:
    vector<Ksiazka> katalog;                          // Katalog książek
    vector<shared_ptr<Uzytkownik>> uzytkownicy;       // Lista użytkowników
    shared_ptr<Uzytkownik> aktualnyUzytkownik;        // Aktualnie zalogowany użytkownik

public:
    // Konstruktor - wczytuje dane z pliku lub tworzy przykładowe dane
    SystemBiblioteczny() {
        wczytajDane();
        naliczKaryZaPrzetrzymanie();
    }

    // Destruktor - zapisuje dane do pliku przy zamknięciu programu
    ~SystemBiblioteczny() {
        zapiszDane();
    }

    // Główna pętla programu - logowanie i obsługa menu użytkownika
    void uruchom() {
        while (true) {
            cout << "\n=== SYSTEM BIBLIOTECZNY ===\n";
            if (!logowanie()) continue;

            if (aktualnyUzytkownik->getRola() == "bibliotekarz") {
                dynamic_cast<Bibliotekarz*>(aktualnyUzytkownik.get())->wyswietlMenu(katalog, uzytkownicy);
            } else {
                dynamic_cast<Czytelnik*>(aktualnyUzytkownik.get())->wyswietlMenu(katalog);
            }

            aktualnyUzytkownik = nullptr;
            cout << "Czy chcesz się zalogować ponownie? (t/n): ";
            char odp;
            cin >> odp;
            cin.ignore();
            if (tolower(odp) != 't') break;
        }
        cout << "Do widzenia!\n";
    }

private:
    // Zapisuje wszystkie dane do pliku tekstowego
    void zapiszDane() {
        ofstream plik("biblioteka.txt");
        // Katalog
        plik << "KSIAZKI\n";
        for (const auto& k : katalog) {
            plik << k.getTytul() << ";" << k.getAutor() << ";" << k.getNumer() << ";" << k.isWypozyczona() << "\n";
        }
        // Użytkownicy
        plik << "CZYTELNICY\n";
        for (const auto& u : uzytkownicy) {
            if (u->getRola() == "czytelnik") {
                auto c = dynamic_cast<Czytelnik*>(u.get());
                plik << c->getImie() << ";" << c->getNazwisko() << ";" << c->getEmail() << ";" << c->getTelefon()
                     << ";" << c->getLogin() << ";" << c->getSaldoKar() << ";" << c->getHaslo() << "\n";
                // Wypożyczenia
                for (const auto& w : c->getWypozyczenia()) {
                    plik << "W:" << w.getTytul() << ";" << w.getDataWypozyczenia() << ";" << w.isZwrocona() << ";" << w.getCzasWypozyczenia() << "\n";
                    for (const auto& kara : w.getKary()) {
                        plik << "K:" << kara.getKwota() << ";" << kara.getPowod() << ";" << kara.getData() << ";" << kara.isZaplacona() << "\n";
                    }
                }
            }
            if (u->getRola() == "bibliotekarz") {
                plik << "BIB;" << u->getLogin() << ";" << u->getHaslo() << "\n";
            }
        }
        plik.close();
    }

    // Wczytuje dane z pliku tekstowego lub tworzy przykładowe dane
    void wczytajDane() {
        ifstream plik("biblioteka.txt");
        if (!plik) {
            inicjalizujDane();
            return;
        }
        katalog.clear();
        uzytkownicy.clear();
        string linia, sekcja;
        shared_ptr<Czytelnik> ostatniCzytelnik = nullptr;
        Wypozyczenie* ostatnieWyp = nullptr;
        while (getline(plik, linia)) {
            if (linia == "KSIAZKI" || linia == "CZYTELNICY") {
                sekcja = linia;
                continue;
            }
            if (sekcja == "KSIAZKI") {
                stringstream ss(linia);
                string tytul, autor, numer, wyp;
                getline(ss, tytul, ';');
                getline(ss, autor, ';');
                getline(ss, numer, ';');
                getline(ss, wyp, ';');
                katalog.emplace_back(tytul, autor, numer, wyp == "1");
            } else if (sekcja == "CZYTELNICY") {
                if (linia.rfind("BIB;", 0) == 0) {
                    stringstream ss(linia.substr(4));
                    string login, haslo;
                    getline(ss, login, ';');
                    getline(ss, haslo, ';');
                    uzytkownicy.push_back(make_shared<Bibliotekarz>(login, haslo));
                } else if (linia.rfind("W:", 0) == 0) {
                    stringstream ss(linia.substr(2));
                    string tytul, data, zwrot, czas;
                    getline(ss, tytul, ';');
                    getline(ss, data, ';');
                    getline(ss, zwrot, ';');
                    getline(ss, czas, ';');
                    Wypozyczenie w(tytul, data, stol(czas), zwrot == "1");
                    ostatniCzytelnik->dodajWypozyczenie(w);
                    ostatnieWyp = &ostatniCzytelnik->getWypozyczenia().back();
                } else if (linia.rfind("K:", 0) == 0 && ostatnieWyp) {
                    stringstream ss(linia.substr(2));
                    string kwota, powod, data, zapl;
                    getline(ss, kwota, ';');
                    getline(ss, powod, ';');
                    getline(ss, data, ';');
                    getline(ss, zapl, ';');
                    ostatnieWyp->dodajKare(Kara(stod(kwota), powod, data, zapl == "1"));
                } else {
                    stringstream ss(linia);
                    string imie, nazwisko, email, telefon, login, saldo, haslo;
                    getline(ss, imie, ';');
                    getline(ss, nazwisko, ';');
                    getline(ss, email, ';');
                    getline(ss, telefon, ';');
                    getline(ss, login, ';');
                    getline(ss, saldo, ';');
                    getline(ss, haslo, ';');
                    ostatniCzytelnik = make_shared<Czytelnik>(imie, nazwisko, email, telefon, login, haslo, stod(saldo));
                    uzytkownicy.push_back(ostatniCzytelnik);
                    ostatnieWyp = nullptr;
                }
            }
        }
        plik.close();
    }

    // Automatycznie nalicza kary za przetrzymanie książek po wczytaniu danych
    void naliczKaryZaPrzetrzymanie() {
        for (const auto& u : uzytkownicy) {
            if (u->getRola() == "czytelnik") {
                auto c = dynamic_cast<Czytelnik*>(u.get());
                for (auto& wyp : c->getWypozyczenia()) {
                    if (!wyp.isZwrocona()) {
                        int dniSpoznienia = wyp.obliczDniSpoznienia(14);
                        if (dniSpoznienia > 0) {
                            double kara = wyp.obliczKareZaPrzetrzymanie();
                            // Sprawdź, czy kara już nie została naliczona
                            bool juzNaliczona = false;
                            for (const auto& k : wyp.getKary()) {
                                if (k.getPowod() == "Przetrzymanie powyżej 14 dni" && !k.isZaplacona()) {
                                    juzNaliczona = true;
                                    break;
                                }
                            }
                            if (!juzNaliczona) {
                                wyp.dodajKare(Kara(kara, "Przetrzymanie powyżej 14 dni"));
                                c->setSaldoKar(c->getSaldoKar() + kara);
                            }
                        }
                    }
                }
            }
        }
    }

    // Pomocnicza funkcja do konwersji string -> time_t
    static time_t stol(const string& s) {
        try { return static_cast<time_t>(stoll(s)); } catch (...) { return time(0); }
    }
    // Pomocnicza funkcja do konwersji string -> double
    static double stod(const string& s) {
        try { return std::stod(s); } catch (...) { return 0.0; }
    }


    // Tworzy przykładowe dane (użytkownicy i książki) jeśli nie ma pliku
    void inicjalizujDane() {
        // Dodaj przykładowych bibliotekarzy
        uzytkownicy.push_back(make_shared<Bibliotekarz>("admin@bib.pl", "admin"));


        // Dodaj przykładowych czytelników
        auto jan = make_shared<Czytelnik>("Jan", "Kowalski", "jan@czytelnik.pl", "123456789", "jan@czytelnik.pl", "1234");
        uzytkownicy.push_back(jan);


        // Dodaj przykładowe książki
        katalog.emplace_back("W pustyni i w puszczy", "Henryk Sienkiewicz", "1234567890");
        katalog.emplace_back("Lalka", "Bolesław Prus", "2345678901");
        katalog.emplace_back("Pan Tadeusz", "Adam Mickiewicz", "3456789012");
        katalog.emplace_back("Zbrodnia i kara", "Fiodor Dostojewski", "4567890123");
        katalog.emplace_back("Władca Pierścieni", "J.R.R. Tolkien", "5678901234");
        katalog.emplace_back("Hobbit", "J.R.R. Tolkien", "6789012345");
        katalog.emplace_back("Mały Książę", "Antoine de Saint-Exupéry", "7890123456");
        katalog.emplace_back("Duma i uprzedzenie", "Jane Austen", "8901234567");
        katalog.emplace_back("Mistrz i Małgorzata", "Michaił Bułhakow", "9012345678");
        katalog.emplace_back("1984", "George Orwell", "0123456789");
    }


    // Obsługuje logowanie użytkownika do systemu
    bool logowanie() {
        string login, haslo;
        cout << "Login (email): ";
        getline(cin >> ws, login);
        cout << "Hasło: ";
        getline(cin, haslo);

        for (const auto& u : uzytkownicy) {
            if (u->getLogin() == login && u->sprawdzHaslo(haslo)) {
                aktualnyUzytkownik = u;
                cout << "Zalogowano jako: " << u->getLogin() << " (" << u->getRola() << ")\n";
                return true;
            }
        }
        cout << "Błędne dane logowania.\n";
        return false;
    }
};
// ------------------------------
// Funkcja main
// Punkt wejścia do programu. Tworzy system biblioteczny i uruchamia główną pętlę.
// ------------------------------
int main() {
    SystemBiblioteczny system;
    system.uruchom();
    return 0;
}