#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connection.h"
#include "chartview.h"
#include "evaluationmodel.h"

#include <QButtonGroup>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QTextDocument>
#include <QPdfWriter>
#include <QPageSize>
#include <QPageLayout>
#include <QDateTime>
#include <QRegularExpression>
#include <QStyle>
#include <algorithm>

#include <QtCharts/QChart>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

// =============================================================================
// === CONSTRUCTEUR & DESTRUCTEUR ==============================================
// =============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ----- Navigation : un bouton = une page du stackedWidget (même ordre) -----
    const QList<QPushButton*> boutons = {
        ui->btnNavTableau, ui->btnNavProfil, ui->btnNavChallenges, ui->btnNavProjets,
        ui->btnNavEquipes, ui->btnNavEvaluation, ui->btnNavStatistiques, ui->btnNavParametres
    };
    m_navigation = new QButtonGroup(this);
    m_navigation->setExclusive(true);
    for (int i = 0; i < boutons.size(); ++i) {
        m_navigation->addButton(boutons[i], i);
        connect(boutons[i], &QPushButton::clicked, this, [this, i] { allerVersPage(i); });
    }

    // Pastille rouge sur la cloche de notifications
    auto *pastille = new QLabel(ui->btnNotifications);
    pastille->setFixedSize(8, 8);
    pastille->setStyleSheet("background:#e2574c; border-radius:4px;");
    pastille->move(25, 6);

    // ----- Initialisation des modules (chaque étudiant appelle la sienne) -----
    initModuleEvaluation();

    allerVersPage(ui->stackedPages->indexOf(ui->pageEvaluation));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// =============================================================================
// === SLOTS GÉNÉRAUX (NAVIGATION ENTRE PAGES) =================================
// =============================================================================
void MainWindow::allerVersPage(int index)
{
    ui->stackedPages->setCurrentIndex(index);
    if (QAbstractButton *b = m_navigation->button(index))
        b->setChecked(true);
}

// =============================================================================
// === MODULE TABLEAU DE BORD ==================================================
// =============================================================================
// 💡 Étudiant responsable du module Tableau de bord écrit ici

// =============================================================================
// === MODULE PROFIL ===========================================================
// =============================================================================
// 💡 Étudiant responsable du module Profil écrit ici

// =============================================================================
// === MODULE CHALLENGES =======================================================
// =============================================================================
// 💡 Étudiant responsable du module Challenges écrit ici

// =============================================================================
// === MODULE PROJETS ==========================================================
// =============================================================================
// 💡 Étudiant responsable du module Projets écrit ici

// =============================================================================
// === MODULE ÉQUIPES ==========================================================
// =============================================================================
// 💡 Étudiant responsable du module Équipes écrit ici

// =============================================================================
// === MODULE EVALUATION =======================================================
// =============================================================================
// 💡 Étudiant responsable du module Évaluation travaille ici uniquement
//    Classe métier : evaluation.h / evaluation.cpp (aucune requête SQL ici)

// ===========================================================================
//  Outils internes (affichage uniquement, aucune requête)
// ===========================================================================
namespace {

const QColor EVAL_C_TEXTE("#e6f1f3");
const QColor EVAL_C_MUET("#8fb3bb");
const QColor EVAL_C_GRILLE("#1d4954");

const int LONGUEUR_MIN_COMMENTAIRE = 3;
const int LONGUEUR_MAX_COMMENTAIRE = 255;

QLabel *evalIconeCarree(const QString &icone, const QString &couleur, int taille)
{
    auto *l = new QLabel(icone);
    l->setFixedSize(taille, taille);
    l->setAlignment(Qt::AlignCenter);
    l->setStyleSheet(QString("background:%1; border-radius:10px; font-size:%2px;")
                     .arg(couleur).arg(taille / 2));
    return l;
}

void evalRemplacerGraphique(QChartView *vue, QChart *nouveau)
{
    QChart *ancien = vue->chart();
    vue->setChart(nouveau);
    delete ancien;
}

QChart *evalNouveauGraphique()
{
    auto *chart = new QChart;
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    return chart;
}

QString evalEchapper(const QString &t) { return t.toHtmlEscaped(); }

} // namespace

