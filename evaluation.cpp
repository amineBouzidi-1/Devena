#include "evaluation.h"
#include "evaluationmodel.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <QObject>
#include <utility>

// ===========================================================================
//  Outils internes
// ===========================================================================
namespace {

QString g_erreur;

bool executer(QSqlQuery &q)
{
    if (!q.exec()) {
        g_erreur = q.lastError().text();
        return false;
    }
    g_erreur.clear();
    return true;
}

bool estOracle()
{
    return QSqlDatabase::database(QSqlDatabase::defaultConnection, false).driverName() == "QODBC";
}

// Partie commune des requêtes d'affichage (jointures vers les clés étrangères)
const char *SELECT_LISTE =
    "SELECT ev.ID_EVALUATION, eq.NOM_EQUIPE, em.PRENOM || ' ' || em.NOM AS EVALUATEUR, "
    "       ev.NOTE, ev.COMMENTAIRE, ev.DATE_EVAL "
    "FROM EVALUATION ev "
    "JOIN EQUIPE  eq ON eq.ID_EQUIPE  = ev.ID_EQUIPE "
    "JOIN EMPLOYE em ON em.ID_EMPLOYE = ev.ID_EMPLOYE ";

// Tri : seules ces valeurs sont possibles (jamais de texte saisi dans un ORDER BY)
QString clauseTri(int tri)
{
    switch (tri) {
    case 1:  return " ORDER BY ev.NOTE DESC, ev.DATE_EVAL DESC";
    case 2:  return " ORDER BY ev.NOTE ASC, ev.DATE_EVAL DESC";
    case 3:  return " ORDER BY eq.NOM_EQUIPE ASC, ev.DATE_EVAL DESC";
    default: return " ORDER BY ev.DATE_EVAL DESC, ev.ID_EVALUATION DESC";   // plus récentes
    }
}

// Recherche multicritère (équipe, évaluateur, commentaire) + période
QSqlQuery requeteListe(const QString &recherche, int tri, const QDate &depuis)
{
    QSqlQuery q;
    q.prepare(QString(SELECT_LISTE) +
              "WHERE (UPPER(eq.NOM_EQUIPE) LIKE :m1 "
              "    OR UPPER(em.PRENOM || ' ' || em.NOM) LIKE :m2 "
              "    OR UPPER(em.NOM || ' ' || em.PRENOM) LIKE :m3 "
              "    OR UPPER(ev.COMMENTAIRE) LIKE :m4) "
              "  AND ev.DATE_EVAL >= :depuis" + clauseTri(tri));

    const QString motif = "%" + recherche.trimmed().toUpper() + "%";
    q.bindValue(":m1", motif);
    q.bindValue(":m2", motif);
    q.bindValue(":m3", motif);
    q.bindValue(":m4", motif);
    q.bindValue(":depuis", depuis.isValid() ? depuis : QDate(1900, 1, 1));
    return q;
}

void entetesListe(QSqlQueryModel *model)
{
    model->setHeaderData(Evaluation::COL_ID,          Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(Evaluation::COL_EQUIPE,      Qt::Horizontal, QObject::tr("Équipe"));
    model->setHeaderData(Evaluation::COL_EVALUATEUR,  Qt::Horizontal, QObject::tr("Évaluateur"));
    model->setHeaderData(Evaluation::COL_NOTE,        Qt::Horizontal, QObject::tr("Note"));
    model->setHeaderData(Evaluation::COL_COMMENTAIRE, Qt::Horizontal, QObject::tr("Commentaire"));
    model->setHeaderData(Evaluation::COL_DATE,        Qt::Horizontal, QObject::tr("Date"));
}

QDate lireDate(const QVariant &v)
{
    return v.toDate().isValid() ? v.toDate() : v.toDateTime().date();
}

QList<ResumeEvaluation> lireResumes(QSqlQuery &q, int maximum = -1)
{
    QList<ResumeEvaluation> liste;
    while (q.next() && (maximum < 0 || liste.size() < maximum)) {
        ResumeEvaluation r;
        r.id         = q.value(0).toInt();
        r.equipe     = q.value(1).toString();
        r.evaluateur = q.value(2).toString();
        r.note       = q.value(3).toDouble();
        r.commentaire = q.value(4).toString();
        r.date       = lireDate(q.value(5));
        liste.append(r);
    }
    return liste;
}

} // namespace

// ===========================================================================
//  Constructeurs
// ===========================================================================
Evaluation::Evaluation()
    : id(0), note(0.0), dateEval(QDate::currentDate()), idEquipe(0), idEmploye(0)
{
}

Evaluation::Evaluation(int id, double note, const QString &commentaire, const QDate &dateEval,
                       int idEquipe, int idEmploye)
{
    this->id          = id;
    this->note        = note;
    this->commentaire = commentaire;
    this->dateEval    = dateEval;
    this->idEquipe    = idEquipe;
    this->idEmploye   = idEmploye;
}

QString Evaluation::derniereErreur()
{
    return g_erreur;
}

// ===========================================================================
//  CREATE
// ===========================================================================
bool Evaluation::ajouter()
{
    QSqlQuery query;

    if (estOracle()) {
        // Oracle : identifiant généré par la séquence
        query.prepare("INSERT INTO EVALUATION (ID_EVALUATION, NOTE, COMMENTAIRE, DATE_EVAL, ID_EQUIPE, ID_EMPLOYE) "
                      "VALUES (SEQ_EVALUATION.NEXTVAL, :note, :commentaire, :date, :equipe, :employe)");
    } else {
        // Base de démonstration : identifiant = max + 1
        QSqlQuery max;
        max.prepare("SELECT COALESCE(MAX(ID_EVALUATION), 0) + 1 FROM EVALUATION");
        if (!executer(max) || !max.next())
            return false;
        id = max.value(0).toInt();

        query.prepare("INSERT INTO EVALUATION (ID_EVALUATION, NOTE, COMMENTAIRE, DATE_EVAL, ID_EQUIPE, ID_EMPLOYE) "
                      "VALUES (:id, :note, :commentaire, :date, :equipe, :employe)");
        query.bindValue(":id", id);
    }

    // Création des variables liées
    query.bindValue(":note",        note);
    query.bindValue(":commentaire", commentaire);
    query.bindValue(":date",        dateEval);
    query.bindValue(":equipe",      idEquipe);
    query.bindValue(":employe",     idEmploye);

    return executer(query);
}

// ===========================================================================
//  READ
// ===========================================================================
QSqlQueryModel *Evaluation::afficher(const QString &recherche, int tri, const QDate &depuis)
{
    auto *model = new EvaluationModel();
    model->setColonneNote(COL_NOTE);
    model->setColonneDate(COL_DATE);

    QSqlQuery query = requeteListe(recherche, tri, depuis);
    executer(query);
    model->setQuery(std::move(query));
    entetesListe(model);
    return model;
}

QList<ResumeEvaluation> Evaluation::lister(const QString &recherche, int tri, const QDate &depuis)
{
    QSqlQuery query = requeteListe(recherche, tri, depuis);
    if (!executer(query))
        return {};
    return lireResumes(query);
}

bool Evaluation::chargerParId(int idRecherche)
{
    QSqlQuery query;
    query.prepare("SELECT NOTE, COMMENTAIRE, DATE_EVAL, ID_EQUIPE, ID_EMPLOYE "
                  "FROM EVALUATION WHERE ID_EVALUATION = :id");
    query.bindValue(":id", idRecherche);
    if (!executer(query) || !query.next())
        return false;

    id          = idRecherche;
    note        = query.value(0).toDouble();
    commentaire = query.value(1).toString();
    dateEval    = lireDate(query.value(2));
    idEquipe    = query.value(3).toInt();
    idEmploye   = query.value(4).toInt();
    return true;
}

// ===========================================================================
//  UPDATE
// ===========================================================================
bool Evaluation::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE EVALUATION SET NOTE = :note, COMMENTAIRE = :commentaire, DATE_EVAL = :date, "
                  "ID_EQUIPE = :equipe, ID_EMPLOYE = :employe WHERE ID_EVALUATION = :id");
    query.bindValue(":note",        note);
    query.bindValue(":commentaire", commentaire);
    query.bindValue(":date",        dateEval);
    query.bindValue(":equipe",      idEquipe);
    query.bindValue(":employe",     idEmploye);
    query.bindValue(":id",          id);

    if (!executer(query))
        return false;
    if (query.numRowsAffected() == 0) {
        g_erreur = QObject::tr("Aucune évaluation ne correspond à l'identifiant %1.").arg(id);
        return false;
    }
    return true;
}

