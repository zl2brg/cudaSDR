import re

# ── helpers ──────────────────────────────────────────────────────────────────

def repl(content, old, new):
    """Return content with old replaced by new, asserting exactly one match."""
    count = content.count(old)
    if count == 0:
        print(f"  MISS: {repr(old[:60])}")
        return content
    if count > 1:
        print(f"  MULTI({count}): {repr(old[:60])}")
    return content.replace(old, new, 1)

# ─────────────────────────────────────────────────────────────────────────────
# DataEngine.cpp  – simple io.mutex.lock / unlock pairs
# ─────────────────────────────────────────────────────────────────────────────
with open('/home/sae/Projects/Personal/cudaSDR/src/DataEngine/cusdr_dataEngine.cpp', 'r') as f:
    de = f.read()

# setSystemState – networkIOMutex wrapping one call
de = repl(de,
    '{\n\tio.networkIOMutex.lock();\n\tset->setSystemState(this, err, hwmode, statemode, enginestate);\n\tio.networkIOMutex.unlock();\n}',
    '{\n\tQMutexLocker locker(&io.networkIOMutex);\n\tset->setSystemState(this, err, hwmode, statemode, enginestate);\n}')

# rxListChanged
de = repl(de,
    '\tio.mutex.lock();\n\tRX = list;\n\tio.mutex.unlock();',
    '\tQMutexLocker locker(&io.mutex);\n\tRX = list;')

# setCurrentReceiver
de = repl(de,
    '\tio.mutex.lock();\n\tio.currentReceiver = rx;\n\tio.mutex.unlock();',
    '\tQMutexLocker locker(&io.mutex);\n\tio.currentReceiver = rx;')

# setSampleRate – large block, replace lock/unlock around the switch
de = repl(de,
    '\tio.mutex.lock();\n\n\tswitch (value) {',
    '\t{\n\tQMutexLocker locker(&io.mutex);\n\n\tswitch (value) {')

de = repl(de,
    '\tio.mutex.unlock();\n\n\tif (!applyOk) {',
    '\t} // QMutexLocker released here\n\n\tif (!applyOk) {')

# setMercuryAttenuators
de = repl(de,
    '\tio.mutex.lock();\n\tio.ccTx.mercuryAttenuators = attn;\n\tio.mutex.unlock();',
    '\tQMutexLocker locker(&io.mutex);\n\tio.ccTx.mercuryAttenuators = attn;')

# setDither
de = repl(de,
    '\tio.mutex.lock();\n\tio.ccTx.dither = value;\n\tio.mutex.unlock();',
    '\tQMutexLocker locker(&io.mutex);\n\tio.ccTx.dither = value;')

# setRandom
de = repl(de,
    '\tio.mutex.lock();\n\tio.ccTx.random = value;\n\tio.mutex.unlock();',
    '\tQMutexLocker locker(&io.mutex);\n\tio.ccTx.random = value;')

# set10MhzSource
de = repl(de,
    '\tio.mutex.lock();\n\tio.control_out[1] = io.control_out[1] & 0xF3;\n\tio.control_out[1] = io.control_out[1] | (source << 2);\n\tio.mutex.unlock();',
    '\tQMutexLocker locker(&io.mutex);\n\tio.control_out[1] = io.control_out[1] & 0xF3;\n\tio.control_out[1] = io.control_out[1] | (source << 2);')

# set122_88MhzSource
de = repl(de,
    '\tio.mutex.lock();\n\tio.control_out[1] = io.control_out[1] & 0xEF;\n\tio.control_out[1] = io.control_out[1] | (source << 4);\n\tio.mutex.unlock();',
    '\tQMutexLocker locker(&io.mutex);\n\tio.control_out[1] = io.control_out[1] & 0xEF;\n\tio.control_out[1] = io.control_out[1] | (source << 4);')

# setMicSource
de = repl(de,
    '\tio.mutex.lock();\n\tio.control_out[1] = io.control_out[1] & 0x7F;\n\tio.control_out[1] = io.control_out[1] | (source << 7);\n\tio.mutex.unlock();',
    '\tQMutexLocker locker(&io.mutex);\n\tio.control_out[1] = io.control_out[1] & 0x7F;\n\tio.control_out[1] = io.control_out[1] | (source << 7);')

# setMercuryClass
de = repl(de,
    '\tio.mutex.lock();\n\tio.rxClass = value;\n\tio.mutex.unlock();',
    '\tQMutexLocker locker(&io.mutex);\n\tio.rxClass = value;')

# setMercuryTiming
de = repl(de,
    '\tio.mutex.lock();\n\tio.timing = value;\n\tio.mutex.unlock();',
    '\tQMutexLocker locker(&io.mutex);\n\tio.timing = value;')

with open('/home/sae/Projects/Personal/cudaSDR/src/DataEngine/cusdr_dataEngine.cpp', 'w') as f:
    f.write(de)
