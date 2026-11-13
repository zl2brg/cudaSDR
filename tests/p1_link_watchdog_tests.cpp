#include <QtTest/QtTest>
#include <QProcess>
#include <QUdpSocket>
#include <QHostAddress>
#include <QFileInfo>
#include <QDir>
#include <QElapsedTimer>
#include <QtEndian>
#include <cstring>
#include <csignal>
#include <sys/types.h>

#include "DataEngine/P1LinkWatchdog.h"

namespace {

QString hpsdrSimPath()
{
    const QString env = qEnvironmentVariable("HPSDR_SIM_PATH");
    if (!env.isEmpty())
        return env;
    const QString relative = QDir::current().filePath(QStringLiteral("hpsdrsim/hpsdr_sim"));
    if (QFileInfo::exists(relative))
        return relative;
#ifdef HPSDR_SIM_DEFAULT_PATH
    return QStringLiteral(HPSDR_SIM_DEFAULT_PATH);
#else
    return QString();
#endif
}

QByteArray p1StartStop(char startByte)
{
    QByteArray pkt(64, '\0');
    pkt[0] = char(0xEF);
    pkt[1] = char(0xFE);
    pkt[2] = 0x04;
    pkt[3] = startByte;
    return pkt;
}

// Minimal 1032-byte host packet: 48 kHz, one receiver, RX, silent TX IQ.
QByteArray p1HostPacket(quint32 sequence)
{
    QByteArray pkt(1032, '\0');
    pkt[0] = char(0xEF);
    pkt[1] = char(0xFE);
    pkt[2] = 0x01;
    pkt[3] = 0x02;
    const quint32 seqBe = qToBigEndian(sequence);
    memcpy(pkt.data() + 4, &seqBe, 4);
    for (int frame = 0; frame < 2; ++frame) {
        char* f = pkt.data() + 8 + 512 * frame;
        f[0] = 0x7F;
        f[1] = 0x7F;
        f[2] = 0x7F;
    }
    return pkt;
}

bool isEp6(const QByteArray& d)
{
    return d.size() == 1032
        && quint8(d[0]) == 0xEF && quint8(d[1]) == 0xFE
        && d[2] == 0x01 && d[3] == 0x06;
}

struct HostRunResult {
    int  packetsBeforeDropout = 0;
    int  packetsInFinalSecond = 0;
    int  restarts = 0;
    bool resumed = false;
};

} // namespace

class P1LinkWatchdogTests : public QObject {
    Q_OBJECT

private slots:
    void cleanup();

    void quietUntilStallThreshold();
    void firstPacketIsReported();
    void startNeverAnsweredRetriesAfterRetryInterval();
    void midStreamStallResendsImmediatelyThenBacksOff();
    void resumeClearsStallAndReportsRestarts();

    void simWithoutWatchdogStaysFrozen();
    void simWithWatchdogRecovers();

private:
    QProcess m_sim;

    void requireSimulator();
    QByteArray simOutput();
    HostRunResult runRxPacedHost(bool useWatchdog);
};

QByteArray P1LinkWatchdogTests::simOutput()
{
    while (m_sim.waitForReadyRead(200)) {
    }
    return m_sim.readAll();
}

void P1LinkWatchdogTests::cleanup()
{
    if (m_sim.state() == QProcess::NotRunning)
        return;
    m_sim.terminate();
    if (!m_sim.waitForFinished(2000)) {
        m_sim.kill();
        m_sim.waitForFinished(2000);
    }
}

void P1LinkWatchdogTests::quietUntilStallThreshold()
{
    P1LinkWatchdog wd(300, 1000);
    wd.reset(0);
    QVERIFY(!wd.isSilent(299));
    QVERIFY(!wd.shouldResendStart(299));
    QVERIFY(!wd.stalled());
}

void P1LinkWatchdogTests::firstPacketIsReported()
{
    P1LinkWatchdog wd(300, 1000);
    wd.reset(0);
    const auto first = wd.onIqPacket(50);
    QVERIFY(first.first);
    QVERIFY(!first.resumed);
    QCOMPARE(first.gapMs, qint64(50));
    QVERIFY(wd.streaming());
    QVERIFY(!wd.onIqPacket(53).first);
}

void P1LinkWatchdogTests::startNeverAnsweredRetriesAfterRetryInterval()
{
    P1LinkWatchdog wd(300, 1000);
    wd.reset(0);
    QVERIFY(!wd.shouldResendStart(300));
    QVERIFY(wd.stalled());
    QVERIFY(!wd.shouldResendStart(999));
    QVERIFY(wd.shouldResendStart(1000));
    QCOMPARE(wd.restarts(), 1);
    QVERIFY(!wd.shouldResendStart(1999));
    QVERIFY(wd.shouldResendStart(2000));
    QCOMPARE(wd.restarts(), 2);
}

void P1LinkWatchdogTests::midStreamStallResendsImmediatelyThenBacksOff()
{
    P1LinkWatchdog wd(300, 1000);
    wd.reset(0);
    for (qint64 t = 3; t <= 5000; t += 3)
        wd.onIqPacket(t);

    QVERIFY(!wd.shouldResendStart(5200));
    QVERIFY(wd.shouldResendStart(5300));
    QCOMPARE(wd.restarts(), 1);
    QVERIFY(!wd.shouldResendStart(5400));
    QVERIFY(!wd.shouldResendStart(6299));
    QVERIFY(wd.shouldResendStart(6300));
    QCOMPARE(wd.restarts(), 2);
}

