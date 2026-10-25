#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts/QChartView>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif

// ---------------------------------------------------------------------------
//  Widget « promu » dans Qt Designer : QWidget  ->  ChartView (chartview.h)
//  Permet de placer un graphique QtCharts directement dans un fichier .ui
//  (compatible Qt 5 et Qt 6).
// ---------------------------------------------------------------------------
class ChartView : public QChartView
{
    Q_OBJECT

public:
    explicit ChartView(QWidget *parent = nullptr) : QChartView(parent)
    {
        setRenderHint(QPainter::Antialiasing);
        setFrameShape(QFrame::NoFrame);
        setStyleSheet("background: transparent;");
    }
};

#endif // CHARTVIEW_H
