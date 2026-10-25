#include "evaluationmodel.h"

#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QBrush>

EvaluationModel::EvaluationModel(QObject *parent) : QSqlQueryModel(parent)
{
}

QString EvaluationModel::etoiles(double note)
{
    const int pleines = qBound(0, static_cast<int>(note + 0.3), 5);
    return QString::fromUtf8("★").repeated(pleines) + QString::fromUtf8("☆").repeated(5 - pleines);
}

QVariant EvaluationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const QVariant brut = QSqlQueryModel::data(index, Qt::DisplayRole);
    const int col = index.column();

    // Note de la ligne (sert aussi à colorer toute la ligne en cas d'alerte)
    double noteLigne = -1;
    if (m_colNote >= 0)
        noteLigne = QSqlQueryModel::data(this->index(index.row(), m_colNote), Qt::DisplayRole).toDouble();
    const bool alerte = noteLigne >= 0 && noteLigne < SEUIL_ALERTE;

    switch (role) {
    case Qt::DisplayRole:
        if (col == m_colNote) {
            const double n = brut.toDouble();
            return etoiles(n) + "   " + QString::number(n, 'f', 1) + (alerte ? QString::fromUtf8("  ⚠") : QString());
        }
        if (col == m_colDate) {
            const QDate d = brut.toDate().isValid() ? brut.toDate() : brut.toDateTime().date();
            return d.isValid() ? d.toString("dd/MM/yyyy") : brut.toString();
        }
        if (col == m_colRang) {
            const int r = brut.toInt();
            if (r == 1) return QString::fromUtf8("🥇");
            if (r == 2) return QString::fromUtf8("🥈");
            if (r == 3) return QString::fromUtf8("🥉");
            return QString::number(r);
        }
        if (brut.typeId() == QMetaType::Double)
            return QString::number(brut.toDouble(), 'f', 1);
        return brut;

    case Qt::ForegroundRole:
        if (col == m_colNote)
            return QBrush(alerte ? QColor("#ff8a8a") : QColor("#f5c242"));
        return QVariant();

    case Qt::BackgroundRole:
        if (alerte)
            return QBrush(QColor(110, 39, 48, 70));
        return QVariant();

    case Qt::TextAlignmentRole:
        if (col == m_colRang)
            return int(Qt::AlignCenter);
        return int(Qt::AlignLeft | Qt::AlignVCenter);

    case Qt::UserRole:          // valeur brute (pour le code)
        return brut;

    default:
        return QSqlQueryModel::data(index, role);
    }
}
