#include "OpenCLManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>

OpenCLManager& OpenCLManager::instance() {
    static OpenCLManager manager;
    return manager;
}

OpenCLManager::OpenCLManager()
    : m_initialized(false) {
    qDebug() << "OpenCLManager created.";
}

OpenCLManager::~OpenCLManager() {
    cleanup();
}

bool OpenCLManager::init() {
    if (m_initialized) return true;

    qDebug() << "OpenCL: Starting initialization...";

    try {
        // Get available platforms
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        if (platforms.empty()) {
            qDebug() << "OpenCL: No platforms found. Check your drivers.";
            return false;
        }

        qDebug() << "OpenCL: Found" << platforms.size() << "platforms.";
        m_platform = platforms.front();

        // Get GPU devices
        std::vector<cl::Device> devices;
        m_platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

        if (devices.empty()) {
            qDebug() << "OpenCL: No GPU devices found on first platform, trying CPU...";
            m_platform.getDevices(CL_DEVICE_TYPE_CPU, &devices);
        }

        if (devices.empty()) {
            qDebug() << "OpenCL: No devices found on platform" << QString::fromStdString(m_platform.getInfo<CL_PLATFORM_NAME>());
            return false;
        }

        m_device = devices.front();
        m_deviceName = QString::fromStdString(m_device.getInfo<CL_DEVICE_NAME>());
        qDebug() << "OpenCL: Selected device:" << m_deviceName;

        // Create context and command queue
        m_context = cl::Context(m_device);
        m_queue = cl::CommandQueue(m_context, m_device);

        // Load kernels from Qt Resource System (embedded in executable)
        if (!loadProgram(":/kernels.cl")) {
            return false;
        }

        // Initialize clFFT
        cl_int err = clfftInitSetupData(&m_fftSetup);
        err = clfftSetup(&m_fftSetup);
        if (err != CL_SUCCESS) {
            qDebug() << "clFFT Setup Error:" << err;
            return false;
        }

        qDebug() << "OpenCL and clFFT initialized on device:" << m_deviceName;
        m_initialized = true;
        return true;

    } catch (const cl::Error& e) {
        qDebug() << "OpenCL Error:" << e.what() << "(" << e.err() << ")";
        return false;
    }
}

bool OpenCLManager::loadProgram(const std::string& path) {
    try {
        QFile file(QString::fromStdString(path));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "OpenCL: Could not open kernel file:" << QString::fromStdString(path);
            return false;
        }

        QTextStream in(&file);
        QString source = in.readAll();
        std::string str = source.toStdString();

        cl::Program::Sources sources;
        sources.push_back({str.c_str(), str.length()});

        m_program = cl::Program(m_context, sources);
        std::vector<cl::Device> buildDevices;
        buildDevices.push_back(m_device);
        
        try {
            m_program.build(buildDevices);
        } catch (const cl::Error& e) {
            if (e.err() == CL_BUILD_PROGRAM_FAILURE) {
                QString log = QString::fromStdString(m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device));
                qDebug() << "OpenCL Build Error Log:\n" << log;
            }
            throw; // Re-throw to be caught by outer catch
        }

        // Pre-create kernels
        std::vector<cl::Kernel> allKernels;
        m_program.createKernels(&allKernels);
        for (auto& k : allKernels) {
            m_kernels[k.getInfo<CL_KERNEL_FUNCTION_NAME>()] = k;
        }

        return true;
    } catch (const cl::Error& e) {
        qDebug() << "OpenCL Program Load Error:" << e.what() << "(" << e.err() << ")";
        return false;
    }
}

cl::Kernel OpenCLManager::getKernel(const std::string& name) {
    if (m_kernels.find(name) != m_kernels.end()) {
        return m_kernels[name];
    }
    return cl::Kernel();
}

void OpenCLManager::cleanup() {
    if (m_initialized) {
        clfftTeardown();
        m_initialized = false;
    }
}
