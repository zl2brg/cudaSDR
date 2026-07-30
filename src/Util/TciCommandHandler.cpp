#include "Util/TciCommandHandler.h"

using namespace TciProtocol;

TciCommandHandler::TciCommandHandler(TciRoutingState *routingState)
    : m_routingState(routingState)
{
}

void TciCommandHandler::resetPending()
{
    m_pendingNotification.clear();
    m_needsServer = false;
    m_lastName.clear();
    m_lastArgs.clear();
    m_vfoRequest.reset();
    m_splitRequest.reset();
    m_trxRequest.reset();
    m_tuneRequest.reset();
    m_driveRequest.reset();
    m_modulationRequest.reset();
    m_filterRequest.reset();
    m_ddsRequest.reset();
    m_ifRequest.reset();
    m_startStopRequest.reset();
}

QString TciCommandHandler::pendingNotification()
{
    const QString note = m_pendingNotification;
    m_pendingNotification.clear();
    return note;
}

std::optional<TciCommandHandler::VfoRequest> TciCommandHandler::takeVfoRequest()
{
    auto req = std::move(m_vfoRequest);
    m_vfoRequest.reset();
    return req;
}

std::optional<TciCommandHandler::SplitRequest> TciCommandHandler::takeSplitRequest()
{
    auto req = std::move(m_splitRequest);
    m_splitRequest.reset();
    return req;
}

std::optional<TciCommandHandler::TrxRequest> TciCommandHandler::takeTrxRequest()
{
    auto req = std::move(m_trxRequest);
    m_trxRequest.reset();
    return req;
}

std::optional<TciCommandHandler::TuneRequest> TciCommandHandler::takeTuneRequest()
{
    auto req = std::move(m_tuneRequest);
    m_tuneRequest.reset();
    return req;
}

std::optional<TciCommandHandler::DriveRequest> TciCommandHandler::takeDriveRequest()
{
    auto req = std::move(m_driveRequest);
    m_driveRequest.reset();
    return req;
}

std::optional<TciCommandHandler::ModulationRequest> TciCommandHandler::takeModulationRequest()
{
    auto req = std::move(m_modulationRequest);
    m_modulationRequest.reset();
    return req;
}

std::optional<TciCommandHandler::FilterRequest> TciCommandHandler::takeFilterRequest()
{
    auto req = std::move(m_filterRequest);
    m_filterRequest.reset();
    return req;
}

std::optional<TciCommandHandler::DdsRequest> TciCommandHandler::takeDdsRequest()
{
    auto req = std::move(m_ddsRequest);
    m_ddsRequest.reset();
    return req;
}

std::optional<TciCommandHandler::IfRequest> TciCommandHandler::takeIfRequest()
{
    auto req = std::move(m_ifRequest);
    m_ifRequest.reset();
    return req;
}

std::optional<TciCommandHandler::StartStopRequest> TciCommandHandler::takeStartStopRequest()
{
    auto req = std::move(m_startStopRequest);
    m_startStopRequest.reset();
    return req;
}

QString TciCommandHandler::handleCommand(const QString &cmd)
{
    resetPending();

    const QString trimmed = cmd.trimmed();
    if (trimmed.isEmpty())
        return {};

    const int colon = trimmed.indexOf(':');
    QString name;
    QStringList args;
    if (colon < 0) {
        name = trimmed.toLower();
    } else {
        name = trimmed.left(colon).trimmed().toLower();
        args = trimmed.mid(colon + 1).split(',', Qt::KeepEmptyParts);
        for (QString &a : args)
            a = a.trimmed();
    }

    m_lastName = name;
    m_lastArgs = args;
    const bool isSet = !args.isEmpty();

    if (name == QLatin1String("vfo"))
        return cmdVfo(args, args.size() >= 3);
    if (name == QLatin1String("dds"))
        return cmdDds(args, args.size() >= 2);
    if (name == QLatin1String("if"))
        return cmdIf(args, args.size() >= 3);
    if (name == QLatin1String("modulation"))
        return cmdModulation(args, args.size() >= 2);
    if (name == QLatin1String("rx_filter_band"))
        return cmdRxFilterBand(args, args.size() >= 3);
    if (name == QLatin1String("trx") || name == QLatin1String("tx_enable"))
        return cmdTrx(args, args.size() >= 2);
    if (name == QLatin1String("tune"))
        return cmdTune(args, args.size() >= 2);
    if (name == QLatin1String("drive"))
        return cmdDrive(args, isSet);
    if (name == QLatin1String("tune_drive"))
        return cmdTuneDrive(args, isSet);
    if (name == QLatin1String("split_enable"))
        return cmdSplitEnable(args, isSet);
    if (name == QLatin1String("rit_enable")
        || name == QLatin1String("xit_enable")
        || name == QLatin1String("mute")
        || name == QLatin1String("lock")
        || name == QLatin1String("vfo_lock"))
        return cmdEnableStub(name, args, isSet);
    if (name == QLatin1String("start"))
        return cmdStart();
    if (name == QLatin1String("stop"))
        return cmdStop();

    // Stream / session verbs and remaining stubs stay on TciServer.
    m_needsServer = true;
    return {};
}

