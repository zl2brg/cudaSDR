#include "MainWindowUI.h"
#include "../../cusdr_mainWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>

MainWindowUI::MainWindowUI(MainWindow *mainWindow)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
{
    set = Settings::instance();
    CFonts* fonts = new CFonts(this);
    m_fonts = fonts->getFonts();
    m_oglDisplayPanel = nullptr;
}

MainWindowUI::~MainWindowUI() {}

void MainWindowUI::setup() {
    menuBar = new QMenuBar(m_mainWindow);
    File = menuBar->addMenu(tr("File"));
    Help = menuBar->addMenu(tr("Help"));
    File->setTitle("File");

    setupActions();
    
    File->addAction(setupAction);
    File->addAction(testAction);
    Help->addAction(aboutAction);
    
    m_mainWindow->setMenuBar(menuBar);

    createDisplayPanelToolBar();
    createMainBtnToolBar();
    createStatusToolBar();
    createModeMenu();
    createViewMenu();
    createAttenuatorMenu();
}

void MainWindowUI::setupActions() {
    setupAction = new QAction(tr("&Setup"), m_mainWindow);
    setupAction->setStatusTip(tr("Setup Menu"));
    CHECKED_CONNECT(setupAction, &QAction::triggered, m_mainWindow, &MainWindow::cusdr_setup);

    aboutAction = new QAction(tr("&About"), m_mainWindow);
    aboutAction->setStatusTip(tr("About cudaSDR"));
    CHECKED_CONNECT(aboutAction, &QAction::triggered, m_mainWindow, &MainWindow::showAboutDialog);

    testAction = new QAction(tr("Test"), m_mainWindow);
}

void MainWindowUI::createStatusToolBar() {
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString() + " (loc)";

    cpuLoadLabel = new QLabel("CPU load:     ", m_mainWindow);
    dateTimeLabel = new QLabel(dateTimeString, m_mainWindow);

    m_mainWindow->statusBar()->addPermanentWidget(cpuLoadLabel);
    m_mainWindow->statusBar()->insertPermanentWidget(1, dateTimeLabel, 0);
}

void MainWindowUI::updateStatusBar(short load) {
    QString str = "CPU load: %1 % \t";
    cpuLoadLabel->setText(str.arg(load));

    QDateTime dateTime = QDateTime::currentDateTime();
    dateTimeLabel->setText(dateTime.toString() + " (loc)");

    m_mainWindow->statusBar()->update();
}

void MainWindowUI::createDisplayPanelToolBar() {
    m_displayPanelToolBar = new QToolBar(tr("Display Panel"), m_mainWindow);
    m_displayPanelToolBar->setObjectName("DisplayPanel");
    m_displayPanelToolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_displayPanelToolBar->setMovable(false);

    m_oglDisplayPanel = new OGLDisplayPanel(m_displayPanelToolBar);
    m_displayPanelToolBar->addWidget(m_oglDisplayPanel);

    m_mainWindow->addToolBar(m_displayPanelToolBar);
    m_mainWindow->addToolBarBreak(Qt::TopToolBarArea);
}

