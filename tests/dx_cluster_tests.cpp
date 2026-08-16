#include <QtTest/QtTest>
#include <QSignalSpy>

#include "Util/DxClusterClient.h"
#include "Models/BandPlanManager.h"

class DxClusterTests : public QObject {
    Q_OBJECT

private slots:
    // Parser Tests
    void testParseRbnCwSpot();
    void testParseRbnFt8Spot();
    void testParseClassicDxSpot();
    void testParseNonDxLine();

    // Spot Manager & TTL Tests
    void testDynamicSpotAddition();
    void testDynamicSpotTtlExpiry();
    void testDynamicSpotUpdateSameFrequency();

    // Client State Tests
    void testClientInitialState();
    void testClientCallsign();
};

void DxClusterTests::testParseRbnCwSpot()
{
    const QString line = QStringLiteral("DX de K3LR-#:    14025.1  ZL2BRG         CW    28 dB  24 WPM  CQ    1420Z");
    qint64 freqHz = 0;
    QString dxCall, mode, spotter, comment, timeUtc;
    int snr = 0, wpm = 0;

    const bool ok = DxClusterClient::parseDxLine(line, freqHz, dxCall, mode, snr, wpm, spotter, comment, timeUtc);
    QVERIFY(ok);
    QCOMPARE(freqHz, 14025100LL);
    QCOMPARE(dxCall, QStringLiteral("ZL2BRG"));
    QCOMPARE(mode, QStringLiteral("CW"));
    QCOMPARE(snr, 28);
    QCOMPARE(wpm, 24);
    QCOMPARE(spotter, QStringLiteral("K3LR-#"));
    QCOMPARE(comment, QStringLiteral("CQ"));
    QCOMPARE(timeUtc, QStringLiteral("1420Z"));
}

void DxClusterTests::testParseRbnFt8Spot()
{
    const QString line = QStringLiteral("DX de VE7CC-#:   14074.0  JA1ABC         FT8   -08 dB         CQ    1422Z");
    qint64 freqHz = 0;
    QString dxCall, mode, spotter, comment, timeUtc;
    int snr = 0, wpm = 0;

    const bool ok = DxClusterClient::parseDxLine(line, freqHz, dxCall, mode, snr, wpm, spotter, comment, timeUtc);
    QVERIFY(ok);
    QCOMPARE(freqHz, 14074000LL);
    QCOMPARE(dxCall, QStringLiteral("JA1ABC"));
    QCOMPARE(mode, QStringLiteral("FT8"));
    QCOMPARE(snr, -8);
    QCOMPARE(wpm, 0);
    QCOMPARE(spotter, QStringLiteral("VE7CC-#"));
}

void DxClusterTests::testParseClassicDxSpot()
{
    const QString line = QStringLiteral("DX de OH2AQ:     14195.0  VK9WA          up 5-10                1423Z");
    qint64 freqHz = 0;
    QString dxCall, mode, spotter, comment, timeUtc;
    int snr = 0, wpm = 0;

    const bool ok = DxClusterClient::parseDxLine(line, freqHz, dxCall, mode, snr, wpm, spotter, comment, timeUtc);
    QVERIFY(ok);
    QCOMPARE(freqHz, 14195000LL);
    QCOMPARE(dxCall, QStringLiteral("VK9WA"));
    QCOMPARE(spotter, QStringLiteral("OH2AQ"));
    QCOMPARE(comment, QStringLiteral("up 5-10"));
    QCOMPARE(timeUtc, QStringLiteral("1423Z"));
}

void DxClusterTests::testParseNonDxLine()
{
    qint64 freqHz = 0;
    QString dxCall, mode, spotter, comment, timeUtc;
    int snr = 0, wpm = 0;

    QVERIFY(!DxClusterClient::parseDxLine(QStringLiteral("Hello and welcome to the DX cluster"), freqHz, dxCall, mode, snr, wpm, spotter, comment, timeUtc));
    QVERIFY(!DxClusterClient::parseDxLine(QStringLiteral("login: "), freqHz, dxCall, mode, snr, wpm, spotter, comment, timeUtc));
    QVERIFY(!DxClusterClient::parseDxLine(QStringLiteral(""), freqHz, dxCall, mode, snr, wpm, spotter, comment, timeUtc));
}