QString TciCommandHandler::cmdVfo(const QStringList &args, bool isSet)
{
    if (args.isEmpty())
        return {};

    bool trxOk = false;
    const int trx = args.at(0).toInt(&trxOk);
    if (!trxOk || trx < 0)
        return {};

    bool channelOk = args.size() < 2;
    const int channel = args.size() >= 2 ? args.at(1).toInt(&channelOk) : 0;
    if (!channelOk || channel < 0 || channel > 1)
        return {};

    if (!isSet) {
        // GET needs live radio state — server fills the reply.
        m_needsServer = true;
        return {};
    }

    if (args.size() < 3)
        return {};
    bool ok = false;
    const qint64 hz = args.at(2).toLongLong(&ok);
    if (!ok || !isValidVfoHz(hz))
        return {};

    m_vfoRequest = VfoRequest{trx, channel, hz};
    return {};
}

QString TciCommandHandler::cmdDds(const QStringList &args, bool isSet)
{
    if (!isSet) {
        m_needsServer = true;
        return {};
    }
    if (args.size() < 2)
        return {};

    bool trxOk = false;
    const int trx = args.at(0).toInt(&trxOk);
    if (!trxOk || trx < 0)
        return {};

    int channel = 0;
    qint64 freq = 0;
    bool ok = false;
    if (args.size() >= 3) {
        channel = args.at(1).toInt(&ok);
        if (!ok || channel < 0 || channel > 1)
            return {};
        freq = args.at(2).toLongLong(&ok);
    } else {
        freq = args.at(1).toLongLong(&ok);
    }
    if (!ok || !isValidVfoHz(freq))
        return {};

    m_ddsRequest = DdsRequest{trx, channel, freq};
    return {};
}

QString TciCommandHandler::cmdIf(const QStringList &args, bool isSet)
{
    if (!isSet) {
        m_needsServer = true;
        return {};
    }
    if (args.size() < 3)
        return {};

    bool trxOk = false;
    const int trx = args.at(0).toInt(&trxOk);
    if (!trxOk || trx < 0)
        return {};

    bool channelOk = false;
    const int channel = args.at(1).toInt(&channelOk);
    if (!channelOk || channel < 0 || channel > 1)
        return {};

    bool ok = false;
    const qint64 offset = args.at(2).toLongLong(&ok);
    if (!ok)
        return {};

    m_ifRequest = IfRequest{trx, channel, offset};
    return {};
}

QString TciCommandHandler::cmdModulation(const QStringList &args, bool isSet)
{
    if (args.isEmpty())
        return {};
    bool trxOk = false;
    const int trx = args.at(0).toInt(&trxOk);
    if (!trxOk || trx < 0)
        return {};

    if (!isSet) {
        m_needsServer = true;
        return {};
    }
    if (args.size() < 2 || args.at(1).isEmpty())
        return {};

    const QString tciName = args.at(1);
    m_modulationRequest = ModulationRequest{trx, tciModeToDsp(tciName), tciName.toLower()};
    return {};
}

QString TciCommandHandler::cmdRxFilterBand(const QStringList &args, bool isSet)
{
    if (args.isEmpty())
        return {};
    bool trxOk = false;
    const int trx = args.at(0).toInt(&trxOk);
    if (!trxOk || trx < 0)
        return {};

    if (!isSet) {
        m_needsServer = true;
        return {};
    }
    if (args.size() < 3)
        return {};

    bool okLo = false;
    bool okHi = false;
    const qreal lo = args.at(1).toDouble(&okLo);
    const qreal hi = args.at(2).toDouble(&okHi);
    if (!okLo || !okHi)
        return {};

    m_filterRequest = FilterRequest{trx, lo, hi};
    return {};
}

QString TciCommandHandler::cmdTrx(const QStringList &args, bool isSet)
{
    if (args.isEmpty())
        return {};
    bool trxOk = false;
    const int trx = args.at(0).toInt(&trxOk);
    if (!trxOk || trx < 0)
        return {};

    if (!isSet) {
        m_needsServer = true;
        return {};
    }
    if (args.size() < 2)
        return {};

    const QString source = args.size() >= 3 ? args.at(2).toLower() : QString();
    m_trxRequest = TrxRequest{trx, parseBoolArg(args.at(1)), source};
    return {};
}