// ===========================================================================
//  DELETE
// ===========================================================================
bool Evaluation::supprimer(int idASupprimer)
{
    QSqlQuery query;
    query.prepare("DELETE FROM EVALUATION WHERE ID_EVALUATION = :id");
    query.bindValue(":id", idASupprimer);

    if (!executer(query))
        return false;
    if (query.numRowsAffected() == 0) {
        g_erreur = QObject::tr("Aucune évaluation ne correspond à l'identifiant %1.").arg(idASupprimer);
        return false;
    }
    return true;
}

// ===========================================================================
//  Contrôle métier : pas deux évaluations de la même équipe par le même
//  employé le même jour
// ===========================================================================
bool Evaluation::existeDeja() const
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM EVALUATION "
                  "WHERE ID_EQUIPE = :equipe AND ID_EMPLOYE = :employe AND DATE_EVAL = :date "
                  "AND ID_EVALUATION <> :id");
    query.bindValue(":equipe",  idEquipe);
    query.bindValue(":employe", idEmploye);
    query.bindValue(":date",    dateEval);
    query.bindValue(":id",      id);
    if (!executer(query) || !query.next())
        return false;
    return query.value(0).toInt() > 0;
}

// ===========================================================================
//  Fonctionnalités avancées
// ===========================================================================
QSqlQueryModel *Evaluation::classementEquipes()
{
    auto *model = new EvaluationModel();
    model->setColonneRang(0);
    model->setColonneNote(2);

    QSqlQuery query;
    query.prepare("SELECT ROW_NUMBER() OVER (ORDER BY AVG(ev.NOTE) DESC) AS RANG, "
                  "       eq.NOM_EQUIPE, ROUND(AVG(ev.NOTE), 1) AS MOYENNE, COUNT(*) AS NB "
                  "FROM EVALUATION ev JOIN EQUIPE eq ON eq.ID_EQUIPE = ev.ID_EQUIPE "
                  "GROUP BY eq.NOM_EQUIPE ORDER BY RANG");
    executer(query);
    model->setQuery(std::move(query));
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Rang"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Équipe"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Moyenne"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Évaluations"));
    return model;
}

