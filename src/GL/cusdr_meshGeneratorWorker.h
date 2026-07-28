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
        int frontRowVertexCount = 0;  // skyline vertices for ridge line strip
    };

    explicit MeshGeneratorWorker(QObject *parent = nullptr);
    ~MeshGeneratorWorker();

    // Generate mesh for a single spectrum slice
    void generateSingleSliceMesh(const QVector<float>& spectrumData,
                                 int timeIndex,
                                 int spectrumWidth,
                                 int lodLevel,
                                 float heightScale,
                                 float frequencyScale,
                                 float timeScale,
                                 float colorLower,
                                 float colorUpper,
                                 float dBmPanMin,
                                 float dBmPanMax);

    // Synchronous mesh build (same logic as worker thread, for scale/color refresh).
    // timeScale must match QGL3DPanel slice spacing (4.0 * timeScale) so ribbons
    // do not overlap in Z and depth-test-hide peaks at grazing view angles.
    static MeshData buildSliceMesh(const QVector<float>& spectrumData,
                                   int spectrumWidth,
                                   int lodLevel,
                                   float frequencyScale,
                                   float timeScale,
                                   float colorLower,
                                   float colorUpper,
                                   float dBmPanMin,
                                   float dBmPanMax);

    // Stop the worker thread
    void stop();
    
    // Check if worker is currently processing
    bool isBusy() const;

signals:
    // Emitted when mesh generation is complete
    void meshReady(const MeshData& meshData);

protected:
    void run() override;

private:
    static QColor amplitudeToColorWithThresholds(float amplitude,
                                                 float lowerThreshold,
                                                 float upperThreshold);

    // Generation state
    bool m_stop;
    bool m_newDataAvailable;
    mutable QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_processing;  // True when actively generating mesh

    // Input parameters (protected by mutex)
    QVector<float> m_spectrumData;  // Single slice data
    int m_timeIndex;
    int m_spectrumWidth;
    int m_lodLevel;
    float m_heightScale;
    float m_frequencyScale;
    float m_timeScale;
    float m_colorLower;
    float m_colorUpper;
    float m_dBmPanMin;
    float m_dBmPanMax;
    
    // Constants
    static const int MAX_TIME_SLICES = 300;
};

#endif // _CUSDR_MESH_GENERATOR_WORKER_H