void MainWindowUI::createMainBtnToolBar() {
    m_mainBtnToolBar = new QToolBar(tr("Main Buttons"), m_mainWindow);
    m_mainBtnToolBar->setObjectName("MainButtons");
    m_mainBtnToolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_mainBtnToolBar->setMovable(false);
    
    m_buttonWidget = new QWidget(m_mainWindow);
    QColor btnCol = QColor(230, 230, 230);
    int btn_width1 = 75;
    int btn_height1 = 21;
    int btn_height3 = 16;
    int btn_width3 = 48;

    startBtn = new AeroButton("Start", m_mainWindow);
    startBtn->setRoundness(10);
    startBtn->setFont(m_fonts.normalFont);
    startBtn->setTextColor(btnCol);
    startBtn->setFixedSize(btn_width1, btn_height1);
    startBtn->setEnabled(false);
    CHECKED_CONNECT(startBtn, &AeroButton::clicked, m_mainWindow, &MainWindow::startButtonClickedEvent);

    serverBtn = new AeroButton("Server", m_mainWindow);
    serverBtn->setRoundness(10);
    serverBtn->setFont(m_fonts.normalFont);
    serverBtn->setTextColor(btnCol);
    serverBtn->setFixedSize(btn_width1, btn_height1);
    mainBtnList.append(serverBtn);
    CHECKED_CONNECT(serverBtn, &AeroButton::clicked, m_mainWindow, &MainWindow::widgetBtnClickedEvent);

    setupBtn = new AeroButton("Setup", m_mainWindow);
    setupBtn->setRoundness(10);
    setupBtn->setFont(m_fonts.normalFont);
    setupBtn->setTextColor(btnCol);
    setupBtn->setFixedSize(btn_width1, btn_height1);
    mainBtnList.append(setupBtn);
    CHECKED_CONNECT(setupBtn, &AeroButton::clicked, m_mainWindow, &MainWindow::widgetBtnClickedEvent);

    wideBandBtn = new AeroButton("Wideband", m_mainWindow);
    wideBandBtn->setRoundness(10);
    wideBandBtn->setFont(m_fonts.normalFont);
    wideBandBtn->setTextColor(btnCol);
    wideBandBtn->setFixedSize(btn_width1, btn_height1);
    wideBandBtn->setEnabled(false);
    CHECKED_CONNECT(wideBandBtn, &AeroButton::clicked, m_mainWindow, &MainWindow::wideBandBtnClickedEvent);

    QColor nullBtnCol(90, 90, 90);
    nullBtn = new AeroButton(m_mainWindow);
    nullBtn->setRoundness(0);
    nullBtn->setFixedHeight(btn_height1);
    nullBtn->setHighlight(nullBtnCol);
    nullBtn->setEnabled(false);

    plusRxBtn = new AeroButton("+Rx", m_mainWindow);
    plusRxBtn->setRoundness(10);
    plusRxBtn->setFont(m_fonts.normalFont);
    plusRxBtn->setTextColor(btnCol);
    plusRxBtn->setFixedSize(btn_width1, btn_height1);
    plusRxBtn->setEnabled(false);
    CHECKED_CONNECT(plusRxBtn, &AeroButton::clicked, m_mainWindow, &MainWindow::addReceiver);

    viewBtn = new AeroButton("View Rx", m_mainWindow);
    viewBtn->setRoundness(10);
    viewBtn->setFont(m_fonts.normalFont);
    viewBtn->setTextColor(btnCol);
    viewBtn->setFixedSize(btn_width1, btn_height1);

    modeBtn = new AeroButton("Mode", m_mainWindow);
    modeBtn->setRoundness(10);
    modeBtn->setFont(m_fonts.normalFont);
    modeBtn->setTextColor(btnCol);
    modeBtn->setFixedSize(btn_width1, btn_height1);

    quitBtn = new AeroButton("Quit", m_mainWindow);
    quitBtn->setRoundness(10);
    quitBtn->setFont(m_fonts.normalFont);
    quitBtn->setTextColor(btnCol);
    quitBtn->setFixedSize(btn_width3, btn_height1);
    CHECKED_CONNECT(quitBtn, &AeroButton::clicked, m_mainWindow, &MainWindow::closeMainWindow);

    int vol = (int)(set->getMainVolume(0) * 100);
    micGainSlider = new QSlider(Qt::Horizontal, m_mainWindow);
    micGainSlider->setFixedSize(100, 14);
    micGainSlider->setRange(0, 128);
    micGainSlider->setValue(vol);
    CHECKED_CONNECT(micGainSlider, &QSlider::valueChanged, m_mainWindow, &MainWindow::setMicLevel);

    drivelevelSlider = new QSlider(Qt::Horizontal, m_mainWindow);
    drivelevelSlider->setFixedSize(100, 14);
    drivelevelSlider->setRange(0, 128);
    drivelevelSlider->setValue(set->getDriveLevel());
    CHECKED_CONNECT(drivelevelSlider, &QSlider::valueChanged, m_mainWindow, &MainWindow::setDriveLevel);

    volumeSlider = new QSlider(Qt::Horizontal, m_mainWindow);
    volumeSlider->setFixedSize(100, 14);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(vol);
    CHECKED_CONNECT(volumeSlider, &QSlider::valueChanged, m_mainWindow, &MainWindow::setMainVolume);

    micGainLabel = new QLabel("Mic:", m_mainWindow);
    micGainLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    drivelevellLabel = new QLabel("Drive:", m_mainWindow);
    drivelevellLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    QLabel* volumeLabel = new QLabel("Vol:", m_mainWindow);
    volumeLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

    int fontMaxWidth = m_fonts.smallFontMetrics->boundingRect("100 % ").width();
    volLevelLabel = new QLabel(QString("%1 %").arg(vol, 2, 10, QLatin1Char(' ')), m_mainWindow);
    volLevelLabel->setFont(m_fonts.smallFont);
    volLevelLabel->setFixedSize(fontMaxWidth, 14);
    volLevelLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

    int agcMaxGain = (int)set->getAGCMaximumGain_dB(0);
    agcGainSlider = new QSlider(Qt::Horizontal, m_mainWindow);
    agcGainSlider->setFixedSize(100, 14);
    agcGainSlider->setRange(-20, 120);
    agcGainSlider->setValue(agcMaxGain);
    CHECKED_CONNECT(agcGainSlider, &QSlider::valueChanged, m_mainWindow, qOverload<int>(&MainWindow::setAGCGain));

    agcGainLabel = new QLabel("AGC-G:", m_mainWindow);
    agcGainLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    
    fontMaxWidth = m_fonts.smallFontMetrics->boundingRect(" 120 dB").width();
    agcGainLevelLabel = new QLabel(QString(" %1 dB").arg(agcMaxGain, 2, 10, QLatin1Char(' ')), m_mainWindow);
    agcGainLevelLabel->setFont(m_fonts.smallFont);
    agcGainLevelLabel->setFixedSize(fontMaxWidth, 14);
    agcGainLevelLabel->setFrameStyle(QFrame::Box | QFrame::Raised);

    moxBtn = new AeroButton("MOX", m_mainWindow);
    moxBtn->setRoundness(10);
    moxBtn->setFont(m_fonts.normalFont);
    moxBtn->setTextColor(btnCol);
    moxBtn->setFixedSize(btn_width1, btn_height3);
    moxBtn->setEnabled(false);
    CHECKED_CONNECT(moxBtn, &AeroButton::clicked, m_mainWindow, &MainWindow::moxBtnClickedEvent);

    tunBtn = new AeroButton("Tune", m_mainWindow);
    tunBtn->setRoundness(10);
    tunBtn->setFont(m_fonts.normalFont);
    tunBtn->setTextColor(btnCol);
    tunBtn->setFixedSize(btn_width1, btn_height3);
    tunBtn->setEnabled(false);
    CHECKED_CONNECT(tunBtn, &AeroButton::clicked, m_mainWindow, &MainWindow::tunBtnClickedEvent);

    alexBtn = new AeroButton("Alex Auto", m_mainWindow);
    alexBtn->setRoundness(10);
    alexBtn->setFont(m_fonts.normalFont);
    alexBtn->setTextColor(btnCol);
    alexBtn->setFixedSize(btn_width1, btn_height3);
    alexBtn->setBtnState(AeroButton::ON);
    CHECKED_CONNECT(alexBtn, &AeroButton::clicked, m_mainWindow, &MainWindow::alexBtnClickedEvent);

    attenuatorBtn = new AeroButton("Attenuator", m_mainWindow);
    attenuatorBtn->setRoundness(10);
    attenuatorBtn->setFont(m_fonts.normalFont);
    attenuatorBtn->setTextColor(btnCol);
    attenuatorBtn->setFixedSize(btn_width1, btn_height1);

    muteBtn = new AeroButton("Mute", m_mainWindow);
    muteBtn->setRoundness(10);
    muteBtn->setFont(m_fonts.normalFont);
    muteBtn->setTextColor(btnCol);
    muteBtn->setFixedSize(btn_width3, btn_height1);
    CHECKED_CONNECT(muteBtn, &AeroButton::clicked, m_mainWindow, &MainWindow::muteBtnClickedEvent);

    lastFreqBtn = new AeroButton(" ", m_mainWindow);
    lastFreqBtn->setRoundness(10);
    lastFreqBtn->setFixedSize(btn_width1, btn_height3);
    CHECKED_CONNECT(lastFreqBtn, &AeroButton::clicked, m_mainWindow, &MainWindow::getLastFrequency);

    QHBoxLayout *firstBtnLayout = new QHBoxLayout;
    firstBtnLayout->setSpacing(0);
    firstBtnLayout->setContentsMargins(0,0,0,0);
    firstBtnLayout->addWidget(startBtn);
    firstBtnLayout->addWidget(serverBtn);
    firstBtnLayout->addWidget(setupBtn);
    firstBtnLayout->addWidget(wideBandBtn);
    firstBtnLayout->addWidget(nullBtn);
    firstBtnLayout->addWidget(plusRxBtn);
    firstBtnLayout->addWidget(viewBtn);
    firstBtnLayout->addWidget(modeBtn);
    firstBtnLayout->addWidget(quitBtn);

    QHBoxLayout* secondBtnLayout = new QHBoxLayout;
    secondBtnLayout->setSpacing(0);
    secondBtnLayout->setContentsMargins(0,0,0,0);
    secondBtnLayout->addWidget(moxBtn);
    secondBtnLayout->addWidget(tunBtn);
    secondBtnLayout->addStretch();
    secondBtnLayout->addWidget(alexBtn);
    secondBtnLayout->addWidget(attenuatorBtn);
    secondBtnLayout->addSpacing(5);
    secondBtnLayout->addWidget(drivelevellLabel);
    secondBtnLayout->addWidget(drivelevelSlider);
    secondBtnLayout->addSpacing(10);
    secondBtnLayout->addWidget(micGainLabel);
    secondBtnLayout->addWidget(micGainSlider);
    secondBtnLayout->addSpacing(10);
    secondBtnLayout->addWidget(agcGainLabel);
    secondBtnLayout->addWidget(agcGainSlider);
    secondBtnLayout->addWidget(agcGainLevelLabel);
    secondBtnLayout->addSpacing(10);
    secondBtnLayout->addWidget(volumeLabel);
    secondBtnLayout->addWidget(volumeSlider);
    secondBtnLayout->addWidget(volLevelLabel);
    secondBtnLayout->addSpacing(2);
    secondBtnLayout->addWidget(muteBtn);
    secondBtnLayout->addWidget(lastFreqBtn);

    QVBoxLayout* btnLayout = new QVBoxLayout;
    btnLayout->setSpacing(0);
    btnLayout->setContentsMargins(0,0,0,0);
    btnLayout->addLayout(firstBtnLayout);
    btnLayout->addLayout(secondBtnLayout);

    m_buttonWidget->setLayout(btnLayout);
    m_mainBtnToolBar->addWidget(m_buttonWidget);
    m_mainWindow->addToolBar(m_mainBtnToolBar);
}

