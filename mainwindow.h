#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QDate>

// === Classes métier des modules (une par étudiant) ===
#include "evaluation.h"

class QButtonGroup;
class QPushButton;
class QTableView;
class QAbstractItemModel;
class QLayout;

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // =========================================================================
    // === SLOTS GÉNÉRAUX (NAVIGATION ENTRE PAGES) =============================
    // =========================================================================
    void allerVersPage(int index);

    // =========================================================================
    // === MODULE TABLEAU DE BORD ==============================================
    // =========================================================================
    // 💡 Étudiant responsable du module Tableau de bord travaille ici uniquement

    // =========================================================================
    // === MODULE PROFIL =======================================================
    // =========================================================================
    // 💡 Étudiant responsable du module Profil travaille ici uniquement

    // =========================================================================
    // === MODULE CHALLENGES ===================================================
    // =========================================================================
    // 💡 Étudiant responsable du module Challenges travaille ici uniquement

    // =========================================================================
    // === MODULE PROJETS ======================================================
    // =========================================================================
    // 💡 Étudiant responsable du module Projets travaille ici uniquement

    // =========================================================================
    // === MODULE ÉQUIPES ======================================================
    // =========================================================================
    // 💡 Étudiant responsable du module Équipes travaille ici uniquement

    // =========================================================================
    // === MODULE EVALUATION ===================================================
    // =========================================================================
    // 💡 Étudiant responsable du module Évaluation travaille ici uniquement
    void changerOngletEvaluation(int index);
    void actualiserEvaluations();
    void majSelectionEvaluation();
    void nouvelleEvaluation();
    void modifierEvaluationSelectionnee();
    void supprimerEvaluationSelectionnee();
    void enregistrerEvaluation();
    void annulerFormulaireEvaluation();
    void exporterEvaluationsPdf();
    void exporterFicheEvaluationPdf();
    void exporterAlertesPdf();
    void majEtoilesEvaluation(double note);
    void majCompteurEvaluation();

private:
    Ui::MainWindow *ui;

    // =========================================================================
    // === SLOTS GÉNÉRAUX ======================================================
    // =========================================================================
    QButtonGroup *m_navigation = nullptr;

    // =========================================================================
    // === MODULE EVALUATION ===================================================
    // =========================================================================
    void initModuleEvaluation();
    void preparerTableEvaluation(QTableView *table);
    void remplacerModeleEvaluation(QTableView *table, QAbstractItemModel *modele);
    void remplirListesEvaluation();
    void viderFormulaireEvaluation();
    bool controlerSaisieEvaluation();
    void marquerErreurEvaluation(QWidget *champ, bool enErreur);
    int  idEvaluationSelectionnee() const;
    QDate debutPeriodeEvaluation() const;
    void rafraichirStatistiquesEvaluation();
    void majGraphiqueBarresEvaluation(const StatistiquesEvaluation &s);
    void majGraphiqueDonutEvaluation(const StatistiquesEvaluation &s);
    void majDernieresEvaluations();
    bool exporterHtmlEnPdfEvaluation(const QString &html, const QString &nomParDefaut);
    QString htmlListeEvaluations(const QString &titre, const QList<ResumeEvaluation> &lignes) const;
    static void viderLayoutEvaluation(QLayout *layout);
    static QString ilYAEvaluation(const QDate &date);

    Evaluation          Evtmp;                         // objet de la classe métier (cf. atelier CRUD)
    int                 m_idEvaluationEnEdition = 0;   // 0 = ajout, sinon id modifié
    QList<QPushButton*> m_ongletsEvaluation;
    QWidget            *m_zoneLegendeEvaluation  = nullptr;
    QWidget            *m_zoneRecentesEvaluation = nullptr;
};

#endif // MAINWINDOW_H
