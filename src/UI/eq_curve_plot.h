#ifndef EQ_CURVE_PLOT_H
#define EQ_CURVE_PLOT_H

#include <QVector>
#include <QWidget>

/** Compact dB-vs-frequency plot for WDSP Get*EQDraw / Get*CFCOMP*Draw (1024 pts). */
class EqCurvePlot : public QWidget {
    Q_OBJECT
public:
    enum class PlotMode {
        Generic,
        BandEq,   // 11 bands: Preamp + 10 octave bands (32 Hz .. 16 kHz), -12 to +12 dB
        Cfc       // 10 speech bands: 50 Hz .. 3.1 kHz, -16 to +16 dB
    };

    static constexpr int kBandCount = 10;
    static constexpr int kBandSliderCount = 11; // Pre + 10 bands
    static constexpr int kCfcBandCount = 10;     // 10 CFC speech bands
    static constexpr double kEqAudioSampleRate = 48000.0;

    explicit EqCurvePlot(QWidget *parent = nullptr);

    void setPlotMode(PlotMode mode);
    PlotMode plotMode() const { return m_plotMode; }

    void setAccentColor(const QColor &color);
    QColor accentColor() const { return m_accentColor; }

    /** Generic linear X plot (CFC compressor curves). */
    void setCurve(const QVector<double> &x, const QVector<double> &yDb);
    /** 10-band graphic EQ: X is normalized frequency (2*Hz/sr), aligned with slider columns. */
    void setBandEqCurve(const QVector<double> &xNorm, const QVector<double> &yDb, double preampDb = 0.0,
                        double audioSampleRate = kEqAudioSampleRate);
    void clearCurve();

    static const char *bandLabel(int index);
    static double bandFrequencyHz(int index);
    static const char *cfcBandLabel(int index);
    static double cfcBandFrequencyHz(int index);

    /** Enable or disable interactive draggable nodes on the equalizer bands. */
    void setInteractive(bool interactive);
    bool isInteractive() const { return m_interactive; }

    /** Current band gains for Pre (index 0) and bands. */
    void setBandGains(const QVector<int> &gains);
    QVector<int> bandGains() const { return m_bandGains; }

    /** Highlight the receiver audio filter passband (e.g. 150 Hz to 2850 Hz). */
    void setAudioPassband(double lowHz, double highHz, bool visible = true);

signals:
    /** Emitted when the user drags a node to adjust a band's gain. */
    void bandGainChanged(int bandIndex, int gainDb);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    static double bandHzToPosition(double hz);
    static double bandPositionToX(double bandPos, int plotWidth);
    int totalBandCount() const;
    double minGainDb() const;
    double maxGainDb() const;
    int hitTestBand(const QPointF &pos, double *distOut = nullptr) const;
    double pixelToYDb(double yPixel, double ymin, double ymax, int top, int h) const;

    PlotMode m_plotMode = PlotMode::Generic;
    QColor m_accentColor = QColor(60, 200, 255);
    QVector<double> m_x;
    QVector<double> m_y;
    double m_preampDb = 0.0;
    double m_audioSampleRate = kEqAudioSampleRate;

    // Interactive node support
    bool m_interactive = false;
    QVector<int> m_bandGains; // size 11 (0=Pre, 1..10=bands)
    int m_hoveredBand = -1;
    int m_draggedBand = -1;
    bool m_isDragging = false;

    // Audio passband highlight
    bool m_hasPassband = false;
    double m_passbandLowHz = 0.0;
    double m_passbandHighHz = 0.0;
};

#endif // EQ_CURVE_PLOT_H
