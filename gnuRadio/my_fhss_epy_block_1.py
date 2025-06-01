"""
Embedded Python Blocks:

Each time this file is saved, GRC will instantiate the first class it finds
to get ports and parameters of your block. The arguments to __init__  will
be the parameters. All of them are required to have default values!
"""

import numpy as np
from gnuradio import gr


class blk(gr.sync_block):  # other base classes are basic_block, decim_block, interp_block
    """Embedded Python Block example - a simple multiply const"""

    def __init__(self):  # only default arguments here
        """arguments to this function show up as parameters in GRC"""
        gr.sync_block.__init__(
            self,
            name='Freq Hoping',
            in_sig=[],
            out_sig=[]
        )

    def work(self, input_items, output_items):
        return 0

import time

freqs = [100e3, 200e3, 300e3, 400e3]
start_time = time.time()

def get_freq():
    now = time.time()
    idx = int(((now - start_time) * 10) % len(freqs))  # кожні 0.1 с
    return freqs[idx]
