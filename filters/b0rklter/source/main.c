#include <nds.h>
#include <feos.h>
#include <common/filter.h>

FIFO_FLTR_MSG msg;
int fifoChan;

/*
 * Example of how you should NOT filter stuff
 * this pretty much b0rks your samplesXD (After
 * all this is the b0rklter)
 */
void filter()
{
	int i, j;
	s16* out = msg.buffer;
	if(msg.off >= (msg.bufLen/4)*3) {
		for(i=0; i<msg.nChans; i++) {
			for(j=msg.off; j<msg.off+msg.len; j++) {
				int off = (j&(msg.bufLen - 1))+i*msg.bufLen;
				out[off] = 0;
			}
		}
	}

	else {
		for(i=0; i<msg.nChans; i++) {
			for(j=msg.off; j<msg.off+msg.len; j++) {
					int off = j&(msg.bufLen - 1);
					out[off]|= (out[off]<<1);
			}
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
