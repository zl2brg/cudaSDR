/**
* @file  cusdr_rigctlserver.cpp
* @brief rigctld-compatible TCP server for WSJT-X / third-party rig control
*
* Implements a subset of the rigctld text protocol sufficient for WSJT-X:
*   f          - get frequency (Hz)
*   F <hz>     - set VFO frequency
*   m          - get mode + passband
*   M <mode> <bw> - set mode
*   t          - get PTT state
*   T <0|1>    - set PTT
*   v          - get VFO name
*   V <vfo>    - set VFO (no-op, always VFOA)
*   s          - get split state (always 0)
*   S <0|1> <vfo> - set split (no-op)
*   _          - get rig model name
*   1          - dump_caps (minimal)
*   q / Q      - quit
*/

#include "cusdr_rigctlserver.h"
#include "cusdr_settings.h"
#include "cusdr_hamDatabase.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QStringList>
#include <QDebug>

RigCtlServer::RigCtlServer(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_watchdog(new QTimer(this))
    , m_settings(Settings::instance())
{
    connect(m_server, &QTcpServer::newConnection, this, &RigCtlServer::onNewConnection);

    m_watchdog->setSingleShot(true);
    m_watchdog->setInterval(WATCHDOG_TIMEOUT_MS);
    connect(m_watchdog, &QTimer::timeout, this, &RigCtlServer::onWatchdogTimeout);
}

RigCtlServer::~RigCtlServer()
{
    stopListening();
}

bool RigCtlServer::startListening(quint16 port)
{
    if (m_server->isListening())
        return true;

    if (!m_server->listen(QHostAddress::AnyIPv4, port)) {
        RIGCTL_WARN << "Failed to listen on port" << port << ":" << m_server->errorString();
        return false;
    }
    RIGCTL_DEBUG << "Listening on port" << port;
    return true;
}

void RigCtlServer::stopListening()
{
    for (QTcpSocket *client : std::as_const(m_clients)) {
        client->disconnectFromHost();
    }
    m_clients.clear();
    m_server->close();
}

bool RigCtlServer::isListening() const
{
    return m_server->isListening();
}

quint16 RigCtlServer::port() const
{
    return m_server->serverPort();
}

void RigCtlServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *client = m_server->nextPendingConnection();
        connect(client, &QTcpSocket::readyRead, this, &RigCtlServer::onClientReadyRead);
        connect(client, &QTcpSocket::disconnected, this, &RigCtlServer::onClientDisconnected);
        const bool wasEmpty = m_clients.isEmpty();
        m_clients.append(client);
        m_watchdog->stop(); // cancel any pending watchdog on reconnect
        RIGCTL_DEBUG << "Client connected from" << client->peerAddress().toString();
        if (wasEmpty)
            emit remoteControlChanged(true);
    }
}

void RigCtlServer::onClientReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (!client)
        return;

    // Process complete lines first
    while (client->canReadLine()) {
        QString line = QString::fromLatin1(client->readLine()).trimmed();
        if (line.isEmpty())
            continue;

        RIGCTL_DEBUG << "RX:" << line;
        QString response = processCommand(line);
        RIGCTL_DEBUG << "TX:" << response;
        client->write((response + "\n").toLatin1());
    }

    // Fallback: handle data with no trailing newline (e.g. bare 'f' with no \n)
    if (client->bytesAvailable() > 0) {
        QString line = QString::fromLatin1(client->readAll()).trimmed();
        if (!line.isEmpty()) {
            RIGCTL_DEBUG << "RX (no-newline):" << line;
            QString response = processCommand(line);
            RIGCTL_DEBUG << "TX:" << response;
            client->write((response + "\n").toLatin1());
        }
    }
}

void RigCtlServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (!client)
        return;

    RIGCTL_DEBUG << "Client disconnected";
    m_clients.removeAll(client);
    client->deleteLater();

    if (m_clients.isEmpty()) {
        emit remoteControlChanged(false);
        // If TX is active with no client, start watchdog
        if (m_settings->getRadioState() != RadioState::RX) {
            RIGCTL_WARN << "Client disconnected while TX active — watchdog started (30s)";
            m_watchdog->start();
        }
    }
}

void RigCtlServer::onWatchdogTimeout()
{
    RIGCTL_WARN << "Watchdog timeout: client disconnected while TX, forcing RX";
    m_settings->setRadioState(RadioState::RX);
}

// ---------------------------------------------------------------------------
// Command dispatch
// ---------------------------------------------------------------------------

