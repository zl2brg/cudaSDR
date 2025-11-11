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
    , m_waterfallOffset(0.0f)
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
                                                  float waterfallOffset,
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
    m_waterfallOffset = waterfallOffset;
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
        QVector<float> spectrumData = m_spectrumData;
        int spectrumWidth = m_spectrumWidth;
        int lodLevel = m_lodLevel;
        float frequencyScale = m_frequencyScale;
        float waterfallOffset = m_waterfallOffset;
        float dBmPanMin = m_dBmPanMin;
        float dBmPanMax = m_dBmPanMax;
        
        // Unused parameters (kept for API consistency)
        Q_UNUSED(m_timeIndex)
        Q_UNUSED(m_timeScale)
        
        m_newDataAvailable = false;
        m_processing = true;
        m_mutex.unlock();
        
        // Generate mesh for single slice (CPU-intensive part, no locks needed)
        MeshData meshData;
        
        if (spectrumData.isEmpty()) {
            continue;
        }
        
        int freqBins = spectrumWidth;
        
        // Apply LOD factor for frequency downsampling
        // LOD 0 = full detail, LOD 1 = half, LOD 2 = quarter
        int lodFactor = 1 << lodLevel;  // 2^lodLevel
        
        // Downsample based on LOD
        // For high bin counts, we downsample more aggressively
        if (freqBins > 4000 && lodLevel == 0) {
            lodFactor = 4;  // Force higher LOD for very high resolution
        } else if (freqBins > 2000 && lodLevel == 0) {
            lodFactor = 2;
        }
        
        int effectiveFreqBins = freqBins / lodFactor;
        
        // Pre-allocate memory - single row of vertices
        int estimatedVertices = effectiveFreqBins * 6; // 6 floats per vertex (pos + color)
        
        meshData.vertices.reserve(estimatedVertices);
        // No indices needed - we'll connect slices at render time or use separate indices per slice
        
        // Generate vertices for this single time slice
        for (int f = 0; f < effectiveFreqBins; f++) {
            // Peak-preserving LOD: Take MAXIMUM across frequency range
            float amplitude = -200.0f;
            
            for (int freqSample = 0; freqSample < lodFactor; freqSample++) {
                int actualFreqIndex = f * lodFactor + freqSample;
                if (actualFreqIndex < spectrumData.size()) {
                    amplitude = qMax(amplitude, spectrumData[actualFreqIndex]);
                }
            }
            
            // If no samples found, use default
            if (amplitude < -199.0f) {
                amplitude = -120.0f;
            }
            
            // Position (frequency, amplitude, time) with user-controllable scales
            float x = (f * lodFactor / (float)freqBins) * 400.0f * frequencyScale - 200.0f * frequencyScale;
            // Position mesh so dBmPanMin is at y=0 and dBmPanMax is at y=40
            float y = (amplitude - dBmPanMin) / (dBmPanMax - dBmPanMin) * 40.0f;
            // Z position is always 0 - offset applied at render time
            float z = 0.0f;
            
            meshData.vertices.append(x);
            meshData.vertices.append(y);
            meshData.vertices.append(z);
            
            // Color based on amplitude with waterfall offset
            QColor color = amplitudeToColorWithOffset(amplitude, waterfallOffset);
            meshData.vertices.append(color.redF());
            meshData.vertices.append(color.greenF());
            meshData.vertices.append(color.blueF());
        }
        
        // Each slice needs TWO rows to make a surface
        // Row 1 at Z=0 (front edge of this slice)
        // Row 2 at Z=-4.0 (back edge, which will align with front of next slice)
        // This eliminates gaps between slices
        
        int verticesPerRow = effectiveFreqBins;
        
        // Add second row of vertices at back edge (where next slice's front will be)
        // This is 4.0 units back, matching the spacing in renderSpectrum3D
        float zBackEdge = -4.0f;  // Back edge of slice (matches slice spacing)
        for (int f = 0; f < effectiveFreqBins; f++) {
            // Copy vertex data but with different Z
            int srcIdx = f * 6;
            meshData.vertices.append(meshData.vertices[srcIdx]);     // x
            meshData.vertices.append(meshData.vertices[srcIdx + 1]); // y
            meshData.vertices.append(zBackEdge);                      // z at back edge
            meshData.vertices.append(meshData.vertices[srcIdx + 3]); // r
            meshData.vertices.append(meshData.vertices[srcIdx + 4]); // g
            meshData.vertices.append(meshData.vertices[srcIdx + 5]); // b
        }
        
        // Now generate indices connecting the two rows
        for (int f = 0; f < effectiveFreqBins - 1; f++) {
            int i0 = f;
            int i1 = f + 1;
            int i2 = verticesPerRow + f;
            int i3 = verticesPerRow + f + 1;
            
            // First triangle
            meshData.indices.append(i0);
            meshData.indices.append(i1);
            meshData.indices.append(i2);
            
            // Second triangle
            meshData.indices.append(i1);
            meshData.indices.append(i3);
            meshData.indices.append(i2);
        }
        
        meshData.vertexCount = meshData.vertices.size() / 6;
        meshData.indexCount = meshData.indices.size();
        
        // Mark processing complete before emitting
        {
            QMutexLocker locker(&m_mutex);
            m_processing = false;
        }
        
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
