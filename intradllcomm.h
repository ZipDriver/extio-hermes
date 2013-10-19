#if !defined __INTRACOMM_H__
#define      __INTRACOMM_H__

#include "pthread.h"

class IntraComm {
public:

	IntraComm (unsigned channel);
	IntraComm ();
	virtual ~IntraComm ();

	int send (unsigned channel, unsigned char *buf, int len);
	virtual int receive (unsigned channel, unsigned char *buf, int len);
	int startReceive ();

	static const int portBase = 37000;
	static const int NSOCK    = 8;
private:

	pthread_t receiveTid;
	static void *rxThread (void *);
	unsigned channel;
	int ss;
	int iqSocket [NSOCK];

};


#endif