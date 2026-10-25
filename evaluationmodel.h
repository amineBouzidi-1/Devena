#ifndef EVALUATIONMODEL_H
#define EVALUATIONMODEL_H

#include <QSqlQueryModel>

// ---------------------------------------------------------------------------
//  QSqlQueryModel avec mise en forme pour la vue (QTableView) :
//   - colonne « note »  : étoiles + valeur, en rouge si note < 3 (alerte)
//   - colonne « date »  : format jj/mm/aaaa (Oracle renvoie un QDateTime)
//   - colonne « rang »  : médailles 🥇🥈🥉 pour le podium
//  Aucune requête ici : le modèle est rempli par la classe Evaluation.
// ---------------------------------------------------------------------------
class EvaluationModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    explicit EvaluationModel(QObject *parent = nullptr);

    void setColonneNote(int c) { m_colNote = c; }
    void setColonneDate(int c) { m_colDate = c; }
    void setColonneRang(int c) { m_colRang = c; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    static constexpr double SEUIL_ALERTE = 3.0;
    static QString etoiles(double note);

private:
    int m_colNote = -1;
    int m_colDate = -1;
    int m_colRang = -1;
};

#endif // EVALUATIONMODEL_H
