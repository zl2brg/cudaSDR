/**
 * @file  TciCommandHandler.h
 * @brief Pure TCI text parse / notify layer (no sockets, no Settings writes).
 *
 * Mirrors AetherSDR's TciProtocol pattern: handleCommand() returns an immediate
 * reply (if any), and take*Request() drains deferred side effects for TciServer.
 */

#ifndef TCI_COMMAND_HANDLER_H
#define TCI_COMMAND_HANDLER_H

#include "Util/tci_protocol_utils.h"
#include "Util/TciRoutingState.h"

#include <optional>

#include <QString>
#include <QStringList>

class TciCommandHandler {
public:
    struct VfoRequest {
        int trx = -1;
        int channel = -1; // 0 = VFO-A (RX), 1 = VFO-B (TX route)
        qint64 frequencyHz = 0;
    };

    struct SplitRequest {
        int trx = -1;
        bool enabled = false;
    };

    struct TrxRequest {
        int trx = -1;
        bool transmitting = false;
        QString source;
    };

    struct TuneRequest {
        int trx = -1;
        bool enabled = false;
    };

    struct DriveRequest {
        int trx = -1;
        int level = -1; // -1 = GET only was handled as reply
        bool isSet = false;
    };

    struct ModulationRequest {
        int trx = -1;
        DSPMode mode = USB;
        QString tciName;
    };

    struct FilterRequest {
        int trx = -1;
        qreal lo = 0.0;
        qreal hi = 0.0;
    };

    struct DdsRequest {
        int trx = -1;
        int channel = 0;
        qint64 frequencyHz = 0;
    };

    struct IfRequest {
        int trx = -1;
        int channel = 0;
        qint64 offsetHz = 0;
    };

    struct StartStopRequest {
        bool start = false;
    };

    explicit TciCommandHandler(TciRoutingState *routingState = nullptr);

    void setRoutingState(TciRoutingState *routingState) { m_routingState = routingState; }

    /** Process one TCI command (without trailing semicolon). */
    QString handleCommand(const QString &cmd);

    /** Notification to broadcast after a SET that mutated shared state locally. */
    QString pendingNotification();

    std::optional<VfoRequest> takeVfoRequest();
    std::optional<SplitRequest> takeSplitRequest();
    std::optional<TrxRequest> takeTrxRequest();
    std::optional<TuneRequest> takeTuneRequest();
    std::optional<DriveRequest> takeDriveRequest();
    std::optional<ModulationRequest> takeModulationRequest();
    std::optional<FilterRequest> takeFilterRequest();
    std::optional<DdsRequest> takeDdsRequest();
    std::optional<IfRequest> takeIfRequest();
    std::optional<StartStopRequest> takeStartStopRequest();

    /** True when the command was recognised as a stream/session verb that
     *  TciServer must handle with per-client state (AUDIO_*, IQ_*, …). */
    bool lastCommandNeedsServer() const { return m_needsServer; }
    QString lastCommandName() const { return m_lastName; }
    QStringList lastCommandArgs() const { return m_lastArgs; }

private:
    void resetPending();

    QString cmdVfo(const QStringList &args, bool isSet);
    QString cmdDds(const QStringList &args, bool isSet);
    QString cmdIf(const QStringList &args, bool isSet);
    QString cmdModulation(const QStringList &args, bool isSet);
    QString cmdRxFilterBand(const QStringList &args, bool isSet);
    QString cmdTrx(const QStringList &args, bool isSet);
    QString cmdTune(const QStringList &args, bool isSet);
    QString cmdDrive(const QStringList &args, bool isSet);
    QString cmdTuneDrive(const QStringList &args, bool isSet);
    QString cmdSplitEnable(const QStringList &args, bool isSet);
    QString cmdEnableStub(const QString &name, const QStringList &args, bool isSet);
    QString cmdStart();
    QString cmdStop();

    TciRoutingState *m_routingState = nullptr;
    QString m_pendingNotification;
    QString m_lastName;
    QStringList m_lastArgs;
    bool m_needsServer = false;

    std::optional<VfoRequest> m_vfoRequest;
    std::optional<SplitRequest> m_splitRequest;
    std::optional<TrxRequest> m_trxRequest;
    std::optional<TuneRequest> m_tuneRequest;
    std::optional<DriveRequest> m_driveRequest;
    std::optional<ModulationRequest> m_modulationRequest;
    std::optional<FilterRequest> m_filterRequest;
    std::optional<DdsRequest> m_ddsRequest;
    std::optional<IfRequest> m_ifRequest;
    std::optional<StartStopRequest> m_startStopRequest;
};

#endif // TCI_COMMAND_HANDLER_H
