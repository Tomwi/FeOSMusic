#include <nds.h>
#include <feos.h>
#include <../common/include/filter.h>

FIFO_FLTR_MSG msg;
int fifoChan;

// support for two channels
int capacitor[2];

#define RC_CONSTANT (3)

void filter()
{
	/* no 8bit filtering */
	if(msg.bytSmp == 1)
		return;
	s16* out = msg.buffer;
	int i,j;
	for(i=0; i<msg.nChans; i++) {
		for(j=msg.off; j<msg.off+msg.len; j++) {
			int off = (j&(msg.bufLen - 1))+i*msg.bufLen;
			/* Capacitor slowly charges */
			capacitor[i] = ((out[off] - capacitor[i])>>RC_CONSTANT) + capacitor[i];
			out[off] = capacitor[i];
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
