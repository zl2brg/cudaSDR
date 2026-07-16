#ifndef MAINWINDOWUI_H
#define MAINWINDOWUI_H

#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QSlider>
#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>
#include <QDateTime>
#include <QList>

#include "../../cusdr_settings.h"
#include "../../cusdr_fonts.h"
#include "../../Util/cusdr_buttons.h"
#include "../../GL/cusdr_oglDisplayPanel.h"

class MainWindow;

class MainWindowUI : public QObject {
    Q_OBJECT

public:
    explicit MainWindowUI(MainWindow *mainWindow, Settings* settings);
    ~MainWindowUI() override;

    void setup();
    void updateStatusBar(short load);

    // Buttons
    AeroButton* startBtn;
    AeroButton* serverBtn;
    AeroButton* setupBtn;
    AeroButton* modeBtn;
    AeroButton* viewBtn;
    AeroButton* wideBandBtn;
    AeroButton* plusRxBtn;
    AeroButton* quitBtn;
    AeroButton* moxBtn;
    AeroButton* tunBtn;
    AeroButton* alexBtn;
    AeroButton* attenuatorBtn;
    AeroButton* muteBtn;
    AeroButton* lastFreqBtn;
    AeroButton* nullBtn;

    // Sliders
    QSlider* micGainSlider;
    QSlider* drivelevelSlider;
    QSlider* volumeSlider;
    QSlider* agcGainSlider;

    // Labels
    QLabel* volLevelLabel;
    QLabel* agcGainLabel;
    QLabel* agcGainLevelLabel;
    QLabel* micGainLabel;
    QLabel* drivelevellLabel;
    QLabel* cpuLoadLabel;
    QLabel* dateTimeLabel;
    QLabel* activeDeviceLabel;

    // Menus & Actions
    QMenu* modeMenu;
    QMenu* viewMenu;
    QMenu* attenuatorMenu;
    QAction* sdrModeAction;
    QAction* chirpWSPRAction;
    QList<QAction*> mercuryAttnActionList;
    QList<QAction*> alexAttnActionList;

    QAction* setupAction;
    QAction* aboutAction;
    QAction* testAction; // renamed from test
    QList<AeroButton*> mainBtnList;

    QMenuBar* menuBar;
    QMenu* File;
    QMenu* Help;

    OGLDisplayPanel* m_oglDisplayPanel;
    QToolBar* m_mainBtnToolBar;
    QToolBar* m_displayPanelToolBar;

private:
    void createDisplayPanelToolBar();
    void createMainBtnToolBar();
    void createStatusToolBar();
    void createModeMenu();
    void createViewMenu();
    void createAttenuatorMenu();
    void setupActions();

    MainWindow* m_mainWindow;
    Settings* set;
    TFonts m_fonts;

    QWidget* m_buttonWidget;
};

#endif // MAINWINDOWUI_H
