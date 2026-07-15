#include "NetworkSettingsController.h"
#include "cusdr_settings.h"
#include "cusdr_networkWidget.h"

NetworkSettingsController::NetworkSettingsController(QObject* parent)
    : QObject(parent)
{
}

void NetworkSettingsController::bind(NetworkWidget* view, Settings* model)
{
    m_view = view;
    m_model = model;

    if (!m_view || !m_model) {
        return;
    }

    // --- 1. Populate initial View state from Model ---
    m_view->setHwInterface(m_model->getHWInterface());
    m_view->setSocketBufSize(m_model->getSocketBufferSize());
    m_view->setManualSocketBufferSize(m_model->getManualSocketBufferSize());
    m_view->setMetisCardsList(m_model->getMetisCardsList(), m_model->getCurrentMetisCard());
#ifdef HAVE_SOAPYSDR
    m_view->setSoapyDevicesList(m_model->getSoapyDeviceList(), m_model->getCurrentSoapyDevice());
#endif

    // --- 2. View -> Model (User interaction events) ---
    connect(m_view, &NetworkWidget::hwInterfaceModeRequested, this, [this](QSDR::_HWInterfaceMode mode) {
        if (m_model->getHWInterface() != mode) {
            m_model->setSystemState(QSDR::NoError, mode, m_model->getCurrentServerMode(), m_model->getDataEngineState());
        }
    });

    connect(m_view, &NetworkWidget::socketBufferSizeRequested, this, [this](int size) {
        if (m_model->getSocketBufferSize() != size) {
            m_model->setSocketBufferSize(size);
        }
    });

    connect(m_view, &NetworkWidget::manualSocketBufferSizeRequested, this, [this](bool manual) {
        if (m_model->getManualSocketBufferSize() != manual) {
            m_model->setManualSocketBufferSize(manual);
        }
    });

    connect(m_view, &NetworkWidget::nicInterfaceSelected, this, [this](int index) {
        m_model->setHPSDRDeviceNIC(index);
    });

    connect(m_view, &NetworkWidget::searchDevicesRequested, this, [this]() {
        m_model->searchDevices();
    });

    connect(m_view, &NetworkWidget::currentHpsdrDeviceSelected, this, [this](const TNetworkDevicecard& card) {
        m_model->setCurrentHPSDRDevice(card);
    });

#ifdef HAVE_SOAPYSDR
    connect(m_view, &NetworkWidget::currentSoapyDeviceSelected, this, [this](const TSoapyDevice& dev) {
        m_model->setCurrentSoapyDevice(dev);
    });
#endif

    // --- 3. Model -> View (Model update notifications) ---
    connect(m_model, &Settings::systemStateChanged, this, [this](QSDR::_Error, QSDR::_HWInterfaceMode mode, QSDR::_ServerMode, QSDR::_DataEngineState) {
        m_view->setHwInterface(mode);
    });

    connect(m_model, &Settings::socketBufferSizeChanged, this, [this](int size) {
        m_view->setSocketBufSize(size);
    });

    connect(m_model, &Settings::newHPSDRDeviceNIC, this, [this](const QString& name, const QString& ip) {
        m_view->addDeviceNICEntry(name, ip);
    });

    connect(m_model, &Settings::hpsdrDeviceNICChanged, this, [this](int index) {
        m_view->setDeviceNIC(index);
    });

    connect(m_model, &Settings::metisCardListChanged, this, [this](const QList<TNetworkDevicecard>& list) {
        m_view->setMetisCardsList(list, m_model->getCurrentMetisCard());
    });

    connect(m_model, &Settings::hpsdrNetworkDeviceChanged, this, [this](const TNetworkDevicecard& card) {
        m_view->setCurrentNetworkDevice(card);
    });

#ifdef HAVE_SOAPYSDR
    connect(m_model, &Settings::soapyDeviceListChanged, this, [this](const QList<TSoapyDevice>& list) {
        m_view->setSoapyDevicesList(list, m_model->getCurrentSoapyDevice());
    });

    connect(m_model, &Settings::soapyDeviceChanged, this, [this](const TSoapyDevice& dev) {
        m_view->setCurrentSoapyDevice(dev);
    });
#endif
}
