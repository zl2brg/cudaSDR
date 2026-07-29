#!/usr/bin/env python3
"""Replay cudaSDR's Protocol 2 start sequence against hpsdrsim and report IQ flow.

Reproduces the DataProcessor::requestProtocol2ReceiverSetup() ordering so the
start handshake can be tested without driving the GUI.  Usage:

    p2_start_probe.py [--host IP] [--general-delay MS] [--runs N]
"""

import argparse
import socket
import struct
import sys
import time

DEVICE_PORT = 1024
DDC_PORT = 1025
DUC_PORT = 1026
HP_PORT = 1027
DDC0_SRC_PORT = 1035

RATE_KHZ = 48
RX_FREQ_HZ = 7050000


class Sequences:
    def __init__(self):
        self._counters = {}

    def next(self, port):
        value = self._counters.get(port, 0)
        self._counters[port] = value + 1
        return value


def general_packet(seq, split_reply_ports=False):
    pkt = bytearray(60)
    struct.pack_into("!I", pkt, 0, seq)
    pkt[4] = 0x00
    struct.pack_into("!H", pkt, 5, DDC_PORT)
    struct.pack_into("!H", pkt, 7, DUC_PORT)
    struct.pack_into("!H", pkt, 9, HP_PORT)
    if split_reply_ports:
        # Give the SDR->PC streams source ports of their own.  Left at 0 the
        # device falls back to defaults that collide with the PC->SDR receive
        # ports (HP status 1025 == DDC specific, mic 1026 == DUC specific,
        # wideband 1027 == high priority).
        struct.pack_into("!H", pkt, 11, 1060)  # HP status
        struct.pack_into("!H", pkt, 19, 1061)  # mic samples
        struct.pack_into("!H", pkt, 21, 1062)  # wideband ADC0
    struct.pack_into("!H", pkt, 13, 1028)
    struct.pack_into("!H", pkt, 15, 1029)
    struct.pack_into("!H", pkt, 17, DDC0_SRC_PORT)
    pkt[23] = 1     # wideband enable
    pkt[59] = 0x01  # Alex0 enable
    return bytes(pkt)


def ddc_specific_packet(seq, receivers=1):
    pkt = bytearray(1444)
    struct.pack_into("!I", pkt, 0, seq)
    pkt[4] = 1                              # number of ADCs
    pkt[7] = (1 << receivers) - 1           # DDC enable bitmask
    for ddc in range(receivers):
        base = 17 + 6 * ddc
        pkt[base] = 0                       # ADC0
        struct.pack_into("!H", pkt, base + 1, RATE_KHZ)
        pkt[base + 5] = 24                  # sample size
    return bytes(pkt)


def tx_specific_packet(seq):
    pkt = bytearray(60)
    struct.pack_into("!I", pkt, 0, seq)
    return bytes(pkt)


def hp_packet(seq, run, receivers=1):
    pkt = bytearray(1444)
    struct.pack_into("!I", pkt, 0, seq)
    pkt[4] = 0x01 if run else 0x00
    for ddc in range(receivers):
        struct.pack_into("!I", pkt, 9 + 4 * ddc, RX_FREQ_HZ)
    struct.pack_into("!I", pkt, 333, RX_FREQ_HZ)
    pkt[345] = 0
    return bytes(pkt)


def run_once(host, general_delay_ms, listen_s, resend_general, split_reply_ports=False):
    seqs = Sequences()
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", 0))
    sock.settimeout(0.2)
    local_port = sock.getsockname()[1]

    def send(payload, port):
        sock.sendto(payload, (host, port))

    # 0. General packet — latches the reply address and creates the HP listener.
    send(general_packet(seqs.next(DEVICE_PORT), split_reply_ports), DEVICE_PORT)
    time.sleep(general_delay_ms / 1000.0)

    # 1./2. DDC + TX specific.
    send(ddc_specific_packet(seqs.next(DDC_PORT)), DDC_PORT)
    send(tx_specific_packet(seqs.next(DUC_PORT)), DUC_PORT)

    # 3. High Priority with Run=0 so frequencies latch before Run is asserted.
    send(hp_packet(seqs.next(HP_PORT), run=False), HP_PORT)
    time.sleep(0.005)

    if resend_general:
        send(general_packet(seqs.next(DEVICE_PORT), split_reply_ports), DEVICE_PORT)
        time.sleep(0.020)

    # 4. Assert Run=1 with a full HP packet.
    send(hp_packet(seqs.next(HP_PORT), run=True), HP_PORT)

    # 5. Post-run burst so freshly spawned threads latch enable/rate/freq.
    time.sleep(0.050)
    for _ in range(5):
        time.sleep(0.020)
        send(ddc_specific_packet(seqs.next(DDC_PORT)), DDC_PORT)
        send(tx_specific_packet(seqs.next(DUC_PORT)), DUC_PORT)
        send(hp_packet(seqs.next(HP_PORT), run=True), HP_PORT)

    counts = {}
    deadline = time.time() + listen_s
    while time.time() < deadline:
        try:
            data, addr = sock.recvfrom(2048)
        except socket.timeout:
            continue
        counts[addr[1]] = counts.get(addr[1], 0) + 1

    # Leave the device stopped, as DataEngine::stop() does.
    send(hp_packet(seqs.next(HP_PORT), run=False), HP_PORT)
    sock.close()
    return local_port, counts


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--general-delay", type=float, default=0.0,
                    help="ms to wait after the General packet before HP packets")
    ap.add_argument("--resend-general", action="store_true",
                    help="re-send the General packet after HP Run=0")
    ap.add_argument("--split-reply-ports", action="store_true",
                    help="assign distinct SDR->PC source ports in the General packet")
    ap.add_argument("--runs", type=int, default=4)
    ap.add_argument("--listen", type=float, default=1.5)
    args = ap.parse_args()

    print(f"host={args.host} general_delay={args.general_delay}ms "
          f"split_reply_ports={args.split_reply_ports} runs={args.runs}")
    verdicts = []
    for attempt in range(1, args.runs + 1):
        local_port, counts = run_once(args.host, args.general_delay,
                                      args.listen, args.resend_general,
                                      args.split_reply_ports)
        iq = counts.get(DDC0_SRC_PORT, 0)
        verdicts.append(iq > 0)
        detail = ", ".join(f"port {p}: {c}" for p, c in sorted(counts.items())) or "nothing"
        print(f"  run {attempt}: localPort={local_port} IQ={iq:5d}  [{detail}]")
        time.sleep(0.5)

    ok = sum(verdicts)
    print(f"result: {ok}/{len(verdicts)} runs received DDC0 IQ")
    return 0 if ok == len(verdicts) else 1


if __name__ == "__main__":
    sys.exit(main())