// ===========================================================================
//  Constructeur / destructeur
// ===========================================================================
void MainWindow::initModuleEvaluation()
{
    // ----- Onglets (QStackedWidget, pas de fenêtres de dialogue) -----
    m_ongletsEvaluation = {ui->btnOngletListe, ui->btnOngletFormulaire, ui->btnOngletClassement};
    for (int i = 0; i < m_ongletsEvaluation.size(); ++i)
        connect(m_ongletsEvaluation[i], &QPushButton::clicked, this, [this, i] { changerOngletEvaluation(i); });

    preparerTableEvaluation(ui->tableEvaluations);
    preparerTableEvaluation(ui->tableClassement);
    preparerTableEvaluation(ui->tableAlertes);

    // ----- Zones remplies dynamiquement -----
    m_zoneLegendeEvaluation = new QWidget;
    auto *vl = new QVBoxLayout(m_zoneLegendeEvaluation);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(9);
    ui->layoutLegende->addWidget(m_zoneLegendeEvaluation);

    m_zoneRecentesEvaluation = new QWidget;
    auto *vr = new QVBoxLayout(m_zoneRecentesEvaluation);
    vr->setContentsMargins(0, 0, 0, 0);
    vr->setSpacing(12);
    ui->layoutRecentes->addWidget(m_zoneRecentesEvaluation);
    ui->layoutRecentes->addStretch();

    ui->lblCentreDonut->setAttribute(Qt::WA_TransparentForMouseEvents);

    // ----- Indicateur Oracle / démonstration -----
    if (Connection::estModeDemo()) {
        ui->lblMode->setText(QString::fromUtf8("● Base de démonstration"));
        ui->lblMode->setStyleSheet("color:#f5c242; font-size:12px; padding-top:8px;");
        ui->lblMode->setToolTip("Oracle indisponible : les données ne sont pas enregistrées dans Oracle.");
    } else {
        ui->lblMode->setText(QString::fromUtf8("● Oracle connecté"));
        ui->lblMode->setStyleSheet("color:#3ddc97; font-size:12px; padding-top:8px;");
    }

    // ----- Formulaire -----
    ui->deDate->setMaximumDate(QDate::currentDate());
    ui->deDate->setMinimumDate(QDate(2000, 1, 1));
    ui->deDate->setDate(QDate::currentDate());

    // ----- Signaux / slots -----
    connect(ui->leRecherche, &QLineEdit::textChanged, this, &MainWindow::actualiserEvaluations);
    connect(ui->cbTri,     QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::actualiserEvaluations);
    connect(ui->cbPeriode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::actualiserEvaluations);

    connect(ui->btnNouvelle,   &QPushButton::clicked, this, &MainWindow::nouvelleEvaluation);
    connect(ui->btnModifier,   &QPushButton::clicked, this, &MainWindow::modifierEvaluationSelectionnee);
    connect(ui->btnSupprimer,  &QPushButton::clicked, this, &MainWindow::supprimerEvaluationSelectionnee);
    connect(ui->btnFiche,      &QPushButton::clicked, this, &MainWindow::exporterFicheEvaluationPdf);
    connect(ui->btnPdf,        &QPushButton::clicked, this, &MainWindow::exporterEvaluationsPdf);
    connect(ui->btnPdfAlertes, &QPushButton::clicked, this, &MainWindow::exporterAlertesPdf);
    connect(ui->btnEnregistrer,&QPushButton::clicked, this, &MainWindow::enregistrerEvaluation);
    connect(ui->btnAnnuler,    &QPushButton::clicked, this, &MainWindow::annulerFormulaireEvaluation);
    connect(ui->tableEvaluations, &QTableView::doubleClicked, this, &MainWindow::modifierEvaluationSelectionnee);

    connect(ui->dsbNote, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::majEtoilesEvaluation);
    connect(ui->teCommentaire, &QPlainTextEdit::textChanged, this, &MainWindow::majCompteurEvaluation);
    connect(ui->cbEquipe, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] {
        marquerErreurEvaluation(ui->cbEquipe, false);
    });
    connect(ui->cbEmploye, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] {
        marquerErreurEvaluation(ui->cbEmploye, false);
    });

    // ----- Affichage initial (appel de afficher(), comme dans l'atelier) -----
    viderFormulaireEvaluation();
    actualiserEvaluations();
    changerOngletEvaluation(0);
}

// ===========================================================================
//  Outils de la vue
// ===========================================================================
void MainWindow::preparerTableEvaluation(QTableView *t)
{
    t->verticalHeader()->hide();
    t->verticalHeader()->setDefaultSectionSize(42);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setFocusPolicy(Qt::NoFocus);
    t->setShowGrid(false);
    t->setWordWrap(false);
    t->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    t->horizontalHeader()->setHighlightSections(false);
}

// Remplace le modèle d'une QTableView en libérant l'ancien (évite les fuites mémoire)
void MainWindow::remplacerModeleEvaluation(QTableView *table, QAbstractItemModel *modele)
{
    QAbstractItemModel  *ancienModele    = table->model();
    QItemSelectionModel *ancienneSelection = table->selectionModel();

    table->setModel(modele);
    delete ancienneSelection;
    delete ancienModele;

    QHeaderView *hh = table->horizontalHeader();
    hh->setSectionResizeMode(QHeaderView::Stretch);
    if (modele->columnCount() > 0)
        hh->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    if (table == ui->tableEvaluations) {
        connect(table->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &MainWindow::majSelectionEvaluation);
    }
}

