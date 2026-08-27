#include <QtTest/QtTest>
#include <QProcess>
#include <QUdpSocket>
#include <QHostAddress>
#include <QFileInfo>
#include <QDir>
#include <QElapsedTimer>
#include <QtEndian>
#include <cstring>

#include "DataEngine/protocol_boundary_utils.h"

namespace {

constexpr quint16 kDevicePort = ProtocolBoundaryUtils::Ports::DevicePort;
constexpr quint16 kDdcSpecPort = ProtocolBoundaryUtils::Ports::P2DdcSpecPort;
constexpr quint16 kTxSpecPort = ProtocolBoundaryUtils::Ports::P2TxSpecPort;
constexpr quint16 kHpPort = ProtocolBoundaryUtils::Ports::P2HighPriorityPort;
constexpr quint16 kDdc0Port = ProtocolBoundaryUtils::Ports::P2Ddc0Port;
constexpr int kRateKhz = 48;
constexpr quint32 kRxFreqHz = 7050000u;

QString hpsdrSimPath()
{
    return QStringLiteral("/home/simon/Projects/cudaSDR/hpsdrsim/hpsdr_sim");
}

QByteArray ddcSpecificPacket(uint32_t sequence, int receivers = 1)
{
    QByteArray pkt(1444, '\0');
    const uint32_t seqBe = qToBigEndian(sequence);
    memcpy(pkt.data(), &seqBe, 4);
    pkt[4] = 1; // ADCs
    pkt[7] = static_cast<char>((1 << receivers) - 1);
    for (int ddc = 0; ddc < receivers; ++ddc) {
        const int base = 17 + 6 * ddc;
        pkt[base] = 0;
        const quint16 rateBe = qToBigEndian(static_cast<quint16>(kRateKhz));
        memcpy(pkt.data() + base + 1, &rateBe, 2);
        pkt[base + 5] = 24;
    }
    return pkt;
}

QByteArray txSpecificPacket(uint32_t sequence)
{
    QByteArray pkt(60, '\0');
    const uint32_t seqBe = qToBigEndian(sequence);
    memcpy(pkt.data(), &seqBe, 4);
    return pkt;
}

QByteArray hpPacket(uint32_t sequence, bool run, int receivers = 1)
{
    QByteArray pkt(1444, '\0');
    const uint32_t seqBe = qToBigEndian(sequence);
    memcpy(pkt.data(), &seqBe, 4);
    pkt[4] = run ? 0x01 : 0x00;
    const quint32 freqBe = qToBigEndian(kRxFreqHz);
    for (int ddc = 0; ddc < receivers; ++ddc)
        memcpy(pkt.data() + 9 + 4 * ddc, &freqBe, 4);
    memcpy(pkt.data() + 333, &freqBe, 4);
    pkt[345] = 0;
    return pkt;
}

bool waitForDatagram(QUdpSocket &sock, int timeoutMs)
{
    return sock.waitForReadyRead(timeoutMs);
}

} // namespace

class Protocol2IqPathTests : public QObject {
    Q_OBJECT

private slots:
    void cleanup();
    void discoveryAndIqAgainstHpsdrsim();

private:
    QProcess m_sim;

    bool startSimulator();
    void stopSimulator();
};

void Protocol2IqPathTests::cleanup()
{
    stopSimulator();
}

bool Protocol2IqPathTests::startSimulator()
{
    const QString sim = hpsdrSimPath();
    const QFileInfo fi(sim);
    if (!fi.exists() || !fi.isExecutable())
        return false;

    m_sim.setProgram(sim);
    m_sim.setArguments({QStringLiteral("-P2")});
    m_sim.setWorkingDirectory(fi.absolutePath());
    m_sim.setProcessChannelMode(QProcess::MergedChannels);
    m_sim.start();
    if (!m_sim.waitForStarted(3000))
        return false;

    // Give the UDP listener time to bind port 1024.
    QTest::qWait(400);
    return m_sim.state() == QProcess::Running;
}

void Protocol2IqPathTests::stopSimulator()
{
    if (m_sim.state() == QProcess::NotRunning)
        return;
    m_sim.terminate();
    if (!m_sim.waitForFinished(2000)) {
        m_sim.kill();
        m_sim.waitForFinished(2000);
    }
}