QSqlQueryModel *Evaluation::alertes()
{
    auto *model = new EvaluationModel();
    model->setColonneNote(COL_NOTE);
    model->setColonneDate(COL_DATE);

    QSqlQuery query;
    query.prepare(QString(SELECT_LISTE) + "WHERE ev.NOTE < :seuil ORDER BY ev.NOTE ASC, ev.DATE_EVAL DESC");
    query.bindValue(":seuil", EvaluationModel::SEUIL_ALERTE);
    executer(query);
    model->setQuery(std::move(query));
    entetesListe(model);
    return model;
}

StatistiquesEvaluation Evaluation::statistiques()
{
    StatistiquesEvaluation s;

    QSqlQuery q1;
    q1.prepare("SELECT COUNT(*), COALESCE(AVG(NOTE), 0), COUNT(DISTINCT ID_EQUIPE), COUNT(DISTINCT ID_EMPLOYE), "
               "       SUM(CASE WHEN NOTE >= 4.5 THEN 1 ELSE 0 END), "
               "       SUM(CASE WHEN NOTE >= 4 AND NOTE < 4.5 THEN 1 ELSE 0 END), "
               "       SUM(CASE WHEN NOTE >= 3 AND NOTE < 4 THEN 1 ELSE 0 END), "
               "       SUM(CASE WHEN NOTE < 3 THEN 1 ELSE 0 END) "
               "FROM EVALUATION");
    if (executer(q1) && q1.next()) {
        s.total           = q1.value(0).toInt();
        s.moyenne         = q1.value(1).toDouble();
        s.equipesEvaluees = q1.value(2).toInt();
        s.evaluateurs     = q1.value(3).toInt();
        for (int i = 0; i < 4; ++i)
            s.tranches[i] = q1.value(4 + i).toInt();
    }

    const QDate auj = QDate::currentDate();
    QSqlQuery q2;
    q2.prepare("SELECT COUNT(*) FROM EVALUATION WHERE DATE_EVAL >= :debut");
    q2.bindValue(":debut", QDate(auj.year(), auj.month(), 1));
    if (executer(q2) && q2.next())
        s.totalCeMois = q2.value(0).toInt();

    QSqlQuery q3;
    q3.prepare("SELECT eq.NOM_EQUIPE, AVG(ev.NOTE) "
               "FROM EVALUATION ev JOIN EQUIPE eq ON eq.ID_EQUIPE = ev.ID_EQUIPE "
               "GROUP BY eq.NOM_EQUIPE ORDER BY AVG(ev.NOTE) DESC");
    if (executer(q3))
        while (q3.next())
            s.moyenneParEquipe.append({q3.value(0).toString(), q3.value(1).toDouble()});

    return s;
}

QList<ResumeEvaluation> Evaluation::dernieres(int nombre)
{
    QSqlQuery query;
    query.prepare(QString(SELECT_LISTE) + clauseTri(0));
    if (!executer(query))
        return {};
    return lireResumes(query, nombre);
}

// ===========================================================================
//  Listes des clés étrangères (pour les listes déroulantes du formulaire)
// ===========================================================================
QList<QPair<int, QString>> Evaluation::listeEquipes()
{
    QList<QPair<int, QString>> liste;
    QSqlQuery query;
    query.prepare("SELECT ID_EQUIPE, NOM_EQUIPE FROM EQUIPE ORDER BY NOM_EQUIPE");
    if (executer(query))
        while (query.next())
            liste.append({query.value(0).toInt(), query.value(1).toString()});
    return liste;
}

QList<QPair<int, QString>> Evaluation::listeEmployes()
{
    QList<QPair<int, QString>> liste;
    QSqlQuery query;
    query.prepare("SELECT ID_EMPLOYE, PRENOM || ' ' || NOM FROM EMPLOYE ORDER BY NOM, PRENOM");
    if (executer(query))
        while (query.next())
            liste.append({query.value(0).toInt(), query.value(1).toString()});
    return liste;
}
