/** 
 * @file ExtIO_hermes.cpp
 * @brief ExtIO_hermes.cpp : Defines the exported functions for the DLL application.
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2013-09-23
 */

/* Copyright (C) 
 * Andrea Montefusco IW0HDV
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#include "stdio.h"
#include "ExtIO_hermes.h"
#include "dllmain.h"

//
// Globals
//

#pragma data_seg (".SS_EXTIO_HERMES")

// !!!! have to be initialized due to shared segments rules constraints
char bufHR [sizeof(ExtioHermesRadio < EXTIO_BASE_TYPE >)]	= { 0 };
char bufHE [sizeof(HermesEthernet)]							= { 0 };

#pragma data_seg ()


IdllComm < EXTIO_BASE_TYPE, EXTIO_NS > *rxIQ = 0;

EXTIO_RX_CALLBACK ExtioCallback = 0;

HermesEthernet *pHermesEth = 0;

Gui *pGui = 0;

ExtioHermesRadio < EXTIO_BASE_TYPE > *pHermes = 0;

extern "C"
EXTIO_HERMES_API bool __stdcall InitHW(char *name, char *model, int & extio_type)
{
	LOG_OPEN ("hermes");
	LOG (("opened in InitHW(): %d\n", GetInstanceNumber() )) ;

	static bool first = true;
	EXTIO_BASE_TYPE extio_type_;
	Ethernet::Device *pDev = 0;

	extio_type = extio_type_.value;

	if (first) {
		if ( GetInstanceNumber() == 1 ) {
			first = false;
			ExtioCallback = NULL;

			pGui = new Gui (EXTIO_DEFAULT_SAMPLE_RATE);
			if (pHermesEth == 0) {
				// placement new used, in order to share it among all instances
				pHermes = new (bufHR) ExtioHermesRadio < EXTIO_BASE_TYPE > (EXTIO_NS);

				if (pHermes != 0) {
					pGui->setRadio (pHermes);
					pHermes->setSampleRate (EXTIO_DEFAULT_SAMPLE_RATE);
				}
				Flow *pFlow = new Flow (pHermes);
				pHermesEth = new HermesEthernet (pGui, pFlow);
			}
			Ethernet::scan_devices ();
	
			pDev = pHermesEth->found();
			//rc = HermesInit (DEFAULT_SAMPLE_RATE, pHermes);
			//	TextModeEthernet *pEth = new TextModeEthernet (pFlow);
			if (pDev != 0) {
				LOG (("HARDWARE FOUND !\n"));
				pGui->HermesSetHwAddressGUI (pDev);
			} else {
				LOG (("HARDWARE NOT FOUND !\n"));
				pGui->HermesSetHwAddressGUI (0);
			}
		} else {
			pHermesEth = (HermesEthernet *) bufHE;
			pDev = pHermesEth->found();
			// point local hermes radio object pointer to shared buffer
			pHermes = (ExtioHermesRadio < EXTIO_BASE_TYPE > *) bufHR;
		}
	}
	if (pDev != 0) {
		LOG(("Radio in use: %s %s %1.1f\n", pDev->ip_address, pDev->mac_address, ((float)pDev->code_version)/10.0f ));
		strcpy(name, "HPSDR");
		strcpy(model, "Hermes");
	} else {
		strcpy(name, "HARDWARE NOT OPERATING !!!");
		strcpy(model, "N/A");
	}
	LOG (("INSTANCE: %d Name: [%s] Model: [%s]\n", GetInstanceNumber(), name, model));
	return (pDev != 0);
}

extern "C"
EXTIO_HERMES_API bool __stdcall OpenHW()
{
	LOG (("OpenHW() called: #%d\n", GetInstanceNumber()));
	if ( GetInstanceNumber() == 1 ) pGui->Show ();
	return true;
}

extern "C"
EXTIO_HERMES_API int __stdcall StartHW(long LOfrequency)
{
	LOG (("StartHW() called with LOfreq: %d\n", LOfrequency));

	if ( GetInstanceNumber() == 1 ) {
		
		if (!pHermesEth->found()) return 0;

		int act_sr;
		pHermes->getSampleRate(act_sr);
		LOG (("StartHW() before starting receive: SAMPLE RATE; %d #rx: %d\n", act_sr, pGui->getRecNumber()));

		pHermes->setNumberOfRx (pGui->getRecNumber());

		pHermesEth->startReceive (pHermesEth->found());

		pHermes->setFrequency ( LOfrequency ) ;

		// not anymore necessary, already done in InitHW
		// pGui->HermesSetHwAddressGUI ( pHermesEth->found() );
	}
	
	//
	// all instances, even the first one, have to prepare to receive data from the main one
	//
	rxIQ = new IdllComm < EXTIO_BASE_TYPE, EXTIO_NS > (GetInstanceNumber() - 1, &ExtioCallback);

	if (rxIQ) {
		rxIQ->startReceive ();
	} else {
		LOG (("FATAL: rxIQ not created !\n"));
	}

	if ( GetInstanceNumber() == 1 ) {
		// in first instance setup the internal data sender
		pHermes->setIdllComm (new IntraComm());
	} else {
		//
		// needed in order to set callback !!!
		//
		//pHermes->setSampleRate (EXTIO_DEFAULT_SAMPLE_RATE);
	}

	// signal to the main instance that we have to increase the number of active receivers
	//pHermes->setNumberOfRx ( GetInstanceNumber() );
	
	return EXTIO_NS; // # of samples returned by callback
}

extern "C"
EXTIO_HERMES_API int __stdcall GetStatus()
{
	LOG (("GetStatus() called\n"));
	
	return 0;
}

extern "C"
EXTIO_HERMES_API void __stdcall StopHW()
{
	LOG (("StopHW() called\n"));
	if ( GetInstanceNumber() == 1 ) {
		pGui->AppendMessage (_strdup("Hardware stopped by user request.\n"));
		pHermesEth->stopReceive ();
	}
	return;
}

extern "C"
EXTIO_HERMES_API void __stdcall CloseHW()
{
	LOG (("CloseHW() called\n"));
	if ( GetInstanceNumber() == 1 ) delete pGui;
	LOG_CLOSE;
	return;
}

extern "C"
EXTIO_HERMES_API int __stdcall SetHWLO(long freq)
{
	LOG (("SetHWLO() called with freq: %d\n", freq));
	
	if (freq < 10000) return -10000;
	if (freq > 60000000) return 60000000;
	pHermes->setFrequency ( freq, GetInstanceNumber() - 1 ) ;
	return 0;
}

extern "C"
EXTIO_HERMES_API long __stdcall GetHWLO()
{
	long LOfreq;
	pHermes->getFrequency (LOfreq, GetInstanceNumber() - 1 );
	LOG (("GetHWLO() called: return LOfreq: %d\n", LOfreq));

	return LOfreq;
}

extern "C"
EXTIO_HERMES_API long __stdcall GetHWSR()
{
	int sr;
	pHermes->getSampleRate (sr);
	LOG (("GetHWSR() called: return: %d\n", sr));
	return sr;
}

extern "C"
EXTIO_HERMES_API void __stdcall SetCallback (EXTIO_RX_CALLBACK parentCallBack)
{
	LOG (("SetCallback() called [%p]\n", parentCallBack));

	ExtioCallback = parentCallBack;

	return;
}

extern "C"
EXTIO_HERMES_API void __stdcall ShowGUI()
{
	LOG(("ShowGUI() called\n"));

	if ( GetInstanceNumber() == 1 ) {
		pGui->Show ();
	}
	return;
}

extern "C"
EXTIO_HERMES_API void __stdcall HideGUI()
{
	LOG(("HideGUI() called\n"));
	
	if ( GetInstanceNumber() == 1 ) {
		pGui->Hide ();
	}
	return;
}

#if 0
extern "C"
EXTIO_HERMES_API void __stdcall IFLimitsChanged (long low, long high)
{
	LOG (("IFLimitsChanged() called\n"));
	
	return;
}

extern "C"
EXTIO_HERMES_API void __stdcall TuneChanged (long freq)
{
	LOG (("TuneChanged() called with freq: %d\n", freq));
	
    return;
}


extern "C"
EXTIO_HERMES_API void __stdcall RawDataReady(long samprate, int *Ldata, int *Rdata, int numsamples)
{
	return;
}

#endif


