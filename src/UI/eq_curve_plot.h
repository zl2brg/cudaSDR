#ifndef EQ_CURVE_PLOT_H
#define EQ_CURVE_PLOT_H

#include <QVector>
#include <QWidget>

/** Compact dB-vs-frequency plot for WDSP Get*EQDraw / Get*CFCOMP*Draw (1024 pts). */
class EqCurvePlot : public QWidget {
    Q_OBJECT
public:
    static constexpr int kBandCount = 10;
    static constexpr int kBandSliderCount = 11; // Pre + 10 bands
    static constexpr double kEqAudioSampleRate = 48000.0;

    explicit EqCurvePlot(QWidget *parent = nullptr);

    /** Generic linear X plot (CFC compressor curves). */
    void setCurve(const QVector<double> &x, const QVector<double> &yDb);
    /** 10-band graphic EQ: X is normalized frequency (2*Hz/sr), aligned with slider columns. */
    void setBandEqCurve(const QVector<double> &xNorm, const QVector<double> &yDb, double preampDb = 0.0,
                        double audioSampleRate = kEqAudioSampleRate);
    void clearCurve();

    static const char *bandLabel(int index);
    static double bandFrequencyHz(int index);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    enum class Style { Generic, BandEq };

    static double bandHzToPosition(double hz);
    static double bandPositionToX(double bandPos, int plotWidth);

    Style m_style = Style::Generic;
    QVector<double> m_x;
    QVector<double> m_y;
    double m_preampDb = 0.0;
    double m_audioSampleRate = kEqAudioSampleRate;
};

#endif // EQ_CURVE_PLOT_H
