/*
 * Based upon the example given @ http://ece124web.groups.et.byu.net/labs/HowTos/Simple%20Software%20Lowpass%20Filter.pdf
 * TODO: write my own implementation
 */
#include <nds.h>
#include <feos.h>
#include <common/filter.h>

FIFO_FLTR_MSG msg;
int fifoChan;

// support for two channels
int amps[2];
#define FILTER_SHIFT (4)

void filter()
{
	s16* out = msg.buffer;
	int i,j;
	for(i=0; i<msg.nChans; i++) {
		for(j=msg.off; j<msg.off+msg.len; j++) {
			int off = (j&(msg.bufLen - 1))+i*msg.bufLen;
			amps[i] = amps[i] - (amps[i]>>FILTER_SHIFT) + out[off];
			out[off] = amps[i]>>FILTER_SHIFT;
		}
	}

}

void FifoMsgHandler(int num_bytes, void *userdata)
{
	fifoGetDatamsg(fifoChan, num_bytes, (u8*)&msg);
	filter();
}

int arm7_main(int fifoCh)
{
	fifoChan = fifoCh;
	coopFifoSetDatamsgHandler(fifoCh,  FifoMsgHandler, 0);
	return 0;
}

void arm7_fini()
{
	fifoSetDatamsgHandler(fifoChan,  0, 0);
}