void MainWindow::viderLayoutEvaluation(QLayout *layout)
{
    if (!layout)
        return;
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *w = item->widget())
            delete w;
        if (QLayout *sous = item->layout()) {
            viderLayoutEvaluation(sous);
            delete sous;
        }
        delete item;
    }
}

QString MainWindow::ilYAEvaluation(const QDate &date)
{
    if (!date.isValid())
        return QString();
    const qint64 jours = date.daysTo(QDate::currentDate());
    if (jours <= 0) return "Aujourd'hui";
    if (jours == 1) return "Il y a 1 jour";
    if (jours < 30) return QString("Il y a %1 jours").arg(jours);
    return date.toString("dd/MM/yyyy");
}

int MainWindow::idEvaluationSelectionnee() const
{
    QItemSelectionModel *sel = ui->tableEvaluations->selectionModel();
    if (!sel || sel->selectedRows().isEmpty())
        return 0;
    const int ligne = sel->selectedRows().first().row();
    return ui->tableEvaluations->model()->index(ligne, Evaluation::COL_ID).data(Qt::UserRole).toInt();
}

QDate MainWindow::debutPeriodeEvaluation() const
{
    const QDate auj = QDate::currentDate();
    switch (ui->cbPeriode->currentIndex()) {
    case 1:  return QDate(auj.year(), auj.month(), 1);                          // ce mois-ci
    case 2:  return QDate(auj.year(), ((auj.month() - 1) / 3) * 3 + 1, 1);     // ce trimestre
    case 3:  return QDate(auj.year(), 1, 1);                                    // cette année
    default: return QDate();                                                    // toutes
    }
}

void MainWindow::changerOngletEvaluation(int index)
{
    if (index < 0 || index >= m_ongletsEvaluation.size())
        index = 0;
    for (int i = 0; i < m_ongletsEvaluation.size(); ++i)
        m_ongletsEvaluation[i]->setChecked(i == index);

    if (index == 1 && ui->cbEquipe->count() == 0)
        remplirListesEvaluation();
    ui->stackPages->setCurrentIndex(index);
}

// ===========================================================================
//  READ : actualisation de l'affichage (après chaque requête qui modifie la table)
// ===========================================================================
void MainWindow::actualiserEvaluations()
{
    remplacerModeleEvaluation(ui->tableEvaluations,
                    Evtmp.afficher(ui->leRecherche->text(), ui->cbTri->currentIndex(), debutPeriodeEvaluation()));
    remplacerModeleEvaluation(ui->tableClassement, Evtmp.classementEquipes());

    QSqlQueryModel *modeleAlertes = Evtmp.alertes();
    remplacerModeleEvaluation(ui->tableAlertes, modeleAlertes);

    const int nbLignes  = ui->tableEvaluations->model()->rowCount();
    const int nbAlertes = modeleAlertes->rowCount();
    ui->lblTitreListe->setText(QString::fromUtf8("Tableau des évaluations (%1)").arg(nbLignes));
    ui->btnOngletClassement->setText(QString::fromUtf8("🏆   Classement && alertes (%1)").arg(nbAlertes));
    ui->lblSousAlertes->setText(nbAlertes == 0
        ? QString::fromUtf8("Aucune alerte : toutes les notes sont supérieures ou égales à 3. 🎉")
        : QString::fromUtf8("%1 évaluation(s) ont une note inférieure à 3 : un accompagnement est recommandé.")
              .arg(nbAlertes));
    ui->btnPdfAlertes->setEnabled(nbAlertes > 0);

    rafraichirStatistiquesEvaluation();
    majSelectionEvaluation();
}

void MainWindow::majSelectionEvaluation()
{
    const int id = idEvaluationSelectionnee();
    const bool selection = id > 0;
    ui->btnModifier->setEnabled(selection);
    ui->btnSupprimer->setEnabled(selection);
    ui->btnFiche->setEnabled(selection);
    ui->lblSelection->setText(selection
        ? QString::fromUtf8("Évaluation n° %1 sélectionnée.").arg(id)
        : QString::fromUtf8("Sélectionnez une évaluation dans le tableau (double-clic pour la modifier)."));
}

