/**
* @file  test3d.cpp  
* @brief Simple test application for 3D Panadapter
*/

#include <QApplication>
#include <QMainWindow>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QtMath>

#include "src/cusdr_settings.h"
#include "src/GL/cusdr_ogl3DPanel.h"

class Test3DWindow : public QMainWindow {
    Q_OBJECT

public:
    Test3DWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        // Create settings
        settings = new Settings(this);
        
        // Create main widget
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        // Create layout
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);
        
        // Create 3D panel
        panel3D = new QGL3DPanel(this, settings);
        panel3D->setMinimumSize(800, 600);
        layout->addWidget(panel3D);
        
        // Add controls
        QLabel *label = new QLabel("3D Panadapter Test - Use mouse to rotate, wheel to zoom");
        layout->addWidget(label);
        
        QLabel *controlsLabel = new QLabel("Controls: R=reset camera, G=toggle grid, A=toggle axes, W=wireframe, +/- height scale");
        layout->addWidget(controlsLabel);
        
        // Generate fake spectrum data
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Test3DWindow::generateFakeData);
        timer->start(100);  // 10 FPS data updates
        
        setWindowTitle("cuSDR 3D Panadapter Test");
        resize(1000, 700);
        
        generateFakeData();  // Initial data
    }

private slots:
    void generateFakeData() {
        QVector<float> spectrum(512);
        static float time = 0.0f;
        time += 0.1f;
        
        // Generate some interesting fake spectrum data
        for (int i = 0; i < spectrum.size(); i++) {
            float freq = (float)i / spectrum.size();
            float noise = (rand() % 100) / 500.0f;  // Random noise
            
            // Create some "signals"
            float signal1 = 5.0f * exp(-50.0f * pow(freq - 0.3f, 2));  // Strong signal at 30%
            float signal2 = 3.0f * exp(-100.0f * pow(freq - 0.7f, 2)); // Medium signal at 70%
            float signal3 = 2.0f * sin(time + freq * 10) * exp(-20.0f * pow(freq - 0.5f, 2)); // Moving signal
            
            spectrum[i] = noise + signal1 + signal2 + signal3;
        }
        
        panel3D->setSpectrumData(spectrum);
    }

private:
    Settings *settings;
    QGL3DPanel *panel3D;
    QTimer *timer;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    Test3DWindow window;
    window.show();
    
    return app.exec();
}

#include "test3d.moc"