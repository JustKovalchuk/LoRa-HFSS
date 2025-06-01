"""
Embedded Python Blocks:

Each time this file is saved, GRC will instantiate the first class it finds
to get ports and parameters of your block. The arguments to __init__  will
be the parameters. All of them are required to have default values!
"""

import numpy as np
from gnuradio import gr
import numpy as np
import time

import random

import pmt

class blk(gr.basic_block):
    def __init__(self, freq=868000000):
        gr.basic_block.__init__(self,
            name='Freq Scheduler',
            in_sig=None,
            out_sig=None)
        self.base_freq = freq
        freq_start = 867000000 -freq
        freq_end = 869000000 -freq
        step = 100000
        frequencies = [round(freq_start + i * step, 3) for i in range(int((freq_end - freq_start) / step))]
        print(frequencies)
        seed = 123
        random.seed(123)  # симетричний ключ

        hop_sequence = []
        for _ in range(10):
           hop_sequence += random.sample(frequencies, len(frequencies))

        self.freqs = hop_sequence
        self.index = 0

        self.message_port_register_out(pmt.intern("msg"))
        self.message_port_register_in(pmt.intern("tick"))

        self.set_msg_handler(pmt.intern("tick"), self.handle_tick)

    def handle_tick(self, msg):
        print("cur freq", self.base_freq)
        freq = self.freqs[self.index]
        self.index = (self.index + 1) % len(self.freqs)

        # Надсилаємо PMT-пару у порт 'cmd'
        self.message_port_pub(
           pmt.intern("msg"),
           pmt.cons(pmt.intern("freq"), pmt.from_float(freq))
        )