// ===========================================================================
//  CREATE / UPDATE : formulaire intégré
// ===========================================================================
void MainWindow::remplirListesEvaluation()
{
    const int equipeActuelle  = ui->cbEquipe->currentData().toInt();
    const int employeActuel   = ui->cbEmploye->currentData().toInt();

    ui->cbEquipe->blockSignals(true);
    ui->cbEmploye->blockSignals(true);

    ui->cbEquipe->clear();
    ui->cbEquipe->addItem(QString::fromUtf8("— Choisir une équipe —"), 0);
    for (const auto &e : Evaluation::listeEquipes())
        ui->cbEquipe->addItem(e.second, e.first);

    ui->cbEmploye->clear();
    ui->cbEmploye->addItem(QString::fromUtf8("— Choisir un évaluateur —"), 0);
    for (const auto &e : Evaluation::listeEmployes())
        ui->cbEmploye->addItem(e.second, e.first);

    ui->cbEquipe->setCurrentIndex(qMax(0, ui->cbEquipe->findData(equipeActuelle)));
    ui->cbEmploye->setCurrentIndex(qMax(0, ui->cbEmploye->findData(employeActuel)));

    ui->cbEquipe->blockSignals(false);
    ui->cbEmploye->blockSignals(false);
}

void MainWindow::viderFormulaireEvaluation()
{
    m_idEvaluationEnEdition = 0;
    remplirListesEvaluation();
    ui->cbEquipe->setCurrentIndex(0);
    ui->cbEmploye->setCurrentIndex(0);
    ui->dsbNote->setValue(4.0);
    ui->deDate->setDate(QDate::currentDate());
    ui->teCommentaire->clear();
    ui->lblErreur->clear();
    ui->lblTitreFormulaire->setText(QString::fromUtf8("Nouvelle évaluation"));
    ui->btnEnregistrer->setText("Ajouter");
    for (QWidget *w : {static_cast<QWidget*>(ui->cbEquipe), static_cast<QWidget*>(ui->cbEmploye),
                       static_cast<QWidget*>(ui->deDate), static_cast<QWidget*>(ui->teCommentaire)})
        marquerErreurEvaluation(w, false);
    majEtoilesEvaluation(ui->dsbNote->value());
    majCompteurEvaluation();
}

void MainWindow::nouvelleEvaluation()
{
    viderFormulaireEvaluation();
    changerOngletEvaluation(1);
    ui->cbEquipe->setFocus();
}

void MainWindow::modifierEvaluationSelectionnee()
{
    const int id = idEvaluationSelectionnee();
    if (id <= 0) {
        QMessageBox::information(this, "Modifier", QString::fromUtf8("Sélectionnez d'abord une évaluation dans le tableau."));
        return;
    }

    Evaluation e;
    if (!e.chargerParId(id)) {
        QMessageBox::critical(this, "Erreur", QString::fromUtf8("Évaluation introuvable.\n") + Evaluation::derniereErreur());
        actualiserEvaluations();
        return;
    }

    viderFormulaireEvaluation();
    m_idEvaluationEnEdition = id;
    ui->cbEquipe->setCurrentIndex(qMax(0, ui->cbEquipe->findData(e.getIdEquipe())));
    ui->cbEmploye->setCurrentIndex(qMax(0, ui->cbEmploye->findData(e.getIdEmploye())));
    ui->dsbNote->setValue(e.getNote());
    ui->deDate->setDate(e.getDateEval().isValid() ? e.getDateEval() : QDate::currentDate());
    ui->teCommentaire->setPlainText(e.getCommentaire());
    ui->lblTitreFormulaire->setText(QString::fromUtf8("Modifier l'évaluation n° %1").arg(id));
    ui->btnEnregistrer->setText("Enregistrer les modifications");
    changerOngletEvaluation(1);
}

void MainWindow::annulerFormulaireEvaluation()
{
    viderFormulaireEvaluation();
    changerOngletEvaluation(0);
}

void MainWindow::majEtoilesEvaluation(double note)
{
    ui->lblEtoilesForm->setText(EvaluationModel::etoiles(note));
    ui->lblEtoilesForm->setStyleSheet(note < EvaluationModel::SEUIL_ALERTE ? "color:#ff8a8a;" : QString());
}

void MainWindow::majCompteurEvaluation()
{
    const int n = ui->teCommentaire->toPlainText().trimmed().length();
    ui->lblCompteur->setText(QString("%1 / %2").arg(n).arg(LONGUEUR_MAX_COMMENTAIRE));
    marquerErreurEvaluation(ui->teCommentaire, n > LONGUEUR_MAX_COMMENTAIRE);
}

void MainWindow::marquerErreurEvaluation(QWidget *champ, bool enErreur)
{
    champ->setProperty("erreur", enErreur);
    champ->style()->unpolish(champ);
    champ->style()->polish(champ);
}

