#if !defined __HERMES_H__
#define      __HERMES_H__

#include "hpsdr.h"
#include "gui.h"

class HermesEthernet: public Ethernet {
public:
	HermesEthernet (Gui *p): Ethernet(), pg(p)
	{}

	void FatalError     (const char *pMsg) { pg->SetHw (pMsg);      };
	void TrasmissionTmo (const char *pMsg) { pg->AppendMessage (pMsg); };

	Gui *pg;
};

bool HermesInit (int sample_rate, HermesEthernet *);
//bool HermesStart (int nrx, const METIS_CARD *pCard, METIS_FATAL_ERROR_CALLBACK cbFatal, METIS_FATAL_ERROR_CALLBACK cbTrTmo );
bool HermesStart (int nrx, HermesEthernet *pHermes );

//bool HermesStop ();
bool HermesStop (HermesEthernet *pHermes);

const char *HermesGetFirmwareVersion (void);

bool HermesSetFreq (long);
bool HermesGetFreq (long &);

bool HermesSetSampleRate (int);
bool HermesGetSampleRate (int &sr);

bool HermesSetReceiversNumber (int n);

bool HermesSetAttenuator (int);
bool HermesSetAttenuator (int newAtt);
bool HermesSetPreamp (int p);
bool HermesSetDither (int d);
bool HermesSetRandomizer (int r);

//#include "metis.h"

//const METIS_CARD *HermesGetAddress(void);

#endif