void DxClusterTests::testDynamicSpotAddition()
{
    BandPlanManager mgr;
    QCOMPARE(mgr.spots().size(), 0);

    QSignalSpy spyPlan(&mgr, &BandPlanManager::planChanged);

    mgr.addSpotMarker(14025100, QStringLiteral("ZL2BRG"), QStringLiteral("CW"), 28, 24, QStringLiteral("K3LR-#"), QStringLiteral("CQ"), 900);
    QCOMPARE(mgr.spots().size(), 1);
    QCOMPARE(spyPlan.count(), 1);

    const BandSpot &s = mgr.spots().at(0);
    QCOMPARE(s.freqHz, 14025100LL);
    QCOMPARE(s.label, QStringLiteral("ZL2BRG [CW 24wpm +28dB]"));
    QCOMPARE(s.mode, QStringLiteral("CW"));
    QCOMPARE(s.snr, 28);
    QCOMPARE(s.wpm, 24);
    QCOMPARE(s.ttlSec, 900);

    QCOMPARE(mgr.spotsInSpan(14020000, 14030000).size(), 1);
    QCOMPARE(mgr.spotsInSpan(7000000, 7100000).size(), 0);
}

void DxClusterTests::testDynamicSpotTtlExpiry()
{
    BandPlanManager mgr;
    mgr.addSpotMarker(7012000, QStringLiteral("W1AW"), QStringLiteral("CW"), 15, 20, QStringLiteral("W3LPL"), QStringLiteral(""), 300);
    QCOMPARE(mgr.spots().size(), 1);

    const qint64 nowSec = QDateTime::currentSecsSinceEpoch();

    // Not expired now
    QVERIFY(!mgr.spots().at(0).isExpired(nowSec + 100));

    // Expired 400s in the future
    QVERIFY(mgr.spots().at(0).isExpired(nowSec + 400));

    // Prune with simulated future time
    const bool pruned = mgr.pruneExpiredSpots(nowSec + 400);
    QVERIFY(pruned);
    QCOMPARE(mgr.spots().size(), 0);
}

void DxClusterTests::testDynamicSpotUpdateSameFrequency()
{
    BandPlanManager mgr;
    mgr.addSpotMarker(14074000, QStringLiteral("JA1ABC"), QStringLiteral("FT8"), -10, 0, QStringLiteral("VE7CC"), QStringLiteral(""), 900);
    QCOMPARE(mgr.spots().size(), 1);
    QCOMPARE(mgr.spots().at(0).label, QStringLiteral("JA1ABC [FT8 -10dB]"));

    // Update with better SNR
    mgr.addSpotMarker(14074050, QStringLiteral("JA1ABC"), QStringLiteral("FT8"), +5, 0, QStringLiteral("K3LR"), QStringLiteral(""), 900);
    // Should update existing spot rather than creating a duplicate
    QCOMPARE(mgr.spots().size(), 1);
    QCOMPARE(mgr.spots().at(0).label, QStringLiteral("JA1ABC [FT8 +5dB]"));
    QCOMPARE(mgr.spots().at(0).snr, 5);
}

void DxClusterTests::testClientInitialState()
{
    DxClusterClient client;
    QCOMPARE(client.state(), DxClusterClient::Disconnected);
    QCOMPARE(client.isConnected(), false);
    QCOMPARE(client.autoReconnect(), true);
}

void DxClusterTests::testClientCallsign()
{
    DxClusterClient client;
    client.setCallsign(QStringLiteral("zl2brg"));
    QCOMPARE(client.callsign(), QStringLiteral("ZL2BRG"));
}

QTEST_MAIN(DxClusterTests)
#include "dx_cluster_tests.moc"