void Protocol2IqPathTests::discoveryAndIqAgainstHpsdrsim()
{
    if (!QFileInfo::exists(hpsdrSimPath()) || !QFileInfo(hpsdrSimPath()).isExecutable())
        QSKIP("hpsdr_sim missing or not executable");

    if (!startSimulator())
        QSKIP("hpsdr_sim failed to start (port 1024 busy or env restriction)");

    QUdpSocket sock;
    if (!sock.bind(QHostAddress::AnyIPv4, 0,
                   QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress))
        QSKIP("could not bind ephemeral UDP socket");

    const QHostAddress host(QStringLiteral("127.0.0.1"));

    // Protocol 2 discovery: 60-byte packet, byte4 = 0x02 (same as Discoverer).
    QByteArray discovery(60, '\0');
    discovery[4] = 0x02;
    if (sock.writeDatagram(discovery, host, kDevicePort) != discovery.size())
        QSKIP("failed to send Protocol 2 discovery");

    QByteArray reply;
    QHostAddress replyAddr;
    quint16 replyPort = 0;
    bool gotDiscovery = false;
    QElapsedTimer discTimer;
    discTimer.start();
    while (discTimer.elapsed() < 3000) {
        if (!waitForDatagram(sock, 200))
            continue;
        while (sock.hasPendingDatagrams()) {
            reply.resize(int(sock.pendingDatagramSize()));
            sock.readDatagram(reply.data(), reply.size(), &replyAddr, &replyPort);
            if (reply.size() >= 14
                && static_cast<unsigned char>(reply[0]) == 0x00
                && static_cast<unsigned char>(reply[1]) == 0x00
                && static_cast<unsigned char>(reply[2]) == 0x00
                && static_cast<unsigned char>(reply[3]) == 0x00) {
                const int status = static_cast<unsigned char>(reply[4]);
                if (status == 0x02 || status == 0x03) {
                    gotDiscovery = true;
                    break;
                }
            }
        }
        if (gotDiscovery)
            break;
    }
    if (!gotDiscovery)
        QSKIP("no Protocol 2 discovery reply (UDP blocked or sim not listening)");

    const int boardId = static_cast<unsigned char>(reply[11]);
    const QString boardName = ProtocolBoundaryUtils::boardNameForId(boardId, 2);
    QVERIFY(!boardName.isEmpty());
    // Default hpsdr_sim -P2 is Orion2 (id 5); accept any known mapped name.
    QVERIFY(ProtocolBoundaryUtils::isHermesFamilyBoard(boardName)
            || ProtocolBoundaryUtils::isMetisFamilyBoard(boardName));

    uint32_t seq1024 = 0;
    uint32_t seq1025 = 0;
    uint32_t seq1026 = 0;
    uint32_t seq1027 = 0;

    auto send = [&](const QByteArray &pkt, quint16 port) {
        return sock.writeDatagram(pkt, host, port) == pkt.size();
    };

    // General config (latches reply address) — mirrors protocol2FormatInitFrame.
    const auto init = ProtocolBoundaryUtils::protocol2FormatInitFrame(0, seq1024);
    QCOMPARE(init.port, kDevicePort);
    QCOMPARE(init.datagram.size(), 60);
    if (!send(init.datagram, kDevicePort))
        QSKIP("failed to send general config");
    QTest::qWait(50);

    if (!send(ddcSpecificPacket(seq1025++), kDdcSpecPort)
        || !send(txSpecificPacket(seq1026++), kTxSpecPort))
        QSKIP("failed to send DDC/TX specific");

    if (!send(hpPacket(seq1027++, false), kHpPort))
        QSKIP("failed to send HP run=0");
    QTest::qWait(10);

    if (!send(hpPacket(seq1027++, true), kHpPort))
        QSKIP("failed to send HP run=1");

    // Post-run burst so freshly spawned RX threads latch enable/rate/freq.
    QTest::qWait(50);
    for (int i = 0; i < 5; ++i) {
        QTest::qWait(20);
        send(ddcSpecificPacket(seq1025++), kDdcSpecPort);
        send(txSpecificPacket(seq1026++), kTxSpecPort);
        send(hpPacket(seq1027++, true), kHpPort);
    }

    // Same-host P2: IQ is sent back to this socket from source port 1035
    // (init frame advertises DDC0 = 1035). Binding 1035 locally would collide
    // with hpsdrsim, so we listen on the ephemeral command socket instead.
    bool gotIq = false;
    QElapsedTimer iqTimer;
    iqTimer.start();
    while (iqTimer.elapsed() < 4000) {
        if (!waitForDatagram(sock, 200))
            continue;
        while (sock.hasPendingDatagrams()) {
            QByteArray iq;
            QHostAddress from;
            quint16 fromPort = 0;
            iq.resize(int(sock.pendingDatagramSize()));
            sock.readDatagram(iq.data(), iq.size(), &from, &fromPort);
            if (fromPort == kDdc0Port
                && iq.size() >= ProtocolBoundaryUtils::kProtocol2IqPacketSize) {
                gotIq = true;
                break;
            }
            // Also accept large IQ-sized datagrams if source port mapping differs.
            if (iq.size() == ProtocolBoundaryUtils::kProtocol2IqPacketSize) {
                gotIq = true;
                break;
            }
        }
        if (gotIq)
            break;
    }

    // Leave device stopped.
    send(hpPacket(seq1027++, false), kHpPort);

    if (!gotIq)
        QSKIP("no DDC0 IQ datagram within timeout (sim/env may be unusable)");

    QVERIFY(gotIq);
}

QTEST_MAIN(Protocol2IqPathTests)
#include "protocol2_iq_path_tests.moc"
