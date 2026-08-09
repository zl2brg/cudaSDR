#ifndef EQ_CURVE_PLOT_H
#define EQ_CURVE_PLOT_H

#include <QVector>
#include <QWidget>

/** Compact dB-vs-frequency plot for WDSP Get*EQDraw / Get*CFCOMP*Draw (1024 pts). */
class EqCurvePlot : public QWidget {
    Q_OBJECT
public:
    explicit EqCurvePlot(QWidget *parent = nullptr);

    void setCurve(const QVector<double> &xHz, const QVector<double> &yDb);
    void clearCurve();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> m_x;
    QVector<double> m_y;
};

#endif // EQ_CURVE_PLOT_H
