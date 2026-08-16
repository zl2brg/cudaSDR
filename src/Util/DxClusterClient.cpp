#include "DxClusterClient.h"

#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>

DxClusterClient::DxClusterClient(QObject *parent)
    : QObject(parent)
    , m_tcpSocket(new QTcpSocket(this))
    , m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_reconnectTimer(new QTimer(this))
{
    // TCP / Telnet Socket Connections
    connect(m_tcpSocket, &QTcpSocket::connected, this, &DxClusterClient::onTcpConnected);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &DxClusterClient::onTcpDisconnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &DxClusterClient::onTcpReadyRead);
    connect(m_tcpSocket, &QTcpSocket::errorOccurred, this, &DxClusterClient::onTcpError);

    // WebSocket Connections
    connect(m_webSocket, &QWebSocket::connected, this, &DxClusterClient::onWsConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &DxClusterClient::onWsDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &DxClusterClient::onWsTextMessageReceived);
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &DxClusterClient::onWsError);

    // Reconnection Timer
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &DxClusterClient::onReconnectTimeout);
}

DxClusterClient::~DxClusterClient()
{
    disconnectFromCluster();
}

void DxClusterClient::setCallsign(const QString &callsign)
{
    m_callsign = callsign.trimmed().toUpper();
}

void DxClusterClient::connectTelnet(const QString &host, quint16 port, const QString &callsign)
{
    const QString targetHost = host.trimmed();
    const quint16 targetPort = port > 0 ? port : 7000;
    const QString targetCall = callsign.trimmed().toUpper();

    if ((m_state == Connected || m_state == Connecting || m_state == WaitingForLogin)
        && m_type == Telnet
        && m_host == targetHost
        && m_port == targetPort
        && (targetCall.isEmpty() || m_callsign == targetCall)) {
        return; // Already connected or connecting to this host
    }

    disconnectFromCluster();
    m_type = Telnet;
    m_host = targetHost;
    m_port = targetPort;
    if (!targetCall.isEmpty())
        m_callsign = targetCall;

    m_userInitiatedDisconnect = false;
    setState(Connecting);
    qDebug() << "DxClusterClient: Connecting Telnet to" << m_host << ":" << m_port << "as" << m_callsign;
    m_tcpSocket->connectToHost(m_host, m_port);
}

void DxClusterClient::connectWebSocket(const QUrl &url, const QString &callsign)
{
    const QString targetCall = callsign.trimmed().toUpper();
    if ((m_state == Connected || m_state == Connecting || m_state == WaitingForLogin)
        && m_type == WebSocket
        && m_url == url
        && (targetCall.isEmpty() || m_callsign == targetCall)) {
        return; // Already connected or connecting to this WebSocket
    }

    disconnectFromCluster();
    m_type = WebSocket;
    m_url = url;
    if (!targetCall.isEmpty())
        m_callsign = targetCall;

    m_userInitiatedDisconnect = false;
    setState(Connecting);
    qDebug() << "DxClusterClient: Connecting WebSocket to" << m_url.toString() << "as" << m_callsign;
    m_webSocket->open(m_url);
}

void DxClusterClient::disconnectFromCluster()
{
    m_userInitiatedDisconnect = true;
    m_reconnectTimer->stop();

    if (m_tcpSocket && m_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        m_tcpSocket->disconnectFromHost();
        if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState)
            m_tcpSocket->abort();
    }

    if (m_webSocket && m_webSocket->state() != QAbstractSocket::UnconnectedState) {
        m_webSocket->close();
    }

    m_incomingBuffer.clear();
    setState(Disconnected);
}

void DxClusterClient::sendCommand(const QString &cmd)
{
    const QString formatted = cmd.trimmed() + QStringLiteral("\r\n");
    if (m_type == Telnet && m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_tcpSocket->write(formatted.toUtf8());
        m_tcpSocket->flush();
    } else if (m_type == WebSocket && m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState) {
        m_webSocket->sendTextMessage(cmd.trimmed());
    }
}

void DxClusterClient::setState(State state)
{
    if (m_state == state)
        return;

    m_state = state;
    emit stateChanged(m_state);

    if (m_state == Connected)
        emit connected();
    else if (m_state == Disconnected)
        emit disconnected();
}

