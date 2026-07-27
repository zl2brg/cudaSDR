/**
* @file  cusdr_meshGeneratorWorker.cpp
* @brief Threaded mesh generation worker implementation
* @author Simon Eatough, ZL2BRG
* @version 0.1
* @date 2025-10-16
*/

#include "cusdr_meshGeneratorWorker.h"
#include <QtMath>

// Define static const members
const int MeshGeneratorWorker::MAX_TIME_SLICES;

MeshGeneratorWorker::MeshGeneratorWorker(QObject *parent)
    : QThread(parent)
    , m_stop(false)
    , m_newDataAvailable(false)
    , m_processing(false)
    , m_timeIndex(0)
    , m_spectrumWidth(512)
    , m_lodLevel(0)
    , m_heightScale(10.0f)
    , m_frequencyScale(1.0f)
    , m_timeScale(1.0f)
    , m_colorLower(-160.0f)
    , m_colorUpper(-80.0f)
{
}

MeshGeneratorWorker::~MeshGeneratorWorker() {
    stop();
    wait(); // Wait for thread to finish
}

void MeshGeneratorWorker::generateSingleSliceMesh(const QVector<float>& spectrumData,
                                                  int timeIndex,
                                                  int spectrumWidth,
                                                  int lodLevel,
                                                  float heightScale,
                                                  float frequencyScale,
                                                  float timeScale,
                                                  float colorLower,
                                                  float colorUpper,
                                                  float dBmPanMin,
                                                  float dBmPanMax) {
    QMutexLocker locker(&m_mutex);
    
    // Update parameters for single slice
    m_spectrumData = spectrumData;  // Qt implicit sharing - efficient copy
    m_timeIndex = timeIndex;
    m_spectrumWidth = spectrumWidth;
    m_lodLevel = lodLevel;
    m_heightScale = heightScale;
    m_frequencyScale = frequencyScale;
    m_timeScale = timeScale;
    m_colorLower = colorLower;
    m_colorUpper = colorUpper;
    m_dBmPanMin = dBmPanMin;
    m_dBmPanMax = dBmPanMax;
    
    m_newDataAvailable = true;
    m_condition.wakeOne(); // Wake up the worker thread
}

void MeshGeneratorWorker::stop() {
    QMutexLocker locker(&m_mutex);
    m_stop = true;
    m_condition.wakeOne();
}

bool MeshGeneratorWorker::isBusy() const {
    QMutexLocker locker(&m_mutex);
    return m_processing || m_newDataAvailable;
}

MeshGeneratorWorker::MeshData MeshGeneratorWorker::buildSliceMesh(
    const QVector<float>& spectrumData,
    int spectrumWidth,
    int lodLevel,
    float frequencyScale,
    float timeScale,
    float colorLower,
    float colorUpper,
    float dBmPanMin,
    float dBmPanMax)
{
    MeshData meshData;

    if (spectrumData.isEmpty()) {
        return meshData;
    }

    int freqBins = spectrumWidth;

  // Apply LOD factor for frequency downsampling
    int lodFactor = 1 << lodLevel;

    if (freqBins > 4000 && lodLevel == 0) {
        lodFactor = 4;
    } else if (freqBins > 2000 && lodLevel == 0) {
        lodFactor = 2;
    }

    int effectiveFreqBins = freqBins / lodFactor;

    meshData.vertices.reserve(effectiveFreqBins * 12);

    for (int f = 0; f < effectiveFreqBins; f++) {
        float amplitude = -200.0f;

        for (int freqSample = 0; freqSample < lodFactor; freqSample++) {
            int actualFreqIndex = f * lodFactor + freqSample;
            if (actualFreqIndex < spectrumData.size()) {
                amplitude = qMax(amplitude, spectrumData[actualFreqIndex]);
            }
        }

        if (amplitude < -199.0f) {
            amplitude = -120.0f;
        }

        float x = (f * lodFactor / (float)freqBins) * 400.0f * frequencyScale - 200.0f * frequencyScale;
        float y = (amplitude - dBmPanMin) / (dBmPanMax - dBmPanMin) * 40.0f;
        float z = 0.0f;

        meshData.vertices.append(x);
        meshData.vertices.append(y);
        meshData.vertices.append(z);

        QColor color = amplitudeToColorWithThresholds(amplitude, colorLower, colorUpper);
        meshData.vertices.append(color.redF());
        meshData.vertices.append(color.greenF());
        meshData.vertices.append(color.blueF());
    }

    int verticesPerRow = effectiveFreqBins;
    // Must match renderSpectrum3D spacing: (i+1)-(i) = 4.0 * timeScale.
    // A fixed -4.0 depth with default timeScale=0.2 overlaps ~5 sheets and
    // depth-tests peaks away depending on orbit angle.
    const float safeTimeScale = qMax(0.1f, timeScale);
    float zBackEdge = -4.0f * safeTimeScale;

    for (int f = 0; f < effectiveFreqBins; f++) {
        int srcIdx = f * 6;
        meshData.vertices.append(meshData.vertices[srcIdx]);
        meshData.vertices.append(meshData.vertices[srcIdx + 1]);
        meshData.vertices.append(zBackEdge);
        meshData.vertices.append(meshData.vertices[srcIdx + 3]);
        meshData.vertices.append(meshData.vertices[srcIdx + 4]);
        meshData.vertices.append(meshData.vertices[srcIdx + 5]);
    }

    for (int f = 0; f < effectiveFreqBins - 1; f++) {
        int i0 = f;
        int i1 = f + 1;
        int i2 = verticesPerRow + f;
        int i3 = verticesPerRow + f + 1;

        // Single winding only — dual windings caused coplanar z-fighting once
        // cull was disabled for Core Profile orbit views.
        meshData.indices.append(i0);
        meshData.indices.append(i1);
        meshData.indices.append(i2);

        meshData.indices.append(i1);
        meshData.indices.append(i3);
        meshData.indices.append(i2);
    }

    meshData.frontRowVertexCount = verticesPerRow;
    meshData.vertexCount = meshData.vertices.size() / 6;
    meshData.indexCount = meshData.indices.size();

    return meshData;
}

