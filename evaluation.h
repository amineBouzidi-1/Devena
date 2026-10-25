#ifndef EVALUATION_H
#define EVALUATION_H

#include <QString>
#include <QDate>
#include <QList>
#include <QPair>
#include <QSqlQuery>
#include <QSqlQueryModel>

// ---------------------------------------------------------------------------
//  Classe purement C++ (MODÈLE) associée à la table EVALUATION
//
//  EVALUATION (ID_EVALUATION, NOTE, COMMENTAIRE, DATE_EVAL, #ID_EQUIPE, #ID_EMPLOYE)
//     - une évaluation concerne une équipe      (Equipe  1,1 -- évaluer -- 1,n Evaluation)
//     - une évaluation est faite par un employé (Employe 1,1 -- gérer   -- 1,n Evaluation)
//
//  Toutes les requêtes SQL du module sont ici (architecture Modèle-Vue),
//  et ce sont toutes des requêtes préparées (protection contre l'injection SQL).
// ---------------------------------------------------------------------------

// Statistiques calculées en base (métier « statistiques »)
struct StatistiquesEvaluation
{
    int    total          = 0;
    int    totalCeMois    = 0;
    int    equipesEvaluees = 0;
    int    evaluateurs    = 0;
    double moyenne        = 0.0;
    int    tranches[4]    = {0, 0, 0, 0};   // ≥4,5 | 4–4,5 | 3–4 | <3
    QList<QPair<QString, double>> moyenneParEquipe;
};

// Ligne résumée pour la liste « dernières évaluations »
struct ResumeEvaluation
{
    int     id = 0;
    QString equipe;
    QString evaluateur;
    double  note = 0.0;
    QString commentaire;
    QDate   date;
};

class Evaluation
{
    int     id;
    double  note;
    QString commentaire;
    QDate   dateEval;
    int     idEquipe;
    int     idEmploye;

public:
    // ----- Constructeurs -----
    Evaluation();
    Evaluation(int id, double note, const QString &commentaire, const QDate &dateEval,
               int idEquipe, int idEmploye);

    // ----- Getters -----
    int     getId()          const { return id; }
    double  getNote()        const { return note; }
    QString getCommentaire() const { return commentaire; }
    QDate   getDateEval()    const { return dateEval; }
    int     getIdEquipe()    const { return idEquipe; }
    int     getIdEmploye()   const { return idEmploye; }

    // ----- Setters -----
    void setId(int v)                     { id = v; }
    void setNote(double v)                { note = v; }
    void setCommentaire(const QString &v) { commentaire = v; }
    void setDateEval(const QDate &v)      { dateEval = v; }
    void setIdEquipe(int v)               { idEquipe = v; }
    void setIdEmploye(int v)              { idEmploye = v; }

    // ----- Fonctionnalités de base (CRUD) -----
    bool ajouter();                         // Create
    QSqlQueryModel *afficher(const QString &recherche = QString(),
                             int tri = 0, const QDate &depuis = QDate());   // Read
    bool modifier();                        // Update (utilise l'attribut id)
    bool supprimer(int id);                 // Delete
    bool chargerParId(int id);              // remplit l'objet depuis la base

    // ----- Contrôles métier -----
    bool existeDeja() const;                // même équipe + même employé + même date

    // ----- Fonctionnalités avancées -----
    QSqlQueryModel *classementEquipes();    // podium des équipes (moyenne des notes)
    QSqlQueryModel *alertes();              // évaluations avec une note < 3
    StatistiquesEvaluation statistiques();
    QList<ResumeEvaluation> dernieres(int nombre);
    QList<ResumeEvaluation> lister(const QString &recherche, int tri, const QDate &depuis);

    // ----- Listes pour les champs du formulaire (clés étrangères) -----
    static QList<QPair<int, QString>> listeEquipes();
    static QList<QPair<int, QString>> listeEmployes();

    static QString derniereErreur();

    // Colonnes renvoyées par afficher()
    enum Colonne { COL_ID = 0, COL_EQUIPE, COL_EVALUATEUR, COL_NOTE, COL_COMMENTAIRE, COL_DATE };
};

#endif // EVALUATION_H
