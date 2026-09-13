#include "WdspChannel.h"
#include <QDebug>

extern "C" {
#include <wdsp.h>
}

QRecursiveMutex WdspChannel::s_wdspWisdomMutex;

WdspChannel::WdspChannel(int channelId)
    : m_channelId(channelId)
{
}

WdspChannel::~WdspChannel()
{
    // Derived classes call their own close() implementations
}

void WdspChannel::stopChannel()
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetChannelState(m_channelId, 0, 0);
        m_isRunning.store(false, std::memory_order_release);
    }
}

void WdspChannel::setChannelState(int run, int wait)
{
    if (m_isOpen.load(std::memory_order_acquire)) {
        SetChannelState(m_channelId, run, wait);
        m_isRunning.store(run != 0, std::memory_order_release);
    }
}

void WdspChannel::setChannelStateById(int channelId, int run, int wait)
{
    SetChannelState(channelId, run, wait);
}
