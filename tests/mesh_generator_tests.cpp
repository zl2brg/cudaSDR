/**
 * @file mesh_generator_tests.cpp
 * @brief CPU tests for 3D spectrum mesh frequency-axis mapping.
 */

#include <QtTest/QtTest>

#include "GL/cusdr_meshGeneratorWorker.h"

class MeshGeneratorTests : public QObject {
    Q_OBJECT

private slots:
    void testLowFrequencyBinMapsToNegativeX();
    void testHighFrequencyBinMapsToPositiveX();
    void testSliceMeshFollowsFrequencyWorldX();
    void testRibbonFollowsScrollDirection();
};

void MeshGeneratorTests::testLowFrequencyBinMapsToNegativeX()
{
    QCOMPARE(MeshGeneratorWorker::frequencyWorldX(0, 512, 1.0f), -200.0f);
}

void MeshGeneratorTests::testHighFrequencyBinMapsToPositiveX()
{
    QCOMPARE(MeshGeneratorWorker::frequencyWorldX(512, 512, 1.0f), 200.0f);
    QVERIFY(MeshGeneratorWorker::frequencyWorldX(511, 512, 1.0f) > 0.0f);
}

void MeshGeneratorTests::testSliceMeshFollowsFrequencyWorldX()
{
    QVector<float> spectrum(64, -120.0f);
    spectrum[0] = -40.0f;
    spectrum[63] = -80.0f;

    const auto mesh = MeshGeneratorWorker::buildSliceMesh(
        spectrum, 64, 0, 1.0f, 1.0f, -160.0f, -80.0f, -140.0f, -40.0f);
    QVERIFY(mesh.frontRowVertexCount >= 2);
    QVERIFY(mesh.vertices.size() >= 12);

    const float xLow = mesh.vertices.at(0);
    const float xHigh = mesh.vertices.at((mesh.frontRowVertexCount - 1) * 6);
    QVERIFY(xLow < 0.0f);
    QVERIFY(xHigh > 0.0f);
    QVERIFY(xLow < xHigh);
}

void MeshGeneratorTests::testRibbonFollowsScrollDirection()
{
    QVector<float> spectrum(8, -120.0f);
    const auto bottom = MeshGeneratorWorker::buildSliceMesh(
        spectrum, 8, 0, 1.0f, 1.0f, -160.0f, -80.0f, -140.0f, -40.0f, true);
    const auto top = MeshGeneratorWorker::buildSliceMesh(
        spectrum, 8, 0, 1.0f, 1.0f, -160.0f, -80.0f, -140.0f, -40.0f, false);
    QVERIFY(bottom.frontRowVertexCount > 0);
    QVERIFY(top.frontRowVertexCount == bottom.frontRowVertexCount);

    const int backZ = bottom.frontRowVertexCount * 6 + 2;
    QVERIFY(bottom.vertices.at(backZ) > 0.0f);
    QVERIFY(top.vertices.at(backZ) < 0.0f);
}

QTEST_MAIN(MeshGeneratorTests)
#include "mesh_generator_tests.moc"
