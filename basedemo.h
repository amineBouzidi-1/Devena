#ifndef BASEDEMO_H
#define BASEDEMO_H

// ---------------------------------------------------------------------------
//  Base de démonstration (SQLite en mémoire)
//  Utilisée UNIQUEMENT si la connexion Oracle échoue, pour pouvoir tester
//  l'application. Elle reprend les tables de la base commune (mêmes noms de
//  tables et de colonnes) avec quelques données de test.
// ---------------------------------------------------------------------------
namespace BaseDemo {
bool initialiser();
}

#endif // BASEDEMO_H
