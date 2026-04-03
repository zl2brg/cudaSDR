#ifndef OPENCLMANAGER_H
#define OPENCLMANAGER_H

#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>
#include <clFFT.h>
#include <QString>
#include <QVector>
#include <QDebug>
#include <map>

class OpenCLManager {
public:
    static OpenCLManager& instance();

    bool init();
    void cleanup();

    bool isInitialized() const { return m_initialized; }
    QString deviceName() const { return m_deviceName; }

    cl::Context& context() { return m_context; }
    cl::Device& device() { return m_device; }
    cl::CommandQueue& queue() { return m_queue; }

    cl::Kernel getKernel(const std::string& name);

private:
    OpenCLManager();
    ~OpenCLManager();

    bool loadProgram(const std::string& path);

    bool m_initialized;
    QString m_deviceName;
    cl::Platform m_platform;
    cl::Device m_device;
    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program m_program;
    std::map<std::string, cl::Kernel> m_kernels;
    
    clfftSetupData m_fftSetup;
};

#endif // OPENCLMANAGER_H
