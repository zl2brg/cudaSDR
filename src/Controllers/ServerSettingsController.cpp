#include "ServerSettingsController.h"
#include "cusdr_serverWidget.h"
#include "cusdr_settings.h"
#include "Util/cusdr_tciserver.h"

ServerSettingsController::ServerSettingsController(QObject* parent)
    : QObject(parent)
{
}

void ServerSettingsController::bind(ServerWidget* view, Settings* model)
{
	m_view = view;
	m_model = model;

	if (!m_view || !m_model) {
		return;
	}

	// 1. Initial setup loading
	m_view->setPorts(m_model->getServerPort(), m_model->getListenPort(), m_model->getAudioPort());
	m_view->setTciServerEnabled(m_model->getTciServerEnabled());
	m_view->setTciRxGain(m_model->getTciRxGain());
	m_view->setTciTxGain(m_model->getTciTxGain());

	quint16 tciPort = 50001;
	TciServer *tci = m_model->tciServer();
	if (tci && tci->port() != 0) {
		tciPort = tci->port();
	}
	m_view->setTciServerPort(tciPort);

	if (tci) {
		tci->setRxGain(m_model->getTciRxGain());
		tci->setTxGain(m_model->getTciTxGain());
		m_view->setTciConnectionStatus(tci->connectionStatusText());
	} else {
		m_view->setTciConnectionStatus(
			m_model->getTciServerEnabled() ? QStringLiteral("Not listening")
			                               : QStringLiteral("Disabled"));
	}

	// 2. View -> Model (requests)
	connect(m_view, &ServerWidget::serverNICRequested, this, [this](int index) {
		m_model->setServerNetworkInterface(index);
	});

	connect(m_view, &ServerWidget::tciServerEnabledRequested, this, [this](bool enabled) {
		m_model->setTciServerEnabled(enabled);
	});

	connect(m_view, &ServerWidget::tciRxGainRequested, this, [this](float gain) {
		m_model->setTciRxGain(gain);
	});

	connect(m_view, &ServerWidget::tciTxGainRequested, this, [this](float gain) {
		m_model->setTciTxGain(gain);
	});

	// 3. Model -> View (updates)
	connect(m_model, &Settings::newServerNetworkInterface, m_view, &ServerWidget::addServerNIEntry);
	connect(m_model, &Settings::serverNICChanged, m_view, &ServerWidget::setServerNIC);
	connect(m_model, &Settings::tciServerEnabledChanged, m_view, &ServerWidget::setTciServerEnabled);
	connect(m_model, &Settings::tciRxGainChanged, m_view, &ServerWidget::setTciRxGain);
	connect(m_model, &Settings::tciTxGainChanged, m_view, &ServerWidget::setTciTxGain);

	connect(m_model, &Settings::tciRxGainChanged, this, [this](float gain) {
		if (TciServer *server = m_model->tciServer())
			server->setRxGain(gain);
	});
	connect(m_model, &Settings::tciTxGainChanged, this, [this](float gain) {
		if (TciServer *server = m_model->tciServer())
			server->setTxGain(gain);
	});
	connect(m_model, &Settings::tciServerEnabledChanged, this, [this](bool) {
		if (TciServer *server = m_model->tciServer())
			m_view->setTciConnectionStatus(server->connectionStatusText());
	});

	if (tci) {
		connect(tci, &TciServer::connectionStatusChanged, this, [this]() {
			if (TciServer *server = m_model->tciServer())
				m_view->setTciConnectionStatus(server->connectionStatusText());
		});
	}
}