void MainWindowUI::createModeMenu() {
    modeMenu = new QMenu(m_mainWindow);
    modeBtn->setMenu(modeMenu);
    QActionGroup* modeActionGroup = new QActionGroup(m_mainWindow);
    modeActionGroup->setExclusive(true);
    sdrModeAction = modeActionGroup->addAction(tr("SDR"));
    sdrModeAction->setCheckable(true);
    chirpWSPRAction = modeActionGroup->addAction(tr("ChirpWSPR"));
    modeMenu->addActions(modeActionGroup->actions());
    CHECKED_CONNECT(sdrModeAction, &QAction::triggered, m_mainWindow, &MainWindow::setSDRMode);
}

void MainWindowUI::createViewMenu() {
    viewMenu = new QMenu(m_mainWindow);
    viewBtn->setMenu(viewMenu);
}

void MainWindowUI::createAttenuatorMenu() {
    attenuatorMenu = new QMenu(m_mainWindow);
    attenuatorBtn->setMenu(attenuatorMenu);
    
    auto addAttnAction = [&](const QString& text, QList<QAction*>& list) {
        QAction* action = attenuatorMenu->addAction(text);
        action->setCheckable(true);
        list.append(action);
        CHECKED_CONNECT(action, &QAction::triggered, m_mainWindow, &MainWindow::setAttenuator);
        return action;
    };

    addAttnAction(tr("Step Att 0 dB"), mercuryAttnActionList);
    addAttnAction(tr("Step Att -10 dB"), mercuryAttnActionList);
    addAttnAction(tr("Step Att -20 dB"), mercuryAttnActionList);
    addAttnAction(tr("Step Att -30 dB"), mercuryAttnActionList);

    attenuatorMenu->addSeparator();

    addAttnAction(tr("Alex Attn 0 dB"), alexAttnActionList);
    addAttnAction(tr("Alex Attn -10 dB"), alexAttnActionList);
    addAttnAction(tr("Alex Attn -20 dB"), alexAttnActionList);
    addAttnAction(tr("Alex Attn -30 dB"), alexAttnActionList);
}
