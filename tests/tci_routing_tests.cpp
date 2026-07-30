#include <QtTest/QtTest>

#include "Util/TciRoutingState.h"
#include "Util/TciCommandHandler.h"

class TciRoutingTests : public QObject {
    Q_OBJECT

private slots:
    void resolveVfoBCreatesWhenNoTx();
    void resolveVfoBUsesExistingTx();
    void resolveVfoBPromotesTrackedRoute();
    void splitFalseDoesNotClearRoute();
    void commandHandlerParsesVfoB();
    void commandHandlerSplitGetUsesRoutingState();
};

void TciRoutingTests::resolveVfoBCreatesWhenNoTx()
{
    TciRoutingState state;
    const QVector<TciSliceEndpoint> endpoints{{0, false}};
    const auto d = state.resolveVfoB(0, endpoints);
    QCOMPARE(d.action, TciRoutingState::RouteAction::Create);
    QCOMPARE(d.txSliceId, -1);
}

void TciRoutingTests::resolveVfoBUsesExistingTx()
{
    TciRoutingState state;
    const QVector<TciSliceEndpoint> endpoints{{0, false}, {1, true}};
    const auto d = state.resolveVfoB(0, endpoints);
    QCOMPARE(d.action, TciRoutingState::RouteAction::UseExisting);
    QCOMPARE(d.txSliceId, 1);
    QCOMPARE(state.owner(), TciRoutingState::TxRouteOwner::External);
}

void TciRoutingTests::resolveVfoBPromotesTrackedRoute()
{
    TciRoutingState state;
    state.bindCreatedRoute(0, 1);
    const QVector<TciSliceEndpoint> endpoints{{0, false}, {1, false}};
    const auto d = state.resolveVfoB(0, endpoints);
    QCOMPARE(d.action, TciRoutingState::RouteAction::PromoteExisting);
    QCOMPARE(d.txSliceId, 1);
}

void TciRoutingTests::splitFalseDoesNotClearRoute()
{
    TciRoutingState state;
    state.bindCreatedRoute(0, 1);
    QVERIFY(state.setSplitRequested(true));
    QVERIFY(!state.setSplitRequested(true)); // no-op
    QCOMPARE(state.txSliceId(), 1);
    QVERIFY(state.setSplitRequested(false));
    // clearTciRoute is explicit — setSplitRequested alone must keep the route.
    QCOMPARE(state.txSliceId(), 1);
    state.clearTciRoute();
    QCOMPARE(state.txSliceId(), -1);
}

void TciRoutingTests::commandHandlerParsesVfoB()
{
    TciRoutingState state;
    TciCommandHandler handler(&state);
    QCOMPARE(handler.handleCommand(QStringLiteral("vfo:0,1,14074000")), QString());
    const auto req = handler.takeVfoRequest();
    QVERIFY(req.has_value());
    QCOMPARE(req->trx, 0);
    QCOMPARE(req->channel, 1);
    QCOMPARE(req->frequencyHz, 14074000);
}

void TciRoutingTests::commandHandlerSplitGetUsesRoutingState()
{
    TciRoutingState state;
    state.setSplitRequested(true);
    TciCommandHandler handler(&state);
    QCOMPARE(handler.handleCommand(QStringLiteral("split_enable:0")),
             QStringLiteral("split_enable:0,true;"));
}

QTEST_MAIN(TciRoutingTests)
#include "tci_routing_tests.moc"
