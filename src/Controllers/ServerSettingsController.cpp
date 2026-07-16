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
	
	quint16 tciPort = 50001;
	if (m_model->tciServer() && m_model->tciServer()->port() != 0) {
		tciPort = m_model->tciServer()->port();
	}
	m_view->setTciServerPort(tciPort);

	// 2. View -> Model (requests)
	connect(m_view, &ServerWidget::serverNICRequested, this, [this](int index) {
		m_model->setServerNetworkInterface(index);
	});

	connect(m_view, &ServerWidget::tciServerEnabledRequested, this, [this](bool enabled) {
		m_model->setTciServerEnabled(enabled);
	});

	// 3. Model -> View (updates)
	connect(m_model, &Settings::newServerNetworkInterface, m_view, &ServerWidget::addServerNIEntry);
	connect(m_model, &Settings::serverNICChanged, m_view, &ServerWidget::setServerNIC);
	connect(m_model, &Settings::tciServerEnabledChanged, m_view, &ServerWidget::setTciServerEnabled);
}