QString RigCtlServer::processCommand(const QString &cmd)
{
    // rigctld protocol: single-char ops AND long-form \command_name ops.
    // WSJT-X uses long-form exclusively; normalize both to canonical short ops.
    const QStringList parts = cmd.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty())
        return "RPRT 0";

    static const QHash<QString, QString> longFormMap = {
        {"\\get_freq",       "f"},
        {"\\set_freq",       "F"},
        {"\\get_mode",       "m"},
        {"\\set_mode",       "M"},
        {"\\get_ptt",        "t"},
        {"\\set_ptt",        "T"},
        {"\\get_vfo",        "v"},
        {"\\set_vfo",        "V"},
        {"\\get_split_vfo",  "s"},
        {"\\set_split_vfo",  "S"},
        {"\\dump_caps",      "1"},
        {"\\get_powerstat",  "_pwr"},
        {"\\chk_vfo",        "_chkvfo"},
        {"\\dump_state",     "_dumpstate"},
    };

    const QString rawOp = parts.at(0);
    const QString op = rawOp.startsWith('\\') ? longFormMap.value(rawOp, rawOp) : rawOp;

    // --- f / \get_freq : get frequency ---
    if (op == "f") {
        long freq = m_settings->getVfoFrequency(0);
        return QString::number(freq) + "\nRPRT 0";
    }

    // --- F / \set_freq <hz> : set frequency ---
    if (op == "F") {
        if (parts.size() < 2) {
            RIGCTL_WARN << "set_freq: missing frequency argument, cmd was:" << cmd;
            return "RPRT -1";
        }
        bool ok = false;
        long freq = static_cast<long>(parts.at(1).toDouble(&ok));
        RIGCTL_DEBUG << "set_freq: raw arg=" << parts.at(1) << "parsed=" << freq << "ok=" << ok;
        if (!ok || freq <= 0) {
            RIGCTL_WARN << "set_freq: invalid frequency value:" << parts.at(1);
            return "RPRT -1";
        }
        long prev = m_settings->getVfoFrequency(0);
        m_settings->setVFOFrequency(this, 0, 0, freq);
        RIGCTL_DEBUG << "set_freq: changed" << prev << "->" << freq << "Hz";
        return "RPRT 0";
    }

    // --- m / \get_mode : get mode ---
    if (op == "m") {
        DSPMode mode = m_settings->getDSPMode(0);
        QString modeStr = dspModeToRigctlMode(static_cast<int>(mode));
        return modeStr + "\n3000\nRPRT 0";
    }

    // --- M / \set_mode <mode> [bw] : set mode ---
    if (op == "M") {
        if (parts.size() < 2)
            return "RPRT -1";
        int dspMode = rigctlModeToDsp(parts.at(1));
        if (dspMode < 0)
            return "RPRT -1";
        m_settings->setDSPMode(nullptr, 0, static_cast<DSPMode>(dspMode));
        return "RPRT 0";
    }

    // --- t / \get_ptt : get PTT ---
    if (op == "t") {
        int ptt = (m_settings->getRadioState() == RadioState::RX) ? 0 : 1;
        return QString::number(ptt) + "\nRPRT 0";
    }

    // --- T / \set_ptt <0|1> : set PTT ---
    if (op == "T") {
        if (parts.size() < 2)
            return "RPRT -1";
        bool ok = false;
        int val = parts.at(1).toInt(&ok);
        if (!ok)
            return "RPRT -1";
        if (val == 0)
            m_watchdog->stop(); // TX off — no need for watchdog
        m_settings->setRadioState(val ? RadioState::MOX : RadioState::RX);
        return "RPRT 0";
    }

    // --- v / \get_vfo : get VFO ---
    if (op == "v") {
        return "VFOA\nRPRT 0";
    }

    // --- V / \set_vfo : set VFO (no-op) ---
    if (op == "V") {
        return "RPRT 0";
    }

    // --- s / \get_split_vfo : get split ---
    if (op == "s") {
        return "0\nVFOA\nRPRT 0";
    }

    // --- S / \set_split_vfo : set split (no-op) ---
    if (op == "S") {
        return "RPRT 0";
    }

    // --- _ : get rig info ---
    if (op == "_") {
        return "Model name:\tcudaSDR\nRPRT 0";
    }

    // --- 1 / \dump_caps : minimal caps ---
    if (op == "1") {
        return "Caps dump for model:\t2\n"
               "Model name:\tcudaSDR\n"
               "Mfg name:\tcudaSDR\n"
               "Backend version:\t0.1\n"
               "Backend copyright:\t\n"
               "Backend status:\tAlpha\n"
               "Rig type:\tOther\n"
               "PTT type:\tRig CAT\n"
               "DCD type:\tNone\n"
               "Port type:\tNetwork\n"
               "Write delay:\t0mS\n"
               "Post Write delay:\t0mS\n"
               "Timeout:\t0mS\n"
               "Retry:\t0\n"
               "Get functions:\n"
               "Set functions:\n"
               "Extra settings:\n"
               "Get level:\n"
               "Set level:\n"
               "Get parm:\n"
               "Set parm:\n"
               "VFOS: VFOA\n"
               "Number of bands:\t0\n"
               "Number of channels:\t0\n"
               "Memory name desc size:\t0\n"
               "Preamp steps:\n"
               "Attenuator steps:\n"
               "Has targetable VFO: N\n"
               "Has transceive: N\n"
               "Announce:\n"
               "Max RIT: 0 Hz\n"
               "Max XIT: 0 Hz\n"
               "Max IF-SHIFT: 0 Hz\n"
               "Preamp steps:\n"
               "Attenuator steps:\n"
               "RPRT 0";
    }

    // --- \get_powerstat : rig is always powered on ---
    if (op == "_pwr") {
        return "1\nRPRT 0";
    }

    // --- \chk_vfo : no per-VFO addressing ---
    if (op == "_chkvfo") {
        return "0\nRPRT 0";
    }

    // --- \dump_state : full rig state — required by hamlib's netrigctl_open() ---
    if (op == "_dumpstate") {
        return dumpState();
    }

    // --- q / Q : quit ---
    if (op == "q" || op == "Q") {
        return "RPRT 0";
    }

    // Unknown command
    RIGCTL_WARN << "Unknown command:" << cmd;
    return "RPRT -1";
}

