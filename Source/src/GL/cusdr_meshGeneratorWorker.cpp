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
    , m_spectrumWidth(512)
    , m_cameraDistance(300.0f)
    , m_heightScale(10.0f)
    , m_frequencyScale(1.0f)
    , m_timeScale(1.0f)
    , m_waterfallOffset(0.0f)
{
}

MeshGeneratorWorker::~MeshGeneratorWorker() {
    stop();
    wait(); // Wait for thread to finish
}

void MeshGeneratorWorker::generateMesh(const QVector<QVector<float>>& spectrumHistory,
                                       int spectrumWidth,
                                       float cameraDistance,
                                       float heightScale,
                                       float frequencyScale,
                                       float timeScale,
                                       float waterfallOffset) {
    QMutexLocker locker(&m_mutex);
    
    // Update parameters
    m_spectrumHistory = spectrumHistory;  // Qt implicit sharing - efficient copy
    m_spectrumWidth = spectrumWidth;
    m_cameraDistance = cameraDistance;
    m_heightScale = heightScale;
    m_frequencyScale = frequencyScale;
    m_timeScale = timeScale;
    m_waterfallOffset = waterfallOffset;
    
    m_newDataAvailable = true;
    m_condition.wakeOne(); // Wake up the worker thread
}

void MeshGeneratorWorker::stop() {
    QMutexLocker locker(&m_mutex);
    m_stop = true;
    m_condition.wakeOne();
}

void MeshGeneratorWorker::run() {
    while (true) {
        // Wait for new data or stop signal
        m_mutex.lock();
        while (!m_newDataAvailable && !m_stop) {
            m_condition.wait(&m_mutex);
        }
        
        if (m_stop) {
            m_mutex.unlock();
            break;
        }
        
        // Copy parameters locally
        QVector<QVector<float>> spectrumHistory = m_spectrumHistory;
        int spectrumWidth = m_spectrumWidth;
        float cameraDistance = m_cameraDistance;
        float frequencyScale = m_frequencyScale;
        float timeScale = m_timeScale;
        float waterfallOffset = m_waterfallOffset;
        
        m_newDataAvailable = false;
        m_mutex.unlock();
        
        // Generate mesh (CPU-intensive part, no locks needed)
        MeshData meshData;
        
        if (spectrumHistory.isEmpty()) {
            continue;
        }
        
        int timeSlices = qMin(spectrumHistory.size(), MAX_TIME_SLICES);
        int freqBins = spectrumWidth;
        
        // Level of Detail (LOD) optimization based on camera distance
        int lodFactor = 1;
        if (cameraDistance > 200.0f) {
            lodFactor = 4;
        } else if (cameraDistance > 100.0f) {
            lodFactor = 2;
        }
        
        int effectiveFreqBins = freqBins / lodFactor;
        int effectiveTimeSlices = timeSlices / lodFactor;
        
        // Pre-allocate memory for efficiency
        int estimatedVertices = effectiveTimeSlices * effectiveFreqBins * 6; // 6 floats per vertex
        int estimatedIndices = (effectiveTimeSlices - 1) * (effectiveFreqBins - 1) * 6; // 6 indices per quad
        
        meshData.vertices.reserve(estimatedVertices);
        meshData.indices.reserve(estimatedIndices);
        
        // Generate vertices with peak-preserving LOD
        for (int t = 0; t < effectiveTimeSlices; t++) {
            for (int f = 0; f < effectiveFreqBins; f++) {
                // Peak-preserving LOD: Take MAXIMUM across both time and frequency ranges
                float amplitude = -200.0f;
                
                // Loop through time samples in this LOD bin
                for (int timeSample = 0; timeSample < lodFactor; timeSample++) {
                    int actualTimeIndex = t * lodFactor + timeSample;
                    if (actualTimeIndex >= spectrumHistory.size()) break;
                    
                    const QVector<float>& spectrum = spectrumHistory[actualTimeIndex];
                    
                    // Loop through frequency samples in this LOD bin
                    for (int freqSample = 0; freqSample < lodFactor; freqSample++) {
                        int actualFreqIndex = f * lodFactor + freqSample;
                        if (actualFreqIndex < spectrum.size()) {
                            amplitude = qMax(amplitude, spectrum[actualFreqIndex]);
                        }
                    }
                }
                
                // If no samples found, use default
                if (amplitude < -199.0f) {
                    amplitude = -120.0f;
                }
                
                // Position (frequency, amplitude, time) with user-controllable scales
                float x = (float)f / (float)effectiveFreqBins * 400.0f * frequencyScale - 200.0f * frequencyScale;
                float y = (amplitude + 120.0f) / 80.0f * 40.0f; // Range 0 to 40
                float z = (effectiveTimeSlices - (float)t) * 4.0f * timeScale - effectiveTimeSlices * 2.0f * timeScale;
                
                meshData.vertices.append(x);
                meshData.vertices.append(y);
                meshData.vertices.append(z);
                
                // Color based on amplitude
                QColor color = amplitudeToColorWithOffset(amplitude, waterfallOffset);
                meshData.vertices.append(color.redF());
                meshData.vertices.append(color.greenF());
                meshData.vertices.append(color.blueF());
            }
        }
        
        // Generate indices for triangle strips
        for (int t = 0; t < effectiveTimeSlices - 1; t++) {
            for (int f = 0; f < effectiveFreqBins - 1; f++) {
                int i0 = t * effectiveFreqBins + f;
                int i1 = t * effectiveFreqBins + (f + 1);
                int i2 = (t + 1) * effectiveFreqBins + f;
                int i3 = (t + 1) * effectiveFreqBins + (f + 1);
                
                // First triangle
                meshData.indices.append(i0);
                meshData.indices.append(i1);
                meshData.indices.append(i2);
                
                // Second triangle
                meshData.indices.append(i1);
                meshData.indices.append(i3);
                meshData.indices.append(i2);
            }
        }
        
        meshData.vertexCount = meshData.vertices.size() / 6;
        meshData.indexCount = meshData.indices.size();
        
        // Emit signal with generated mesh data
        emit meshReady(meshData);
    }
}

QColor MeshGeneratorWorker::amplitudeToColorWithOffset(float amplitude, float offset) {
    // Enhanced waterfall color algorithm
    float lowerThreshold = -160.0f + offset;
    float upperThreshold = -80.0f + offset;
    
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
