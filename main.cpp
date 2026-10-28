#include <iostream>
#include <map>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <string>
using namespace std;
using namespace std::chrono;

// ---------- ABR écrit à partir de zéro ----------
class ABR {
private:
    struct Noeud {
        int valeur;
        Noeud* gauche;
        Noeud* droite;
        Noeud(int v) : valeur(v), gauche(nullptr), droite(nullptr) {}
    };

    Noeud* racine;
    int taille;

    // Insertion récursive
    Noeud* inserer(Noeud* n, int x, bool& ajoute) {
        if (n == nullptr) {
            ajoute = true;
            return new Noeud(x);
        }
        if (x < n->valeur)
            n->gauche = inserer(n->gauche, x, ajoute);
        else if (x > n->valeur)
            n->droite = inserer(n->droite, x, ajoute);
        return n;                       // x == valeur : doublon ignoré
    }

    int hauteur(Noeud* n) const {
        if (n == nullptr) return -1;    // arbre vide : -1
        return 1 + max(hauteur(n->gauche), hauteur(n->droite));
    }

    void infixe(Noeud* n) const {
        if (n == nullptr) return;
        infixe(n->gauche);
        cout << n->valeur << " ";
        infixe(n->droite);
    }

    // Noeud ayant la plus petite valeur d'un sous-arbre (le plus à gauche)
    Noeud* minimum(Noeud* n) const {
        while (n->gauche != nullptr)
            n = n->gauche;
        return n;
    }

    // Suppression récursive : retourne la nouvelle racine du sous-arbre
    Noeud* supprimer(Noeud* n, int x, bool& supprime) {
        if (n == nullptr)
            return nullptr;                     // x absent

        if (x < n->valeur) {
            n->gauche = supprimer(n->gauche, x, supprime);
        } else if (x > n->valeur) {
            n->droite = supprimer(n->droite, x, supprime);
        } else {
            supprime = true;

            // Cas 1 : feuille ou un seul fils (droit)
            if (n->gauche == nullptr) {
                Noeud* fils = n->droite;
                delete n;
                return fils;
            }
            // Cas 2 : un seul fils (gauche)
            if (n->droite == nullptr) {
                Noeud* fils = n->gauche;
                delete n;
                return fils;
            }
            // Cas 3 : deux fils -> on remplace par le successeur
            // (plus petite valeur du sous-arbre droit), puis on le supprime
            Noeud* succ = minimum(n->droite);
            n->valeur = succ->valeur;
            bool inutile = false;
            n->droite = supprimer(n->droite, succ->valeur, inutile);
        }
        return n;
    }

    void detruire(Noeud* n) {
        if (n == nullptr) return;
        detruire(n->gauche);
        detruire(n->droite);
        delete n;
    }

public:
    ABR() : racine(nullptr), taille(0) {}
    ~ABR() { detruire(racine); }

    void inserer(int x) {
        bool ajoute = false;
        racine = inserer(racine, x, ajoute);
        if (ajoute) taille++;
    }

    // Retourne true si x a été trouvé et supprimé
    bool supprimer(int x) {
        bool supprime = false;
        racine = supprimer(racine, x, supprime);
        if (supprime) taille--;
        return supprime;
    }

    // Recherche itérative (sans récursion, donc plus rapide)
    bool rechercher(int x) const {
        Noeud* courant = racine;
        while (courant != nullptr) {
            if (x == courant->valeur) return true;
            courant = (x < courant->valeur) ? courant->gauche : courant->droite;
        }
        return false;
    }

    int hauteur() const { return hauteur(racine); }
    int getTaille() const { return taille; }
    void afficher() const { infixe(racine); cout << endl; }
};

// ---------- Petit exemple ----------
// Insère les valeurs dans l'ordre donné, puis montre recherche et suppressions
void exemple(const string& titre, const vector<int>& valeurs) {
    ABR a;
    for (int v : valeurs)
        a.inserer(v);

    cout << "=== Exemple : " << titre << " ===" << endl;
    cout << "Valeurs inserees : ";
    for (int v : valeurs) cout << v << " ";
    cout << endl;

    cout << "Parcours infixe  : ";
    a.afficher();
    cout << "Hauteur          : " << a.hauteur()
         << ", taille : " << a.getTaille() << endl;

    int dernier = valeurs.back();
    cout << "Recherche de " << dernier << "    : "
         << (a.rechercher(dernier) ? "trouve" : "absent") << endl;
    cout << "Recherche de 65    : "
         << (a.rechercher(65) ? "trouve" : "absent") << endl;

    cout << "\n--- Suppressions ---" << endl;

    auto supprimerEtAfficher = [&](int v, const string& role) {
        bool ok = a.supprimer(v);
        cout << "Suppression de " << setw(3) << v << " (" << role << ") : ";
        if (!ok) cout << "introuvable ";
        a.afficher();
    };

    supprimerEtAfficher(valeurs.front(), "premiere valeur inseree = racine");
    supprimerEtAfficher(valeurs.back(), "derniere valeur inseree = feuille");
    supprimerEtAfficher(valeurs[valeurs.size() / 2], "valeur du milieu de la liste");
    supprimerEtAfficher(999, "absent");

    cout << "Hauteur : " << a.hauteur() << ", taille : " << a.getTaille() << endl;
    cout << endl;
}