// ---------------------------------------------------------------------------
// Rig state dump (hamlib netrigctl_open protocol)
// Format: protocol_ver, model, type, rx_ranges, tx_ranges, tuning_steps,
//         filters, rit/xit/ifshift, announces, preamp, att, func/level/parm caps
// ---------------------------------------------------------------------------

QString RigCtlServer::dumpState() const
{
    return
        "0\n"                                        // protocol version
        "2\n"                                        // rig model (NET rigctl)
        "2\n"                                        // rig type: transceiver
        // RX frequency ranges (100 kHz – 61.44 MHz, all modes, VFOA, ant1)
        "100000 61440000 0xffff -1 -1 0x1 0x1\n"
        "0 0 0 0 0 0 0\n"                            // RX range terminator
        // TX frequency ranges
        "1800000 29700000 0xffff 5 100 0x1 0x1\n"
        "0 0 0 0 0 0 0\n"                            // TX range terminator
        // Tuning steps (1 Hz, all modes)
        "1 0xffff\n"
        "0 0\n"                                      // tuning step terminator
        // Filters (none declared — hamlib uses defaults)
        "0 0\n"                                      // filter terminator
        "0\n"                                        // max_rit
        "0\n"                                        // max_xit
        "0\n"                                        // max_ifshift
        "0\n"                                        // announces
        " 0\n"                                       // preamp list (none)
        " 0\n"                                       // attenuator list (none)
        "0x0\n"                                      // has_get_func
        "0x0\n"                                      // has_set_func
        "0x0\n"                                      // has_get_level
        "0x0\n"                                      // has_set_level
        "0x0\n"                                      // has_get_parm
        "0x0\n"                                      // has_set_parm
        "RPRT 0";
}

// ---------------------------------------------------------------------------
// Mode mapping
// ---------------------------------------------------------------------------

QString RigCtlServer::dspModeToRigctlMode(int dspMode) const
{
    switch (static_cast<DSPMode>(dspMode)) {
        case LSB:  return "LSB";
        case USB:  return "USB";
        case DSB:  return "DSB";
        case CWL:  return "CWR";
        case CWU:  return "CW";
        case FMN:  return "FM";
        case AM:   return "AM";
        case DIGU: return "PKTUSB";
        case DIGL: return "PKTLSB";
        case SAM:  return "AMS";
        default:   return "USB";
    }
}

int RigCtlServer::rigctlModeToDsp(const QString &mode) const
{
    const QString m = mode.toUpper();
    if (m == "LSB")    return LSB;
    if (m == "USB")    return USB;
    if (m == "DSB")    return DSB;
    if (m == "CWR")    return CWL;
    if (m == "CW")     return CWU;
    if (m == "FM")     return FMN;
    if (m == "AM")     return AM;
    if (m == "AMS")    return SAM;
    if (m == "PKTUSB") return DIGU;
    if (m == "PKTLSB") return DIGL;
    return -1;
}