// Contrôle de saisie (obligatoire dans le code C++, cf. consignes de l'atelier)
bool MainWindow::controlerSaisieEvaluation()
{
    QStringList erreurs;

    const int     idEquipe    = ui->cbEquipe->currentData().toInt();
    const int     idEmploye   = ui->cbEmploye->currentData().toInt();
    const double  note        = ui->dsbNote->value();
    const QDate   date        = ui->deDate->date();
    const QString commentaire = ui->teCommentaire->toPlainText().trimmed();

    marquerErreurEvaluation(ui->cbEquipe, idEquipe <= 0);
    if (idEquipe <= 0)
        erreurs << QString::fromUtf8("• Choisissez l'équipe évaluée.");

    marquerErreurEvaluation(ui->cbEmploye, idEmploye <= 0);
    if (idEmploye <= 0)
        erreurs << QString::fromUtf8("• Choisissez l'évaluateur.");

    if (note < 0.0 || note > 5.0)
        erreurs << QString::fromUtf8("• La note doit être comprise entre 0 et 5.");

    const bool dateOk = date.isValid() && date <= QDate::currentDate() && date >= QDate(2000, 1, 1);
    marquerErreurEvaluation(ui->deDate, !dateOk);
    if (!dateOk)
        erreurs << QString::fromUtf8("• La date doit être valide et ne peut pas être dans le futur.");

    static const QRegularExpression queChiffres("^[0-9\\s\\W]+$");
    const bool commentaireOk = commentaire.length() >= LONGUEUR_MIN_COMMENTAIRE
                            && commentaire.length() <= LONGUEUR_MAX_COMMENTAIRE
                            && !queChiffres.match(commentaire).hasMatch();
    marquerErreurEvaluation(ui->teCommentaire, !commentaireOk);
    if (!commentaireOk)
        erreurs << QString::fromUtf8("• Le commentaire doit contenir entre 3 et 255 caractères, avec au moins une lettre.");

    // Règle métier : pas deux évaluations de la même équipe par le même employé le même jour
    if (erreurs.isEmpty()) {
        Evaluation test(m_idEvaluationEnEdition, note, commentaire, date, idEquipe, idEmploye);
        if (test.existeDeja())
            erreurs << QString::fromUtf8("• Cet employé a déjà évalué cette équipe à cette date.");
    }

    ui->lblErreur->setText(erreurs.join("\n"));
    return erreurs.isEmpty();
}

void MainWindow::enregistrerEvaluation()
{
    if (!controlerSaisieEvaluation())
        return;

    // Récupération des informations saisies + instanciation de l'objet Evaluation
    Evaluation E(m_idEvaluationEnEdition,
                 ui->dsbNote->value(),
                 ui->teCommentaire->toPlainText().trimmed(),
                 ui->deDate->date(),
                 ui->cbEquipe->currentData().toInt(),
                 ui->cbEmploye->currentData().toInt());

    const bool ajout = (m_idEvaluationEnEdition == 0);
    const bool test  = ajout ? E.ajouter() : E.modifier();   // INSERT ou UPDATE dans la base

    if (test) {
        actualiserEvaluations();   // Refresh : ré-exécuter le SELECT
        QMessageBox::information(this, QObject::tr("OK"),
            ajout ? QObject::tr("Ajout effectué.") : QObject::tr("Modification effectuée."));
        viderFormulaireEvaluation();
        changerOngletEvaluation(0);
    } else {
        QMessageBox::critical(this, QObject::tr("Not OK"),
            (ajout ? QObject::tr("Ajout non effectué.\n") : QObject::tr("Modification non effectuée.\n"))
                + Evaluation::derniereErreur());
    }
}

// ===========================================================================
//  DELETE
// ===========================================================================
void MainWindow::supprimerEvaluationSelectionnee()
{
    const int id = idEvaluationSelectionnee();
    if (id <= 0)
        return;

    if (QMessageBox::question(this, "Supprimer",
            QString::fromUtf8("Supprimer définitivement l'évaluation n° %1 ?").arg(id)) != QMessageBox::Yes)
        return;

    const bool test = Evtmp.supprimer(id);   // DELETE dans la base
    if (test) {
        actualiserEvaluations();
        QMessageBox::information(this, QObject::tr("OK"), QObject::tr("Suppression effectuée."));
    } else {
        QMessageBox::critical(this, QObject::tr("Not OK"),
                              QObject::tr("Suppression non effectuée.\n") + Evaluation::derniereErreur());
    }
}