void MeshGeneratorWorker::run() {
    while (true) {
        m_mutex.lock();
        while (!m_newDataAvailable && !m_stop) {
            m_condition.wait(&m_mutex);
        }

        if (m_stop) {
            m_mutex.unlock();
            break;
        }

        QVector<float> spectrumData = m_spectrumData;
        int spectrumWidth = m_spectrumWidth;
        int lodLevel = m_lodLevel;
        float frequencyScale = m_frequencyScale;
        float timeScale = m_timeScale;
        float colorLower = m_colorLower;
        float colorUpper = m_colorUpper;
        float dBmPanMin = m_dBmPanMin;
        float dBmPanMax = m_dBmPanMax;

        Q_UNUSED(m_timeIndex)
        Q_UNUSED(m_heightScale)

        m_newDataAvailable = false;
        m_processing = true;
        m_mutex.unlock();

        MeshData meshData = buildSliceMesh(spectrumData,
                                           spectrumWidth,
                                           lodLevel,
                                           frequencyScale,
                                           timeScale,
                                           colorLower,
                                           colorUpper,
                                           dBmPanMin,
                                           dBmPanMax);

        {
            QMutexLocker locker(&m_mutex);
            m_processing = false;
        }

        if (!meshData.vertices.isEmpty()) {
            emit meshReady(meshData);
        }
    }
}

QColor MeshGeneratorWorker::amplitudeToColorWithThresholds(float amplitude,
                                                             float lowerThreshold,
                                                             float upperThreshold) {
    QColor color;
    int r, g, b;

    if (amplitude <= lowerThreshold) {
        color = QColor(0, 0, 20);
    }
    else if (amplitude >= upperThreshold) {
        color = QColor(255, 255, 255);
    }
    else {
        float offset_amp = amplitude - lowerThreshold;
        float globalRange = offset_amp / (upperThreshold - lowerThreshold);

        if (globalRange < 2.0f/9.0f) {
            float localRange = globalRange / (2.0f/9.0f);
            r = (int)((1.0f - localRange) * 0);
            g = (int)((1.0f - localRange) * 0);
            b = (int)(20 + localRange * (255 - 20));
        }
        else if (globalRange < 3.0f/9.0f) {
            float localRange = (globalRange - 2.0f/9.0f) / (1.0f/9.0f);
            r = 0;
            g = (int)(localRange * 255);
            b = 255;
        }
        else if (globalRange < 4.0f/9.0f) {
            float localRange = (globalRange - 3.0f/9.0f) / (1.0f/9.0f);
            r = 0;
            g = 255;
            b = (int)((1.0f - localRange) * 255);
        }
        else if (globalRange < 5.0f/9.0f) {
            float localRange = (globalRange - 4.0f/9.0f) / (1.0f/9.0f);
            r = (int)(localRange * 255);
            g = 255;
            b = 0;
        }
        else if (globalRange < 7.0f/9.0f) {
            float localRange = (globalRange - 5.0f/9.0f) / (2.0f/9.0f);
            r = 255;
            g = (int)((1.0f - localRange) * 255);
            b = 0;
        }
        else if (globalRange < 8.0f/9.0f) {
            float localRange = (globalRange - 7.0f/9.0f) / (1.0f/9.0f);
            r = 255;
            g = 0;
            b = (int)(localRange * 255);
        }
        else {
            float localRange = (globalRange - 8.0f/9.0f) / (1.0f/9.0f);
            r = (int)((0.75f + 0.25f * (1.0f - localRange)) * 255);
            g = (int)(localRange * 255 * 0.5f);
            b = 255;
        }

        r = qBound(0, r, 255);
        g = qBound(0, g, 255);
        b = qBound(0, b, 255);

        color = QColor(r, g, b);
    }

    return color;
}