QString TciCommandHandler::cmdTune(const QStringList &args, bool isSet)
{
    if (args.isEmpty())
        return {};
    bool trxOk = false;
    const int trx = args.at(0).toInt(&trxOk);
    if (!trxOk || trx < 0)
        return {};

    if (!isSet) {
        m_needsServer = true;
        return {};
    }
    if (args.size() < 2)
        return {};

    m_tuneRequest = TuneRequest{trx, parseBoolArg(args.at(1))};
    return {};
}

QString TciCommandHandler::cmdDrive(const QStringList &args, bool isSet)
{
    // drive:<trx> is a read; only drive:<trx>,<power> (or legacy bare? no —
    // single-arg is always GET for ESDR3 / WSJT-X) writes when two args present.
    Q_UNUSED(isSet)
    if (args.size() < 2) {
        m_needsServer = true;
        return {};
    }

    bool ok = false;
    const int trx = args.at(0).toInt(&ok);
    if (!ok || trx < 0)
        return {};
    const int level = args.at(1).toInt(&ok);
    if (!ok)
        return {};
    m_driveRequest = DriveRequest{trx, qBound(0, level, 100), true};
    return {};
}

QString TciCommandHandler::cmdTuneDrive(const QStringList &args, bool isSet)
{
    // Same shape as drive for WSJT-X ESDR3 (trx,power). cudaSDR maps both to
    // the single drive level today.
    return cmdDrive(args, isSet);
}

QString TciCommandHandler::cmdSplitEnable(const QStringList &args, bool isSet)
{
    int trx = 0;
    bool enable = false;
    bool hasValue = false;

    if (!args.isEmpty()) {
        bool ok = false;
        trx = args.at(0).toInt(&ok);
        if (!ok || trx < 0)
            return {};
        if (args.size() >= 2) {
            enable = parseBoolArg(args.at(1));
            hasValue = true;
        } else {
            // Pure integer single arg = GET for that TRX (ExpertSDR style).
            const QString a0 = args.at(0).trimmed().toLower();
            bool intOnly = false;
            a0.toInt(&intOnly);
            if (!intOnly) {
                enable = parseBoolArg(args.at(0));
                hasValue = true;
                trx = 0;
            }
        }
    }

    if (!hasValue) {
        const bool split = m_routingState && m_routingState->splitRequested();
        return tciMessage(QStringLiteral("SPLIT_ENABLE"),
                          {QString::number(trx),
                           split ? QStringLiteral("true") : QStringLiteral("false")});
    }

    m_splitRequest = SplitRequest{trx, enable};
    return {};
}

QString TciCommandHandler::cmdEnableStub(const QString &name, const QStringList &args, bool isSet)
{
    int trx = 0;
    bool enable = false;
    bool hasValue = false;

    if (name == QLatin1String("mute")) {
        if (isSet && !args.isEmpty()) {
            enable = parseBoolArg(args.last());
            hasValue = true;
        }
        if (hasValue) {
            m_pendingNotification = tciMessage(name, {enable ? QStringLiteral("true")
                                                             : QStringLiteral("false")});
            return {};
        }
        return tciMessage(name, {QStringLiteral("false")});
    }

    if (name == QLatin1String("vfo_lock") && args.size() >= 3) {
        bool ok = false;
        trx = args.at(0).toInt(&ok);
        if (!ok)
            return {};
        enable = parseBoolArg(args.at(2));
        m_pendingNotification = tciMessage(name,
                                           {QString::number(trx),
                                            args.at(1),
                                            enable ? QStringLiteral("true") : QStringLiteral("false")});
        return {};
    }

    if (!args.isEmpty()) {
        bool ok = false;
        trx = args.at(0).toInt(&ok);
        if (!ok)
            trx = 0;
        if (args.size() >= 2) {
            enable = parseBoolArg(args.at(1));
            hasValue = true;
        }
    }

    if (hasValue) {
        m_pendingNotification = tciMessage(name,
                                           {QString::number(trx),
                                            enable ? QStringLiteral("true") : QStringLiteral("false")});
        return {};
    }

    return tciMessage(name, {QString::number(trx), QStringLiteral("false")});
}

QString TciCommandHandler::cmdStart()
{
    m_startStopRequest = StartStopRequest{true};
    return {};
}

QString TciCommandHandler::cmdStop()
{
    m_startStopRequest = StartStopRequest{false};
    return {};
}