// ===========================================================================
//  Statistiques
// ===========================================================================
void MainWindow::rafraichirStatistiquesEvaluation()
{
    const StatistiquesEvaluation s = Evtmp.statistiques();

    ui->lblValTotal->setText(QString::number(s.total));
    ui->lblTrendTotal->setText(QString::fromUtf8("↗ +%1 ce mois").arg(s.totalCeMois));
    ui->lblValEquipes->setText(QString::number(s.equipesEvaluees));
    ui->lblTrendEquipes->setText(QString::fromUtf8("sur %1 équipes").arg(Evaluation::listeEquipes().size()));
    ui->lblValEmployes->setText(QString::number(s.evaluateurs));
    ui->lblTrendEmployes->setText(QString::fromUtf8("sur %1 employés").arg(Evaluation::listeEmployes().size()));
    ui->lblValNote->setText(QString::number(s.moyenne, 'f', 1));
    ui->lblTrendNote->setText(EvaluationModel::etoiles(s.moyenne));
    ui->lblCentreDonut->setText(
        QString("<div align='center'><span style='font-size:22px; font-weight:700;'>%1</span><br>"
                "<span style='font-size:11px; color:#8fb3bb;'>Note moyenne</span></div>")
            .arg(QString::number(s.moyenne, 'f', 1)));

    majGraphiqueBarresEvaluation(s);
    majGraphiqueDonutEvaluation(s);
    majDernieresEvaluations();
}

void MainWindow::majGraphiqueBarresEvaluation(const StatistiquesEvaluation &s)
{
    auto *serie = new QBarSet("Moyenne");
    QStringList categories;
    const int nb = std::min(5, static_cast<int>(s.moyenneParEquipe.size()));
    for (int i = 0; i < nb; ++i) {
        QString nom = s.moyenneParEquipe[i].first;
        if (nom.length() > 12)
            nom = nom.left(11) + QString::fromUtf8("…");
        categories << nom;
        *serie << qRound(s.moyenneParEquipe[i].second * 10.0) / 10.0;
    }
    serie->setColor(QColor("#2ec4c0"));
    serie->setBorderColor(Qt::transparent);
    serie->setLabelColor(EVAL_C_TEXTE);

    auto *barres = new QBarSeries;
    barres->append(serie);
    barres->setBarWidth(0.55);
    barres->setLabelsVisible(true);
    barres->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);

    QChart *chart = evalNouveauGraphique();
    chart->addSeries(barres);

    auto *axeX = new QBarCategoryAxis;
    axeX->append(categories);
    axeX->setLabelsColor(EVAL_C_MUET);
    axeX->setGridLineVisible(false);
    axeX->setLinePenColor(EVAL_C_GRILLE);

    auto *axeY = new QValueAxis;
    axeY->setRange(0, 5);
    axeY->setTickCount(6);
    axeY->setLabelFormat("%d");
    axeY->setLabelsColor(EVAL_C_MUET);
    axeY->setGridLineColor(EVAL_C_GRILLE);
    axeY->setLinePenColor(EVAL_C_GRILLE);

    chart->addAxis(axeX, Qt::AlignBottom);
    chart->addAxis(axeY, Qt::AlignLeft);
    barres->attachAxis(axeX);
    barres->attachAxis(axeY);

    evalRemplacerGraphique(ui->chartBarres, chart);
}

void MainWindow::majGraphiqueDonutEvaluation(const StatistiquesEvaluation &s)
{
    struct Tranche { QString nom; QString couleur; int nb; };
    const QList<Tranche> tranches = {
        {QString::fromUtf8("Excellent (≥ 4,5)"),   "#3ddc97", s.tranches[0]},
        {QString::fromUtf8("Très bien (4 – 4,5)"), "#2ec4c0", s.tranches[1]},
        {QString::fromUtf8("Bien (3 – 4)"),        "#f5c242", s.tranches[2]},
        {QString::fromUtf8("Faible (< 3)"),        "#e2574c", s.tranches[3]},
    };

    auto *donut = new QPieSeries;
    donut->setHoleSize(0.60);
    donut->setPieSize(0.92);

    viderLayoutEvaluation(m_zoneLegendeEvaluation->layout());
    for (const Tranche &tr : tranches) {
        if (tr.nb > 0) {
            QPieSlice *slice = donut->append(tr.nom, tr.nb);
            slice->setColor(QColor(tr.couleur));
            slice->setBorderColor(QColor("#10303a"));
            slice->setBorderWidth(2);
        }

        auto *ligne = new QWidget;
        auto *h = new QHBoxLayout(ligne);
        h->setContentsMargins(0, 0, 0, 0);
        auto *point = new QLabel(QString::fromUtf8("●"));
        point->setStyleSheet(QString("color:%1; font-size:14px;").arg(tr.couleur));
        auto *nom = new QLabel(tr.nom);
        nom->setStyleSheet("color:#cfe1e5; font-size:12px;");
        const int pct = s.total ? qRound(100.0 * tr.nb / s.total) : 0;
        auto *valeur = new QLabel(QString("%1  (%2 %)").arg(tr.nb).arg(pct));
        valeur->setProperty("role", "muted");
        h->addWidget(point);
        h->addWidget(nom);
        h->addStretch();
        h->addWidget(valeur);
        m_zoneLegendeEvaluation->layout()->addWidget(ligne);
    }

    QChart *chart = evalNouveauGraphique();
    chart->addSeries(donut);
    evalRemplacerGraphique(ui->chartDonut, chart);
}

