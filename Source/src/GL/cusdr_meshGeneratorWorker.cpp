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
                                       float waterfallOffset,
                                       int minTimeSlice,
                                       int maxTimeSlice,
                                       int minFreqBin,
                                       int maxFreqBin,
                                       float dBmPanMin,
                                       float dBmPanMax) {
    QMutexLocker locker(&m_mutex);
    
    // Update parameters
    m_spectrumHistory = spectrumHistory;  // Qt implicit sharing - efficient copy
    m_spectrumWidth = spectrumWidth;
    m_cameraDistance = cameraDistance;
    m_heightScale = heightScale;
    m_frequencyScale = frequencyScale;
    m_timeScale = timeScale;
    m_waterfallOffset = waterfallOffset;
    m_minTimeSlice = minTimeSlice;
    m_maxTimeSlice = maxTimeSlice;
    m_minFreqBin = minFreqBin;
    m_maxFreqBin = maxFreqBin;
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
        QVector<QVector<float>> spectrumHistory = m_spectrumHistory;
        int spectrumWidth = m_spectrumWidth;
        float cameraDistance = m_cameraDistance;
        float frequencyScale = m_frequencyScale;
        float timeScale = m_timeScale;
        float waterfallOffset = m_waterfallOffset;
        int minTimeSlice = m_minTimeSlice;
        int maxTimeSlice = m_maxTimeSlice;
        int minFreqBin = m_minFreqBin;
        int maxFreqBin = m_maxFreqBin;
        float dBmPanMin = m_dBmPanMin;
        float dBmPanMax = m_dBmPanMax;
        
        m_newDataAvailable = false;
        m_processing = true;
        m_mutex.unlock();
        
        // Generate mesh (CPU-intensive part, no locks needed)
        MeshData meshData;
        
        if (spectrumHistory.isEmpty()) {
            continue;
        }
        
        int timeSlices = qMin(spectrumHistory.size(), MAX_TIME_SLICES);
        int freqBins = spectrumWidth;
        
        // Clamp visible range to valid bounds
        minTimeSlice = qMax(0, minTimeSlice);
        maxTimeSlice = qMin(timeSlices, maxTimeSlice);
        minFreqBin = qMax(0, minFreqBin);
        maxFreqBin = qMin(freqBins, maxFreqBin);
        
        int visibleTimeSlices = maxTimeSlice - minTimeSlice;
        int visibleFreqBins = maxFreqBin - minFreqBin;
        
        if (visibleTimeSlices <= 0 || visibleFreqBins <= 0) {
            continue; // Nothing visible
        }
        
        // Level of Detail (LOD) optimization based on camera distance AND frequency density
        // Similar to computeDisplayBins() in 2D panadapter
        
        // Camera distance LOD
        int distanceLodFactor = 1;
        if (cameraDistance > 250.0f) {
            distanceLodFactor = 4;  // Very far - skip 3 out of 4 points
        } else if (cameraDistance > 150.0f) {
            distanceLodFactor = 2;  // Medium distance - skip every other point
        }
        
        // Frequency density LOD - downsample if we have way more bins than needed
        // Target ~800-1000 frequency bins for display (similar to 2D panadapter)
        int frequencyLodFactor = 1;
        if (visibleFreqBins > 4000) {
            frequencyLodFactor = 4;  // 4096 bins -> ~1024 display bins
        } else if (visibleFreqBins > 2000) {
            frequencyLodFactor = 2;  // 2048+ bins -> ~1024 display bins
        }
        
        // Time LOD - similar logic for time slices
        int timeLodFactor = 1;
        if (visibleTimeSlices > 80) {
            timeLodFactor = 2;  // Reduce if we have many time slices
        }
        
        // Combine LOD factors (use max to avoid over-reduction)
        int freqLodFactor = qMax(distanceLodFactor, frequencyLodFactor);
        int timeLodFactorFinal = qMax(distanceLodFactor, timeLodFactor);
        
        int effectiveFreqBins = visibleFreqBins / freqLodFactor;
        int effectiveTimeSlices = visibleTimeSlices / timeLodFactorFinal;
        
        // Pre-allocate memory for efficiency
        int estimatedVertices = effectiveTimeSlices * effectiveFreqBins * 6; // 6 floats per vertex
        int estimatedIndices = (effectiveTimeSlices - 1) * (effectiveFreqBins - 1) * 6; // 6 indices per quad
        
        meshData.vertices.reserve(estimatedVertices);
        meshData.indices.reserve(estimatedIndices);
        
        // Generate vertices with peak-preserving LOD (only for visible region)
        for (int t = 0; t < effectiveTimeSlices; t++) {
            for (int f = 0; f < effectiveFreqBins; f++) {
                // Peak-preserving LOD: Take MAXIMUM across both time and frequency ranges
                float amplitude = -200.0f;
                
                // Loop through time samples in this LOD bin (offset by minTimeSlice)
                for (int timeSample = 0; timeSample < timeLodFactorFinal; timeSample++) {
                    int actualTimeIndex = minTimeSlice + t * timeLodFactorFinal + timeSample;
                    if (actualTimeIndex >= spectrumHistory.size()) break;
                    
                    const QVector<float>& spectrum = spectrumHistory[actualTimeIndex];
                    
                    // Loop through frequency samples in this LOD bin (offset by minFreqBin)
                    for (int freqSample = 0; freqSample < freqLodFactor; freqSample++) {
                        int actualFreqIndex = minFreqBin + f * freqLodFactor + freqSample;
                        if (actualFreqIndex < spectrum.size()) {
                            amplitude = qMax(amplitude, spectrum[actualFreqIndex]);
                        }
                    }
                }
                
                // If no samples found, use default
                if (amplitude < -199.0f) {
                    amplitude = -120.0f;
                }
                
                // Calculate global position (where this bin sits in the full spectrum)
                float globalF = minFreqBin + f * freqLodFactor;
                float globalT = minTimeSlice + t * timeLodFactorFinal;
                
                // Position (frequency, amplitude, time) with user-controllable scales
                // Use global positions to maintain correct world coordinates
                float x = (globalF / (float)freqBins) * 400.0f * frequencyScale - 200.0f * frequencyScale;
                // Position mesh so dBmPanMin is at y=0 and dBmPanMax is at y=40
                // When scale moves up/down, mesh moves with it
                // When scale expands/compresses, signals scale to maintain accurate positioning
                float y = (amplitude - dBmPanMin) / (dBmPanMax - dBmPanMin) * 40.0f;
                float z = ((float)timeSlices - globalT) * 4.0f * timeScale - (float)timeSlices * 2.0f * timeScale;
                
                meshData.vertices.append(x);
                meshData.vertices.append(y);
                meshData.vertices.append(z);
                
                // Color based on amplitude with waterfall offset
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
