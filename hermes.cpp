#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include <winsock.h>
#include "log.h"
#include "pthread.h"

#include "ExtIO_hermes.h"
#include "hpsdr.h"
#include "receiver.h"
#include "ozy.h"
#include "hermes.h"
#include "dllmain.h"   // for GetInstanceNumber()

static char* interface_ = "eth0";
static char* metisip    = "0.0.0.0";

int  sr = 0;


/**
 *   Hermes Initialization
 *
 *    phase 1:  discover
 *    phase 
 */
bool HermesInit (int sample_rate, HermesEthernet *pHermes)
{
	// search hardware over default interface 
	// starts a background thread
    //if (metis_discover(interface_,metisip) == false) return false;
	if ( Ethernet::scan_devices () == false ) return false;
	
	// select Hermes
	int hermes=1;
    ozy_set_hermes(hermes);
	// tx power to zero
    ozy_set_hermes_power(0);

	// we are able to process only one receiver
	int r = 1;
	//ozy_set_receivers(r);

	// sample rate fixed
	sr = sample_rate;
	ozy_set_sample_rate (sr);   // reset in StartHW  ????

	// wait for hardware connection
	//Sleep (1000);

	if (pHermes->found() == 0) {
		return false;
    } else {
		// select i/o buffer suitable for Hermes
		ozy_set_buffers(2, hermes); 
		return true;
	}
}



//bool HermesStart (int nrx, const METIS_CARD *pCard, METIS_FATAL_ERROR_CALLBACK cbFatal, METIS_FATAL_ERROR_CALLBACK cbTrTmo )
bool HermesStart (int nrx, HermesEthernet *pHermes )
{       
	init_receivers(nrx);
	ozy_set_sample_rate(sr); // set in MermesInit() ?????
	ozy_set_hardware (pHermes);
    //init_bandscope();  NO bandscope in Studio 1

    //create_listener_thread(); we don't need listeners, because the DSP processing is made locally

	// starts main UDP packet receive thread
	pHermes->startReceive (pHermes->found());
    
	return true;
}

bool HermesStop (HermesEthernet *pHermes)
{
	//metis_stop_receive_thread ();
	pHermes->stopReceive();
	return true;
}

bool HermesSetFreq (long newfreq)
{
	int rx = GetInstanceNumber() - 1;
	LOG(("Set Frequency: %ld Receiver #%d\n", newfreq, rx ));
	receiver[rx].frequency         = newfreq;
	receiver[rx].frequency_changed = 1;
	return true;
}

bool HermesGetFreq (long &f)
{
	f = receiver[0].frequency ;
	return true;
}

const char *HermesGetFirmwareVersion (void)
{
	static char buf [256];
	sprintf (buf, "Hermes %d", ozy_get_hermes_sw_ver() );
	LOGX("%s\r\n", buf);
	return buf;
}

#if 0
const METIS_CARD *HermesGetAddress(void)
{
	return metis_found();
}
#endif

bool HermesSetSampleRate (int newsr)
{
	sr = newsr;
	ozy_set_sample_rate(newsr);
	pCallback (-1, 100, 0., 0);
	return true;
}

bool HermesGetSampleRate (int &sr)
{
	sr = ozy_get_sample_rate();
	return true;
}

bool HermesSetReceiversNumber (int n)
{
	LOGX(" %d\n", n);
	ozy_set_receivers_in_use (n);
	return true;
}

bool HermesSetAttenuator (int newAtt)
{
	ozy_set_hermes_att(newAtt);
	return true;
}


bool HermesSetPreamp (int p)
{
	ozy_set_preamp(p);
	return true;
}


bool HermesSetDither (int d)
{
	ozy_set_dither(d);
	return true;
}

bool HermesSetRandomizer (int r)
{
	ozy_set_random(r);
	return true;
}