void MainWindow::majDernieresEvaluations()
{
    viderLayoutEvaluation(m_zoneRecentesEvaluation->layout());

    const QList<ResumeEvaluation> recentes = Evtmp.dernieres(3);
    if (recentes.isEmpty()) {
        auto *vide = new QLabel(QString::fromUtf8("Aucune évaluation pour le moment."));
        vide->setProperty("role", "muted");
        m_zoneRecentesEvaluation->layout()->addWidget(vide);
        return;
    }

    for (const ResumeEvaluation &r : recentes) {
        auto *ligne = new QWidget;
        auto *h = new QHBoxLayout(ligne);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(12);
        h->addWidget(evalIconeCarree(QString::fromUtf8("👥"), "rgba(46,150,220,0.25)", 38));

        auto *textes = new QVBoxLayout;
        textes->setSpacing(1);
        auto *nom = new QLabel(r.equipe);
        nom->setStyleSheet("font-weight:600;");
        auto *par = new QLabel(QString::fromUtf8("par %1").arg(r.evaluateur));
        par->setProperty("role", "muted");
        textes->addWidget(nom);
        textes->addWidget(par);

        auto *note = new QLabel(QString("<span style='color:%1;'>★</span> <b>%2</b>")
                                    .arg(r.note < EvaluationModel::SEUIL_ALERTE ? "#ff8a8a" : "#f5c242",
                                         QString::number(r.note, 'f', 1)));
        auto *quand = new QLabel(ilYAEvaluation(r.date));
        quand->setProperty("role", "muted");
        quand->setFixedWidth(90);
        quand->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        h->addLayout(textes);
        h->addStretch();
        h->addWidget(note);
        h->addWidget(quand);
        m_zoneRecentesEvaluation->layout()->addWidget(ligne);
    }
}

// ===========================================================================
//  Export PDF
// ===========================================================================
QString MainWindow::htmlListeEvaluations(const QString &titre, const QList<ResumeEvaluation> &lignes) const
{
    double somme = 0;
    for (const ResumeEvaluation &r : lignes)
        somme += r.note;
    const double moyenne = lignes.isEmpty() ? 0.0 : somme / lignes.size();

    QString html;
    html += "<html><body style='font-family:Arial; font-size:10pt; color:#1b2b30;'>";
    html += "<table width='100%'><tr>"
            "<td><span style='font-size:20pt; font-weight:bold; color:#127c79;'>&#9670; Devena</span><br>"
            "<span style='color:#5f7c83;'>Connecter les talents, construire l'avenir.</span></td>"
            "<td align='right' style='color:#5f7c83;'>Généré le " +
            QDateTime::currentDateTime().toString("dd/MM/yyyy à HH:mm") + "</td></tr></table><hr>";
    html += "<h2 style='color:#127c79;'>" + evalEchapper(titre) + "</h2>";
    html += QString("<p><b>%1</b> évaluation(s) &nbsp;|&nbsp; Note moyenne : <b>%2 / 5</b></p>")
                .arg(lignes.size()).arg(QString::number(moyenne, 'f', 1));
    html += "<table width='100%' cellspacing='0' cellpadding='6' border='1' style='border-collapse:collapse; border-color:#c9dadd;'>"
            "<tr style='background-color:#127c79; color:white;'>"
            "<th>ID</th><th>Équipe</th><th>Évaluateur</th><th>Note</th><th>Commentaire</th><th>Date</th></tr>";
    for (int i = 0; i < lignes.size(); ++i) {
        const ResumeEvaluation &r = lignes[i];
        const bool alerte = r.note < EvaluationModel::SEUIL_ALERTE;
        html += QString("<tr style='background-color:%1;'>").arg(alerte ? "#fde8e8" : (i % 2 ? "#f2f8f9" : "#ffffff"));
        html += "<td>" + QString::number(r.id) + "</td>";
        html += "<td>" + evalEchapper(r.equipe) + "</td>";
        html += "<td>" + evalEchapper(r.evaluateur) + "</td>";
        html += QString("<td><b style='color:%1;'>%2</b></td>")
                    .arg(alerte ? "#c0392b" : "#127c79", QString::number(r.note, 'f', 1));
        html += "<td>" + evalEchapper(r.commentaire) + "</td>";
        html += "<td>" + r.date.toString("dd/MM/yyyy") + "</td></tr>";
    }
    html += "</table><p style='color:#5f7c83; font-size:8pt;'>Lignes en rouge : note inférieure à 3.</p>";
    html += "</body></html>";
    return html;
}