print("cusdr_dataEngine.cpp done")

# ─────────────────────────────────────────────────────────────────────────────
# cusdr_dataIO.cpp  – simple lock/unlock pairs around debug logging
# ─────────────────────────────────────────────────────────────────────────────
with open('/home/sae/Projects/Personal/cudaSDR/src/DataEngine/cusdr_dataIO.cpp', 'r') as f:
    dio = f.read()

# stop() – single-line lock pair
dio = repl(dio,
    '    io->networkIOMutex.lock();\n        m_stopped = true;\n    io->networkIOMutex.unlock();',
    '    {\n        QMutexLocker locker(&io->networkIOMutex);\n        m_stopped = true;\n    }')

# sendInitFramesToDevice error branch
dio = repl(dio,
    '\t\tio->networkIOMutex.lock();\n\t\tDATAIO_DEBUG << "error sending init data to device: " << qPrintable(m_dataIOSocket->errorString());\n\t\tio->networkIOMutex.unlock();',
    '\t\t{ QMutexLocker l(&io->networkIOMutex); DATAIO_DEBUG << "error sending init data to device: " << qPrintable(m_dataIOSocket->errorString()); }')

dio = repl(dio,
    '\t\tio->networkIOMutex.lock();\n\t\tDATAIO_DEBUG << "init frames sent to network device. " << rx << " port " << port;\n\t\tio->networkIOMutex.unlock();',
    '\t\t{ QMutexLocker l(&io->networkIOMutex); DATAIO_DEBUG << "init frames sent to network device. " << rx << " port " << port; }')

# networkDeviceStartStop start command
dio = repl(dio,
    '\t\t\t\tio->networkIOMutex.lock();\n\t\t\t\tDATAIO_DEBUG << "sent start command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port;\n\t\t\t\tio->networkIOMutex.unlock();',
    '\t\t\t\t{ QMutexLocker l(&io->networkIOMutex); DATAIO_DEBUG << "sent start command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port; }')

dio = repl(dio,
    '\t\t\t\tio->networkIOMutex.lock();\n\t\t\t\tDATAIO_DEBUG << "sent stop command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port;\n\t\t\t\tio->networkIOMutex.unlock();',
    '\t\t\t\t{ QMutexLocker l(&io->networkIOMutex); DATAIO_DEBUG << "sent stop command to device at: "<< qPrintable(metis.ip_address.toString()) << " port " << port; }')

with open('/home/sae/Projects/Personal/cudaSDR/src/DataEngine/cusdr_dataIO.cpp', 'w') as f:
    f.write(dio)
print("cusdr_dataIO.cpp done")

# ─────────────────────────────────────────────────────────────────────────────
# CProtocol1.cpp  – wrap entire encodeCCBytes body with QMutexLocker
# ─────────────────────────────────────────────────────────────────────────────
with open('/home/sae/Projects/Personal/cudaSDR/src/DataEngine/CProtocol1.cpp', 'r') as f:
    p1 = f.read()

p1 = repl(p1,
    '    io->mutex.lock();\n    switch (sendState) {',
    '    QMutexLocker locker(&io->mutex);\n    switch (sendState) {')

# The unlock is at the bottom of the function – remove it (QMutexLocker handles it)
p1 = repl(p1,
    '    io->mutex.unlock();\n}\n\nQByteArray CProtocol1::formatStartStop',
    '}\n\nQByteArray CProtocol1::formatStartStop')

with open('/home/sae/Projects/Personal/cudaSDR/src/DataEngine/CProtocol1.cpp', 'w') as f:
    f.write(p1)
print("CProtocol1.cpp done")

# ─────────────────────────────────────────────────────────────────────────────
# CProtocol2.cpp  – wrap entire encodeCCBytes body with QMutexLocker
# ─────────────────────────────────────────────────────────────────────────────
with open('/home/sae/Projects/Personal/cudaSDR/src/DataEngine/CProtocol2.cpp', 'r') as f:
    p2 = f.read()

p2 = repl(p2,
    '    io->mutex.lock();\n    // Protocol 2 High Priority and DDC packets must be 1444 bytes.',
    '    QMutexLocker locker(&io->mutex);\n    // Protocol 2 High Priority and DDC packets must be 1444 bytes.')

# Find and remove the matching unlock – it's near the end of encodeCCBytes.
# The last io->mutex.unlock() before the next top-level function definition.
# Find it after the big switch.
p2 = repl(p2,
    '    io->mutex.unlock();\n}\n\nQByteArray CProtocol2::formatStartStop',
    '}\n\nQByteArray CProtocol2::formatStartStop')

with open('/home/sae/Projects/Personal/cudaSDR/src/DataEngine/CProtocol2.cpp', 'w') as f:
    f.write(p2)
print("CProtocol2.cpp done")