void DxClusterClient::onTcpConnected()
{
    qDebug() << "DxClusterClient: TCP connected to" << m_host << ":" << m_port;
    setState(WaitingForLogin);
}

void DxClusterClient::onTcpDisconnected()
{
    qDebug() << "DxClusterClient: TCP disconnected";
    setState(Disconnected);

    if (m_autoReconnect && !m_userInitiatedDisconnect) {
        qDebug() << "DxClusterClient: Scheduling reconnect in" << m_reconnectIntervalMs << "ms";
        m_reconnectTimer->start(m_reconnectIntervalMs);
    }
}

void DxClusterClient::onTcpError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    const QString errorStr = m_tcpSocket->errorString();
    qWarning() << "DxClusterClient TCP error:" << errorStr;
    emit errorOccurred(errorStr);
}

void DxClusterClient::onTcpReadyRead()
{
    m_incomingBuffer.append(m_tcpSocket->readAll());

    // Check for login prompt in buffer if waiting for login
    if (m_state == WaitingForLogin || m_state == Connecting) {
        handleLoginPrompt(QString::fromUtf8(m_incomingBuffer));
    }

    // Process complete newline-terminated lines
    int newlineIndex = -1;
    while ((newlineIndex = m_incomingBuffer.indexOf('\n')) != -1) {
        const QByteArray rawLine = m_incomingBuffer.left(newlineIndex);
        m_incomingBuffer.remove(0, newlineIndex + 1);

        QString line = QString::fromUtf8(rawLine).trimmed();
        if (!line.isEmpty()) {
            processLine(line);
        }
    }
}

void DxClusterClient::onWsConnected()
{
    qDebug() << "DxClusterClient: WebSocket connected to" << m_url.toString();
    setState(WaitingForLogin);
    if (!m_callsign.isEmpty()) {
        sendCommand(m_callsign);
        setState(Connected);
    }
}

void DxClusterClient::onWsDisconnected()
{
    qDebug() << "DxClusterClient: WebSocket disconnected";
    setState(Disconnected);

    if (m_autoReconnect && !m_userInitiatedDisconnect) {
        m_reconnectTimer->start(m_reconnectIntervalMs);
    }
}

void DxClusterClient::onWsTextMessageReceived(const QString &message)
{
    const QStringList lines = message.split(QLatin1Char('\n'));
    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (!line.isEmpty()) {
            processLine(line);
        }
    }
}

void DxClusterClient::onWsError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    const QString errorStr = m_webSocket->errorString();
    qWarning() << "DxClusterClient WebSocket error:" << errorStr;
    emit errorOccurred(errorStr);
}

void DxClusterClient::onReconnectTimeout()
{
    if (m_state == Disconnected && !m_userInitiatedDisconnect) {
        if (m_type == Telnet && !m_host.isEmpty()) {
            connectTelnet(m_host, m_port, m_callsign);
        } else if (m_type == WebSocket && m_url.isValid()) {
            connectWebSocket(m_url, m_callsign);
        }
    }
}

void DxClusterClient::handleLoginPrompt(const QString &text)
{
    static const QRegularExpression promptRegex(
        QStringLiteral("(?:call(?:sign)?|login|enter your call(?:sign)?|user(?:name)?):\\s*$"),
        QRegularExpression::CaseInsensitiveOption);

    if (promptRegex.match(text).hasMatch() || text.endsWith(QLatin1Char(':')) || text.endsWith(QLatin1String(": "))) {
        const QString callToSend = m_callsign.isEmpty() ? QStringLiteral("NOCALL") : m_callsign;
        qDebug() << "DxClusterClient: Detected login prompt, sending callsign:" << callToSend;
        sendCommand(callToSend);
        setState(Connected);
        m_incomingBuffer.clear();
    }
}

