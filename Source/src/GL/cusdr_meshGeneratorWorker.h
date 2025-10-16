/**
* @file  cusdr_meshGeneratorWorker.h
* @brief Threaded mesh generation worker for 3D Panadapter
* @author Simon Eatough, ZL2BRG
* @version 0.1
* @date 2025-10-16
*/

#ifndef _CUSDR_MESH_GENERATOR_WORKER_H
#define _CUSDR_MESH_GENERATOR_WORKER_H

#include <QThread>
#include <QVector>
#include <QMutex>
#include <QWaitCondition>
#include <QColor>

/**
 * @brief Worker class that generates 3D mesh data in a separate thread
 * 
 * This class offloads CPU-intensive mesh generation (vertex calculation,
 * color computation, index generation) from the main/GUI thread to improve
 * responsiveness and frame rates.
 */
class MeshGeneratorWorker : public QThread {
    Q_OBJECT

public:
    struct MeshData {
        QVector<float> vertices;
        QVector<unsigned int> indices;
        int vertexCount;
        int indexCount;
    };

    explicit MeshGeneratorWorker(QObject *parent = nullptr);
    ~MeshGeneratorWorker();

    // Request mesh generation with current parameters
    void generateMesh(const QVector<QVector<float>>& spectrumHistory,
                     int spectrumWidth,
                     float cameraDistance,
                     float heightScale,
                     float frequencyScale,
                     float timeScale,
                     float waterfallOffset,
                     int minTimeSlice,
                     int maxTimeSlice,
                     int minFreqBin,
                     int maxFreqBin);

    // Stop the worker thread
    void stop();

signals:
    // Emitted when mesh generation is complete
    void meshReady(const MeshData& meshData);

protected:
    void run() override;

private:
    // Color conversion (same as in main class)
    QColor amplitudeToColorWithOffset(float amplitude, float offset);

    // Generation state
    bool m_stop;
    bool m_newDataAvailable;
    QMutex m_mutex;
    QWaitCondition m_condition;

    // Input parameters (protected by mutex)
    QVector<QVector<float>> m_spectrumHistory;
    int m_spectrumWidth;
    float m_cameraDistance;
    float m_heightScale;
    float m_frequencyScale;
    float m_timeScale;
    float m_waterfallOffset;
    int m_minTimeSlice;
    int m_maxTimeSlice;
    int m_minFreqBin;
    int m_maxFreqBin;
    
    // Constants
    static const int MAX_TIME_SLICES = 300;
};

#endif // _CUSDR_MESH_GENERATOR_WORKER_H