bool MainWindow::exporterHtmlEnPdfEvaluation(const QString &html, const QString &nomParDefaut)
{
    const QString dossier = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString chemin = QFileDialog::getSaveFileName(this, "Enregistrer le PDF",
                                                  QDir(dossier).filePath(nomParDefaut),
                                                  "Fichiers PDF (*.pdf)");
    if (chemin.isEmpty())
        return false;
    if (!chemin.endsWith(".pdf", Qt::CaseInsensitive))
        chemin += ".pdf";

    QPdfWriter writer(chemin);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    writer.setTitle("Devena - Evaluations");
    writer.setCreator("Devena");

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&writer);

    if (QMessageBox::question(this, "Export PDF",
            QString::fromUtf8("PDF enregistré :\n%1\n\nVoulez-vous l'ouvrir ?").arg(chemin)) == QMessageBox::Yes)
        QDesktopServices::openUrl(QUrl::fromLocalFile(chemin));
    return true;
}

void MainWindow::exporterEvaluationsPdf()
{
    const QList<ResumeEvaluation> lignes =
        Evtmp.lister(ui->leRecherche->text(), ui->cbTri->currentIndex(), debutPeriodeEvaluation());
    if (lignes.isEmpty()) {
        QMessageBox::information(this, "Export PDF", QString::fromUtf8("Aucune évaluation à exporter."));
        return;
    }
    const QString titre = QString::fromUtf8("Liste des évaluations — %1").arg(ui->cbPeriode->currentText());
    exporterHtmlEnPdfEvaluation(htmlListeEvaluations(titre, lignes),
                      QString("evaluations_%1.pdf").arg(QDate::currentDate().toString("yyyyMMdd")));
}

void MainWindow::exporterAlertesPdf()
{
    QList<ResumeEvaluation> faibles;
    for (const ResumeEvaluation &r : Evtmp.lister(QString(), 2, QDate()))   // tri par note croissante
        if (r.note < EvaluationModel::SEUIL_ALERTE)
            faibles.append(r);
    if (faibles.isEmpty())
        return;
    exporterHtmlEnPdfEvaluation(htmlListeEvaluations(QString::fromUtf8("Évaluations à surveiller (note < 3)"), faibles),
                      QString("alertes_%1.pdf").arg(QDate::currentDate().toString("yyyyMMdd")));
}

void MainWindow::exporterFicheEvaluationPdf()
{
    const int id = idEvaluationSelectionnee();
    if (id <= 0)
        return;

    ResumeEvaluation r;
    bool trouve = false;
    for (const ResumeEvaluation &x : Evtmp.lister(QString(), 0, QDate()))
        if (x.id == id) { r = x; trouve = true; break; }
    if (!trouve)
        return;

    const bool alerte = r.note < EvaluationModel::SEUIL_ALERTE;
    QString html;
    html += "<html><body style='font-family:Arial; font-size:11pt; color:#1b2b30;'>";
    html += "<p><span style='font-size:20pt; font-weight:bold; color:#127c79;'>&#9670; Devena</span><br>"
            "<span style='color:#5f7c83;'>Fiche d'évaluation</span></p><hr>";
    html += QString("<h1>%1</h1>").arg(evalEchapper(r.equipe));
    html += QString("<p style='color:#5f7c83;'>Évaluation n° %1 &nbsp;|&nbsp; %2</p>")
                .arg(r.id).arg(r.date.toString("dd/MM/yyyy"));
    html += "<table cellpadding='8' cellspacing='0' border='1' width='100%' style='border-collapse:collapse; border-color:#c9dadd;'>";
    html += "<tr><td width='35%' style='background-color:#eaf4f5;'><b>Évaluateur</b></td><td>" + evalEchapper(r.evaluateur) + "</td></tr>";
    html += QString("<tr><td style='background-color:#eaf4f5;'><b>Note</b></td>"
                    "<td><b style='font-size:16pt; color:%1;'>%2 / 5</b></td></tr>")
                .arg(alerte ? "#c0392b" : "#127c79", QString::number(r.note, 'f', 1));
    html += "<tr><td style='background-color:#eaf4f5;'><b>Commentaire</b></td><td>" + evalEchapper(r.commentaire) + "</td></tr>";
    html += "</table>";
    if (alerte)
        html += "<p style='color:#c0392b;'><b>&#9888; Alerte :</b> note inférieure à 3, un accompagnement est recommandé.</p>";
    html += "<p style='color:#5f7c83; font-size:8pt;'>Généré le " +
            QDateTime::currentDateTime().toString("dd/MM/yyyy à HH:mm") + "</p></body></html>";

    QString nom = r.equipe;
    nom.replace(QRegularExpression("[^A-Za-z0-9]+"), "_");
    exporterHtmlEnPdfEvaluation(html, QString("fiche_evaluation_%1_%2.pdf").arg(r.id).arg(nom));
}

// =============================================================================
// === FIN MODULE EVALUATION ===================================================
// =============================================================================