void DxClusterClient::processLine(const QString &line)
{
    emit rawLineReceived(line);

    // If we receive a DX spot line while in WaitingForLogin, mark as connected
    if (m_state == WaitingForLogin && line.startsWith(QLatin1String("DX de"), Qt::CaseInsensitive)) {
        setState(Connected);
    }

    qint64 freqHz = 0;
    QString dxCall;
    QString mode;
    int snr = 0;
    int wpm = 0;
    QString spotter;
    QString comment;
    QString timeUtc;

    if (parseDxLine(line, freqHz, dxCall, mode, snr, wpm, spotter, comment, timeUtc)) {
        emit spotReceived(freqHz, dxCall, mode, snr, wpm, spotter, comment);
    }
}

bool DxClusterClient::parseDxLine(const QString &line,
                                  qint64 &freqHz,
                                  QString &dxCall,
                                  QString &mode,
                                  int &snr,
                                  int &wpm,
                                  QString &spotter,
                                  QString &comment,
                                  QString &timeUtc)
{
    if (!line.startsWith(QLatin1String("DX de"), Qt::CaseInsensitive))
        return false;

    // Pattern 1: RBN Skimmer Format
    // e.g. "DX de K3LR-#:    14025.1  ZL2BRG         CW    28 dB  24 WPM  CQ    1420Z"
    // e.g. "DX de VE7CC-#:   14074.0  JA1ABC         FT8   -08 dB         CQ    1422Z"
    static const QRegularExpression rbnRegex(
        QStringLiteral(R"(^DX\s+de\s+([A-Za-z0-9/#\-]+):\s+([0-9.]+)\s+([A-Za-z0-9/]+)\s+([A-Za-z0-9]+)\s+([+-]?[0-9]+)\s*dB(?:\s+([0-9]+)\s*WPM)?\s*(.*?)\s+([0-9]{4}Z?)$)"),
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatch match = rbnRegex.match(line);
    if (match.hasMatch()) {
        spotter = match.captured(1).trimmed();
        const double freqKHz = match.captured(2).toDouble();
        freqHz = qRound64(freqKHz * 1000.0);
        dxCall = match.captured(3).trimmed().toUpper();
        mode = match.captured(4).trimmed().toUpper();
        snr = match.captured(5).toInt();
        wpm = match.captured(6).isEmpty() ? 0 : match.captured(6).toInt();
        comment = match.captured(7).trimmed();
        timeUtc = match.captured(8).trimmed();
        return freqHz > 0 && !dxCall.isEmpty();
    }

    // Pattern 2: Standard Telnet DX Cluster Format
    // e.g. "DX de OH2AQ:     14195.0  VK9WA          up 5-10                1423Z"
    // e.g. "DX de K1TTT:     21085.0  ZS6CC          RTTY contest           1424Z"
    static const QRegularExpression classicRegex(
        QStringLiteral(R"(^DX\s+de\s+([A-Za-z0-9/#\-]+):\s+([0-9.]+)\s+([A-Za-z0-9/]+)\s+(.*?)\s+([0-9]{4}Z?)$)"),
        QRegularExpression::CaseInsensitiveOption);

    match = classicRegex.match(line);
    if (match.hasMatch()) {
        spotter = match.captured(1).trimmed();
        const double freqKHz = match.captured(2).toDouble();
        freqHz = qRound64(freqKHz * 1000.0);
        dxCall = match.captured(3).trimmed().toUpper();
        comment = match.captured(4).trimmed();
        timeUtc = match.captured(5).trimmed();
        snr = 0;
        wpm = 0;

        // Try extracting mode from comment if present
        const QString upperComment = comment.toUpper();
        if (upperComment.contains(QStringLiteral("CW")))
            mode = QStringLiteral("CW");
        else if (upperComment.contains(QStringLiteral("FT8")))
            mode = QStringLiteral("FT8");
        else if (upperComment.contains(QStringLiteral("FT4")))
            mode = QStringLiteral("FT4");
        else if (upperComment.contains(QStringLiteral("RTTY")))
            mode = QStringLiteral("RTTY");
        else if (upperComment.contains(QStringLiteral("PSK")))
            mode = QStringLiteral("PSK");
        else if (upperComment.contains(QStringLiteral("SSB")) || upperComment.contains(QStringLiteral("USB")) || upperComment.contains(QStringLiteral("LSB")))
            mode = QStringLiteral("SSB");
        else
            mode = QString();

        return freqHz > 0 && !dxCall.isEmpty();
    }

    return false;
}