// ---------- Mesure du temps ----------
using Instant = high_resolution_clock::time_point;

double ms(Instant debut, Instant fin) {
    return duration<double, milli>(fin - debut).count();
}

// Mesure insertion, recherche et suppression pour l'ABR et la map
// sur un jeu de données donné (l'ordre de 'donnees' est l'ordre d'insertion)
void tester(const string& titre, const vector<int>& donnees, const vector<int>& aChercher) {
    const int N = (int)donnees.size();
    const int M = (int)aChercher.size();

    cout << "=== " << titre << " (N = " << N << ", M = " << M << ") ===" << endl;

    // --- Notre ABR ---
    ABR abr;
    auto d1 = high_resolution_clock::now();
    for (int x : donnees) abr.inserer(x);
    auto f1 = high_resolution_clock::now();

    int trouvesABR = 0;
    auto d2 = high_resolution_clock::now();
    for (int x : aChercher)
        if (abr.rechercher(x)) trouvesABR++;
    auto f2 = high_resolution_clock::now();

    // --- std::map pour comparer ---
    map<int, int> m;
    auto d3 = high_resolution_clock::now();
    for (int x : donnees) m[x] = 0;
    auto f3 = high_resolution_clock::now();

    int trouvesMap = 0;
    auto d4 = high_resolution_clock::now();
    for (int x : aChercher)
        if (m.find(x) != m.end()) trouvesMap++;
    auto f4 = high_resolution_clock::now();

    // État avant suppression (à relever avant de supprimer)
    int tailleABR = abr.getTaille();
    int hauteurABR = abr.hauteur();
    size_t tailleMap = m.size();

    // --- Suppression des valeurs cherchées (ABR puis map) ---
    int supprABR = 0;
    auto d5 = high_resolution_clock::now();
    for (int x : aChercher)
        if (abr.supprimer(x)) supprABR++;
    auto f5 = high_resolution_clock::now();

    int supprMap = 0;
    auto d6 = high_resolution_clock::now();
    for (int x : aChercher)
        supprMap += (int)m.erase(x);
    auto f6 = high_resolution_clock::now();

    double tIns[2]  = { ms(d1, f1), ms(d3, f3) };
    double tRech[2] = { ms(d2, f2), ms(d4, f4) };
    double tSupp[2] = { ms(d5, f5), ms(d6, f6) };

    cout << "ABR : taille " << tailleABR << ", hauteur " << hauteurABR
         << " | map : taille " << tailleMap << endl;
    cout << "Trouves : ABR " << trouvesABR << ", map " << trouvesMap
         << " | Supprimes : ABR " << supprABR << ", map " << supprMap << endl;

    cout << fixed << setprecision(2);
    cout << left << setw(14) << "Operation"
         << right << setw(12) << "ABR (ms)" << setw(12) << "map (ms)"
         << setw(14) << "map / ABR" << endl;

    auto ligne = [](const string& nom, double a, double b) {
        cout << left << setw(14) << nom
             << right << setw(12) << a << setw(12) << b
             << setw(14) << (a > 0 ? b / a : 0.0) << endl;
    };
    ligne("Insertion",   tIns[0],  tIns[1]);
    ligne("Recherche",   tRech[0], tRech[1]);
    ligne("Suppression", tSupp[0], tSupp[1]);
    cout << endl;
}

int main() {
    // Les memes 8 valeurs, inserees d'abord en ordre croissant puis en ordre aleatoire
    vector<int> croissant = {10, 20, 30, 40, 50, 60, 70, 80};
    vector<int> melange = croissant;
    mt19937 genExemple(42);
    shuffle(melange.begin(), melange.end(), genExemple);

    exemple("insertion en ORDRE CROISSANT", croissant);
    exemple("insertion en ORDRE ALEATOIRE", melange);

    mt19937 gen(42);
    uniform_int_distribution<int> dist(0, 1000000);

    // ----- Scénario 1 : insertion ALEATOIRE, grand nombre de données -----
    {
        const int N = 100000;
        vector<int> donnees(N), aChercher(N);
        for (int& x : donnees)   x = dist(gen);
        for (int& x : aChercher) x = dist(gen);
        tester("Insertion ALEATOIRE", donnees, aChercher);
    }

    // ----- Scénarios 2 et 3 : memes valeurs, ordre aleatoire puis trie -----
    // N plus petit : en ordre trie l'ABR devient une liste (hauteur ~ N),
    // donc les temps explosent (O(N^2)) et la récursion devient très profonde.
    {
        const int N = 5000;
        vector<int> aleatoire(N), aChercher(N);
        for (int& x : aleatoire) x = dist(gen);
        for (int& x : aChercher) x = dist(gen);

        vector<int> trie = aleatoire;
        sort(trie.begin(), trie.end());      // memes valeurs, ordre croissant

        tester("Insertion ALEATOIRE", aleatoire, aChercher);
        tester("Insertion en ORDRE CROISSANT (trie)", trie, aChercher);
    }

    cout << "Appuyez sur Entree pour quitter...";
    cin.get();   // garde la console ouverte

    return 0;
}