void P1LinkWatchdogTests::resumeClearsStallAndReportsRestarts()
{
    P1LinkWatchdog wd(300, 1000);
    wd.reset(0);
    wd.onIqPacket(10);
    QVERIFY(wd.shouldResendStart(1010));
    const auto ev = wd.onIqPacket(1100);
    QVERIFY(!ev.first);
    QVERIFY(ev.resumed);
    QCOMPARE(ev.gapMs, qint64(1090));
    QCOMPARE(ev.restarts, 1);
    QVERIFY(!wd.stalled());
    QCOMPARE(wd.restarts(), 0);
    QVERIFY(!wd.onIqPacket(1103).resumed);
}

void P1LinkWatchdogTests::requireSimulator()
{
    const QString sim = hpsdrSimPath();
    const QFileInfo fi(sim);
    if (sim.isEmpty() || !fi.exists() || !fi.isExecutable())
        QSKIP("hpsdr_sim missing or not executable");

    m_sim.setProgram(fi.absoluteFilePath());
    m_sim.setArguments({QStringLiteral("-P1"), QStringLiteral("-watchdog")});
    m_sim.setWorkingDirectory(fi.absolutePath());
    m_sim.setProcessChannelMode(QProcess::MergedChannels);
    m_sim.start();
    if (!m_sim.waitForStarted(3000))
        QSKIP("hpsdr_sim failed to start");
    QTest::qWait(400);
    if (m_sim.state() != QProcess::Running)
        QSKIP("hpsdr_sim exited early (port 1024 busy?)");
}

// Behaves like cudaSDR's Protocol 1 path: one host packet per received EP6
// packet, so a radio-side dropout also silences the host.
HostRunResult P1LinkWatchdogTests::runRxPacedHost(bool useWatchdog)
{
    HostRunResult result;
    QUdpSocket sock;
    if (!sock.bind(QHostAddress::AnyIPv4, 0)) {
        qWarning() << "bind failed:" << sock.errorString();
        return result;
    }
    const QHostAddress sim(QHostAddress::LocalHost);
    constexpr quint16 simPort = 1024;

    P1LinkWatchdog wd(300, 1000);
    QElapsedTimer clock;
    clock.start();
    sock.writeDatagram(p1StartStop(0x01), sim, simPort);
    wd.reset(0);

    constexpr qint64 dropoutAtMs = 1000;
    constexpr qint64 endMs = 5500;
    bool dropoutSent = false;
    quint32 sendSeq = 0;
    QByteArray datagram;

    while (clock.elapsed() < endMs) {
        sock.waitForReadyRead(20);
        while (sock.hasPendingDatagrams()) {
            datagram.resize(int(sock.pendingDatagramSize()));
            sock.readDatagram(datagram.data(), datagram.size());
            if (!isEp6(datagram))
                continue;
            const qint64 now = clock.elapsed();
            const auto ev = wd.onIqPacket(now);
            if (ev.resumed)
                result.resumed = true;
            if (!dropoutSent)
                ++result.packetsBeforeDropout;
            if (now >= endMs - 1000)
                ++result.packetsInFinalSecond;
            sock.writeDatagram(p1HostPacket(sendSeq++), sim, simPort);
        }

        const qint64 now = clock.elapsed();
        if (!dropoutSent && now >= dropoutAtMs) {
            ::kill(pid_t(m_sim.processId()), SIGUSR1);
            dropoutSent = true;
        }
        if (useWatchdog && wd.shouldResendStart(now)) {
            ++result.restarts;
            sock.writeDatagram(p1StartStop(0x00), sim, simPort);
            sock.writeDatagram(p1StartStop(0x01), sim, simPort);
        }
    }

    sock.writeDatagram(p1StartStop(0x00), sim, simPort);
    return result;
}

void P1LinkWatchdogTests::simWithoutWatchdogStaysFrozen()
{
    requireSimulator();
    const HostRunResult r = runRxPacedHost(false);
    const QByteArray log = simOutput();

    QVERIFY2(r.packetsBeforeDropout > 200, qPrintable(QString::number(r.packetsBeforeDropout)));
    QVERIFY2(log.contains("WATCHDOG"), log.constData());
    QCOMPARE(r.packetsInFinalSecond, 0);
    QCOMPARE(r.restarts, 0);
}

void P1LinkWatchdogTests::simWithWatchdogRecovers()
{
    requireSimulator();
    const HostRunResult r = runRxPacedHost(true);
    const QByteArray log = simOutput();
    qInfo().noquote() << "restarts" << r.restarts << "final-second packets"
                      << r.packetsInFinalSecond << "\n" << log;

    QVERIFY2(r.packetsBeforeDropout > 200, qPrintable(QString::number(r.packetsBeforeDropout)));
    QVERIFY2(r.restarts >= 1, log.constData());
    QVERIFY2(r.resumed, log.constData());
    QVERIFY2(r.packetsInFinalSecond > 200,
             qPrintable(QString("final-second packets %1, restarts %2\n%3")
                            .arg(r.packetsInFinalSecond).arg(r.restarts)
                            .arg(QString::fromLatin1(log))));
}

QTEST_GUILESS_MAIN(P1LinkWatchdogTests)
#include "p1_link_watchdog_tests.moc"
