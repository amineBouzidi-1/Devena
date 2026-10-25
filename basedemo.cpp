#include "basedemo.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QVariant>
#include <QDebug>

namespace {

bool executer(const QString &sql)
{
    QSqlQuery q;
    if (!q.exec(sql)) {
        qDebug() << "BaseDemo :" << q.lastError().text() << sql;
        return false;
    }
    return true;
}

} // namespace

bool BaseDemo::initialiser()
{
    // ----- Tables (mêmes noms et colonnes que dans Oracle) -----
    bool ok = executer("CREATE TABLE EQUIPE ("
                       " ID_EQUIPE INTEGER PRIMARY KEY, NOM_EQUIPE TEXT NOT NULL UNIQUE,"
                       " NB_MEMBRES INTEGER, CAPACITE_MAX INTEGER, SECTEUR TEXT,"
                       " AGE_MOYEN INTEGER, COMPETENCES TEXT)")
           && executer("CREATE TABLE EMPLOYE ("
                       " ID_EMPLOYE INTEGER PRIMARY KEY, NOM TEXT NOT NULL,"
                       " PRENOM TEXT NOT NULL, SPECIALITE TEXT)")
           && executer("CREATE TABLE EVALUATION ("
                       " ID_EVALUATION INTEGER PRIMARY KEY,"
                       " NOTE REAL NOT NULL CHECK (NOTE BETWEEN 0 AND 5),"
                       " COMMENTAIRE TEXT, DATE_EVAL DATE NOT NULL,"
                       " ID_EQUIPE INTEGER NOT NULL REFERENCES EQUIPE(ID_EQUIPE) ON DELETE CASCADE,"
                       " ID_EMPLOYE INTEGER NOT NULL REFERENCES EMPLOYE(ID_EMPLOYE))");
    if (!ok)
        return false;

    // ----- Données de test -----
    struct E { int id; const char *nom; int nb; const char *secteur; };
    const E equipes[] = {
        {1, "Equipe Innovatrice", 4, "Web"},
        {2, "Equipe Digital",     3, "Education"},
        {3, "Logic Devs",         5, "IA"},
        {4, "Code Breakers",      2, "Green Tech"},
    };
    for (const E &e : equipes) {
        QSqlQuery q;
        q.prepare("INSERT INTO EQUIPE (ID_EQUIPE, NOM_EQUIPE, NB_MEMBRES, CAPACITE_MAX, SECTEUR) "
                  "VALUES (:id, :nom, :nb, 6, :secteur)");
        q.bindValue(":id", e.id);
        q.bindValue(":nom", QString::fromUtf8(e.nom));
        q.bindValue(":nb", e.nb);
        q.bindValue(":secteur", QString::fromUtf8(e.secteur));
        if (!q.exec()) return false;
    }

    struct M { int id; const char *nom, *prenom, *specialite; };
    const M employes[] = {
        {1, "Mansour", "Leila", "Développement web"},
        {2, "Jaziri",  "Karim", "Intelligence artificielle"},
        {3, "Ayari",   "Hana",  "Gestion de projet"},
    };
    for (const M &m : employes) {
        QSqlQuery q;
        q.prepare("INSERT INTO EMPLOYE (ID_EMPLOYE, NOM, PRENOM, SPECIALITE) "
                  "VALUES (:id, :nom, :prenom, :spec)");
        q.bindValue(":id", m.id);
        q.bindValue(":nom", QString::fromUtf8(m.nom));
        q.bindValue(":prenom", QString::fromUtf8(m.prenom));
        q.bindValue(":spec", QString::fromUtf8(m.specialite));
        if (!q.exec()) return false;
    }

    struct V { double note; const char *comm; int jours; int equipe; int employe; };
    const V evaluations[] = {
        {4.6, "Très bonne collaboration",         0, 1, 1},
        {4.0, "Bon travail, documentation à compléter", 2, 2, 1},
        {4.8, "Excellente équipe",                1, 3, 2},
        {2.6, "Manque d'organisation",           40, 4, 3},
        {4.3, "Bonne présentation",               9, 1, 3},
        {2.8, "Retard sur le planning",          20, 2, 2},
        {4.9, "Solution très innovante",         60, 3, 1},
    };
    int id = 1;
    for (const V &v : evaluations) {
        QSqlQuery q;
        q.prepare("INSERT INTO EVALUATION (ID_EVALUATION, NOTE, COMMENTAIRE, DATE_EVAL, ID_EQUIPE, ID_EMPLOYE) "
                  "VALUES (:id, :note, :comm, :date, :equipe, :employe)");
        q.bindValue(":id", id++);
        q.bindValue(":note", v.note);
        q.bindValue(":comm", QString::fromUtf8(v.comm));
        q.bindValue(":date", QDate::currentDate().addDays(-v.jours));
        q.bindValue(":equipe", v.equipe);
        q.bindValue(":employe", v.employe);
        if (!q.exec()) return false;
    }
    return true;
}
