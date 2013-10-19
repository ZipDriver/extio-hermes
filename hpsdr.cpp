/** 
 * @file hpsdr.cpp
 * @brief HPSDR Hermes modeling classes
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

#include "hpsdr.h"
#include <stdlib.h>
#include <stdio.h>
#include <strsafe.h>
#include <string.h>
#include <errno.h>
#include "log.h"
#include "util.h"

//
// Ethernet class data
//
#pragma data_seg (".SS_HPSDR")
struct Ethernet::Device Ethernet :: devs [MAX_DEVICES] = {0};
struct Ethernet::NetInterface Ethernet :: interfaces[MAX_INTERFACES] = {0};
int Ethernet :: nif   = 0;
int Ethernet :: dev_found = 0;
#pragma data_seg ()

#define ARRAY_SIZE(x) ((sizeof(x))/(sizeof(x[0])))

struct Ethernet::Device * Ethernet :: search_dev_by_ip (const char * ip)
{
	for (int i = 0; i < ARRAY_SIZE(devs); ++i)
		if (strcmp(ip, devs[i].ip_address) == 0) return &(devs[i]);
	return 0;
}

std::list < struct Ethernet::NetInterface > Ethernet :: getInterfaceList ()
{
	std::list < struct NetInterface > rl;

	for (int i = 0; i < nif; ++i) rl.push_back (interfaces[i]);
	return rl;
}

std::list < struct Ethernet::Device > Ethernet :: getDeviceList ()
{
	std::list < struct Device > dl;

	for (int i = 0; i < dev_found; ++i) dl.push_back (devs[i]);
	return dl;
}

struct Ethernet::Device * Ethernet :: found ()
{
	if (dev_found) return &devs[0];
	return 0;
}

//
// constants for scan_interface
//

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

int Ethernet :: scan_interface (int /* x */, char * /* ifName */  )
{
	/* Declare and initialize variables */

    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    unsigned int i = 0;

	int found_interface = 0;

	//int                nRet;
    //struct hostent    *phe;
    //struct in_addr    *paddr;

    // Set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

    // default to unspecified address family (both)
    ULONG family = AF_UNSPEC;

    LPVOID lpMsgBuf = NULL;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG Iterations = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
    PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
    IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
    IP_ADAPTER_PREFIX *pPrefix = NULL;

    family = AF_INET; // IP v4 only !

	LOG(("Calling GetAdaptersAddresses function with family = \n"));
	if (family == AF_INET) {LOG(("AF_INET\n"));}
    if (family == AF_INET6) {LOG(("AF_INET6\n"));}
    if (family == AF_UNSPEC) {LOG(("AF_UNSPEC\n"));}

    // Allocate a 15 KB buffer to start with.
    outBufLen = WORKING_BUFFER_SIZE;

    do {

        pAddresses = (IP_ADAPTER_ADDRESSES *) malloc(outBufLen);
        if (pAddresses == NULL) {
            LOG(("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n"));
            return 0;
        }

        dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            pAddresses = NULL;
        } else {
            break;
        }

        Iterations++;

    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

    if (dwRetVal == NO_ERROR) {
        // If successful, output some information from the data we received
        pCurrAddresses = pAddresses;
        while (pCurrAddresses && !found_interface) {
            LOGX("Length of the IP_ADAPTER_ADDRESS struct: %ld\n",
                   pCurrAddresses->Length);
            LOGX("IfIndex (IPv4 interface): %u\n", pCurrAddresses->IfIndex);
            LOGX("Adapter name: %s\n", pCurrAddresses->AdapterName);
			LOGX("Friendly name: %wS\n", pCurrAddresses->FriendlyName);
            pUnicast = pCurrAddresses->FirstUnicastAddress;
            if (pUnicast != NULL) {
                LOGX("ADDRESS: %s\n",  inet_ntoa ( ((struct sockaddr_in *)(pUnicast->Address.lpSockaddr))->sin_addr ) );
#if 0
				if ((!wcscmp(pCurrAddresses->FriendlyName, L"Ethernet"))
					 &&
					(strcmp(inet_ntoa ( ((struct sockaddr_in *)(pUnicast->Address.lpSockaddr))->sin_addr ), "127.0.0.1") != 0)) found_interface = 1;
#endif
				memcpy (&(interfaces[nif].b_ip_address), &(((struct sockaddr_in *)(pUnicast->Address.lpSockaddr))->sin_addr ), sizeof(struct in_addr) );
				strcpy (interfaces[nif].ip_address,  inet_ntoa ( ((struct sockaddr_in *)(pUnicast->Address.lpSockaddr))->sin_addr ) );
				for (i = 0; pUnicast != NULL; i++)
                    pUnicast = pUnicast->Next;
                LOGX("Number of Unicast Addresses: %d\n", i);
            } else {
                LOGX("%s", "No Unicast Addresses\n");
			}

            if (pCurrAddresses->PhysicalAddressLength != 0) {

			    for(i=0;i<6;i++) interfaces[nif].hw_address[i]=pCurrAddresses->PhysicalAddress[i];

				LOG(("Physical address: %02X-%02X-%02X-%02X-%02X-%02X\n", (interfaces[nif].hw_address[0]) & 0xFF,  (interfaces[nif].hw_address[1]) & 0xFF, (interfaces[nif].hw_address[2]) & 0xFF,
					 (interfaces[nif].hw_address[3]) & 0xFF, (interfaces[nif].hw_address[4]) & 0xFF, (interfaces[nif].hw_address[5]) & 0xFF
					));
            }
			LOGX("Friendly name: [%wS]\n", pCurrAddresses->FriendlyName);
			{
				// Convert to a char*
				size_t origsize = wcslen(pCurrAddresses->FriendlyName) + 1;
                const size_t newsize = sizeof (interfaces[nif].name);
				size_t convertedChars = 0;
				wcstombs_s(&convertedChars, interfaces[nif].name, wcslen(pCurrAddresses->FriendlyName) + 1, pCurrAddresses->FriendlyName, _TRUNCATE);
    		}
			nif++;
			pCurrAddresses = pCurrAddresses->Next;
        }
    } else {
        LOGX("Call to GetAdaptersAddresses failed with error: %d\n", dwRetVal);
        if (dwRetVal == ERROR_NO_DATA)
            LOGX("%s", "No addresses were found for the requested parameters\n");
        else {

            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                    NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   
                    // Default language
                    (LPTSTR) & lpMsgBuf, 0, NULL)) {
                LOGX("Error: %s", lpMsgBuf);
                LocalFree(lpMsgBuf);
                if (pAddresses)
                    free(pAddresses);
                return 0;
            }
        }
    }

    if (pAddresses) {
        free(pAddresses);
    }
	return nif;
}

#define PORT 1024
#define DISCOVERY_SEND_PORT PORT
#define DISCOVERY_RECEIVE_PORT PORT
#define DATA_PORT PORT

bool Ethernet :: scan_devices ()   // was discover()
{
    int rc;
    int i;
	static int data_socket = -1;
	static struct sockaddr_in data_addr;
	static int data_addr_length;
	static unsigned char buffer[70];

 
    LOGX("%s\n", "Looking for Metis card on all interfaces");

//    discovering=1;
	
    // get my MAC address and IP address
    if ( scan_interface (0, 0) <= 0) return false;

		for (int n=0; n < nif; n++) {

			LOG(("Interface [%s]: IP Address: %s\n", interfaces[n].name, interfaces[n].ip_address));

            LOG(("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                interfaces[n].hw_address[0]&0xFF, interfaces[n].hw_address[1]&0xFF, interfaces[n].hw_address[2]&0xFF, 
				interfaces[n].hw_address[3]&0xFF, interfaces[n].hw_address[4]&0xFF, interfaces[n].hw_address[5]&0xFF ));

			// bind to this interface

		    struct sockaddr_in name = {0} ;
			struct sockaddr_in discovery_addr = {0} ;
            int discovery_length = 0 ;

			interfaces[n].d_socket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (interfaces[n].d_socket < 0) {
				LOGX("create socket failed for d_socket %s\n", strerror(errno));
				continue;
			}

			name.sin_family      = AF_INET;
			name.sin_addr.s_addr = inet_addr(interfaces[n].ip_address);
			name.sin_port        = htons(DISCOVERY_SEND_PORT);
			bind (interfaces[n].d_socket, (struct sockaddr*)&name, sizeof(name));


		    // allow broadcast on the socket
			int broadcast = 1;
			rc = setsockopt (interfaces[n].d_socket, SOL_SOCKET, SO_BROADCAST, (const char *)&broadcast, sizeof(broadcast));
			if(rc != 0) {
				LOGX("cannot set SO_BROADCAST: rc=%d\n", rc);
				return false;
			}
			int rcv_tmo = 10;
			rc = setsockopt (interfaces[n].d_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&rcv_tmo, sizeof(int));
			if ( rc == SOCKET_ERROR ) {
                LOGX("setsockopt SO_RCVTIMEO failed! [%s]\n", strerror(errno));
			}

			discovery_length=sizeof(discovery_addr);
			memset(&discovery_addr,0,discovery_length);
			discovery_addr.sin_family=AF_INET;
			discovery_addr.sin_port=htons(DISCOVERY_SEND_PORT);
			discovery_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);

			for (int x=0; x<5; ++x) {
				// broadcast the discovery message 
				buffer[0]=0xEF;
				buffer[1]=0xFE;
				buffer[2]=0x02;         
				for (i=0; i<60; i++) buffer[i+3]=0x00;

				if (sendto(interfaces[n].d_socket, (const char *)buffer,63,0,(struct sockaddr*)&discovery_addr,discovery_length)<0) {
					perror("sendto socket failed for discovery_socket\n");
					continue;
				} else {
					LOGX ("#%d discovery message sent.\r\n", x);
				} 
//				Sleep (100);
				// reading answers with 100 ms timeout
			    struct sockaddr_in from_addr;
				unsigned long my_ip_address = inet_addr(interfaces[n].ip_address);
				int length;
				int bytes_read;
				char *rc = 0;

				for (int nr = 0; nr < 3; ++nr) {
					unsigned char inp_buf[1024];

					length = sizeof(from_addr);

   					bytes_read = recvfrom (interfaces[n].d_socket, (char *) inp_buf, sizeof(inp_buf), 0, (struct sockaddr*)&from_addr, &length);
					if (bytes_read < 0) {
						LOGX("Reading discovery: %s\n", strerror(errno));
						continue;
					}
					//LOG(("RECEIVED %d bytes: from [%s]\n", bytes_read, inet_ntoa ( ((struct sockaddr_in *)&from_addr)->sin_addr)) );

					if (memcmp ( &(((struct sockaddr_in *)&from_addr)->sin_addr), &my_ip_address, sizeof(my_ip_address)) == 0) {
						LOG (("WARNING: ignoring fake answer coming from ourselves !\n"));
						continue;
					}

					if (inp_buf[0]==0xEF && inp_buf[1]==0xFE) {
						switch(inp_buf[2]) {
							case 1:
								LOGX("%s", "unexpected data packet when in discovery mode\n");
								break;
							case 2:  // response to a discovery packet - hardware is not yet sending
							case 3:  // response to a discovery packet - hardware is already sending
								if (dev_found < MAX_DEVICES) {
									struct Device card;

									if (inp_buf[3] == 0 && inp_buf[4] == 0 && inp_buf[5] == 0 &&
										inp_buf[6] == 0 && inp_buf[7] == 0 && inp_buf[8] == 0) {
										LOGX("%s", "NULL MAC address in answer, skipping\n");
										break;
									}

									// get MAC address from reply
									sprintf(card.mac_address,"%02X:%02X:%02X:%02X:%02X:%02X",
                                            inp_buf[3]&0xFF, inp_buf[4]&0xFF, inp_buf[5]&0xFF, inp_buf[6]&0xFF, inp_buf[7]&0xFF, inp_buf[8]&0xFF);
									LOGX("Radio MAC address %s\n", card.mac_address);
    
									// get ip address from packet header
									sprintf (card.ip_address,"%d.%d.%d.%d",
											 from_addr.sin_addr.s_addr      & 0xFF,
											(from_addr.sin_addr.s_addr>>8)  & 0xFF,
											(from_addr.sin_addr.s_addr>>16) & 0xFF,
											(from_addr.sin_addr.s_addr>>24) & 0xFF);
									LOGX("Radio IP address %s\n", card.ip_address);
									card.code_version = inp_buf[9];
									switch (inp_buf[10]) {
										case 0x00:
											snprintf (card.board_id, sizeof(card.board_id), "%s", "Metis" );
											break;
										case 0x01:
											snprintf (card.board_id, sizeof(card.board_id), "%s", "Hermes" );
											break;
										case 0x02:
											snprintf (card.board_id, sizeof(card.board_id), "%s", "Griffin" );
											break;
										case 0x04:
											snprintf (card.board_id, sizeof(card.board_id), "%s", "Angelia" );
											break;
										default: 
											snprintf (card.board_id, sizeof(card.board_id), "%s", "unknown" );
											break;
									}       
									LOGX("***** Board id: %s\n",    card.board_id);
									LOGX("***** version:  %1.2f\n", card.code_version /10.0);

									card.b_card_ip_address = interfaces[n].b_ip_address;

									// check if this device was already discovered
									if ( search_dev_by_ip (card.ip_address) == 0) {
										devs[dev_found] = card;
										dev_found++;
									} else {
										LOGX("%s", "Duplicated response, discard\n");
									}


									if(inp_buf[2]==3) {
										LOGX("%s", "Radio is sending\n");
									}
								} else {
									LOGX("%s", "too many radio cards!\n");
									break;
								}
							break;
							default:
								LOGX("unexpected packet type: 0x%02X\n",inp_buf[2]);
								break;
							}
						} else {
							LOG(("received bad header bytes on data port %02X,%02X\n", inp_buf[0], inp_buf[1] ));
						}
				}
	
		}
		closesocket (interfaces[n].d_socket);
	}
	return true;
}

void	Ethernet :: startReceive (struct Device *p)
///////////////	void  metis_start_receive_thread (const METIS_CARD *pCard, METIS_FATAL_ERROR_CALLBACK fecb, METIS_FATAL_ERROR_CALLBACK ttmocb)
{
    int i;
    int rc;
    struct hostent *h;
	unsigned char buffer [64];

    LOGX("%s", "Metis starting receive thread\n");

    //discovering = 0;

    h = gethostbyname (p->ip_address);
    if (h == NULL) {
        LOGX("metis_start_receiver_thread: unknown host %s\n", p->ip_address);
        return;
    } else {
        LOGX("metis_start_receiver_thread on host %s\n", p->ip_address);
	}

	//
	// check if socket is yet available (after a STOP)
	//
	if (data_socket == -1) {
		struct sockaddr_in xname = {0} ;
//		int on;

		data_socket = socket (PF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if (data_socket < 0) {
			LOGX("BAD SOCKET !!!!!! [%s]\n", strerror(errno));
			return;
		} else {
			LOGX("Socket created: %d\n", data_socket);
		}
		// bind to this interface
		xname.sin_family = AF_INET;
		xname.sin_addr.s_addr = p->b_card_ip_address;
		//memcpy((char *)&name.sin_addr.s_addr, &metis_cards[0].card_ip_address, sizeof(metis_cards[0].card_ip_address));
		xname.sin_port = htons(DISCOVERY_SEND_PORT);
		rc = bind (data_socket, (struct sockaddr*)&xname, sizeof(xname));
		if (rc != 0) {
			LOG(("BIND on %s FAILED: %s\n", inet_ntoa(xname.sin_addr), strerror (errno) ));
			//return;
		} else {
			LOGX("BIND to %s successfull\n", inet_ntoa(xname.sin_addr));
		}

		// extablish a sort of a receive queue
		int bsize = 102400000;
		rc = setsockopt (data_socket, SOL_SOCKET, SO_RCVBUF, (const char *)&bsize, sizeof(bsize));
		if(rc != 0) {
			LOGX("cannot set SO_RCVBUF: rc=%d\n", rc);
		}

//	    discovery_length=sizeof(discovery_addr);
//		memset(&discovery_addr,0,discovery_length);
//		discovery_addr.sin_family=AF_INET;
//		discovery_addr.sin_port=htons(DISCOVERY_SEND_PORT);
//		discovery_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);

	    // start a receive thread to get discovery responses
		//((METIS_CARD *)pCard)->CbFatalError = fecb;
	    rc = pthread_create ( &receive_thread_id, NULL, receive_thread, (void *)this);
		if (rc != 0) {
			LOGX("pthread_create failed on receive thread: rc=%d\n", rc);
			return;
		} else {
			LOGX("%s\n", "receive thread: thread succcessfully started");
		}
	}

    data_addr_length = sizeof(data_addr);
    memset (&data_addr, 0, data_addr_length);
    data_addr.sin_family = AF_INET;
    data_addr.sin_port   = htons(DATA_PORT);
    memcpy((char *)&data_addr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);

	// TBD TBD
	//ozy_prime();
	pFlow->initialization(this);
	LOGX("%s", "Flow initialization done\n");

	Sleep(300);

    // send a packet to start the stream
    buffer[0]=0xEF;
    buffer[1]=0xFE;
    buffer[2]=0x04;    // data send state send (0x00=stop)
    buffer[3]=0x01;    // I/Q only

    for (i=0; i<60; i++) buffer[i+4] = 0x00;

    send_buffer (data_socket, &buffer[0], 64);
    LOGX("%s", "START COMMAND SENT\n");

	LOGX("%s", "starting metis_watchdog_thread\n");
	watchdog_timeout_in_ms = 250;
    // start a watchdog to make sure we are receiving frames
	//((METIS_CARD *)pCard)->CbTransmissionTmo = ttmocb;
    rc = pthread_create ( &watchdog_thread_id, NULL, watchdog_thread, (void *)this);
    if(rc != 0) {
        LOGX("pthread_create failed on watchdog thread: rc=%d\n", rc);
        return;
    }

}


//static unsigned char input_buffer[20480];


void* Ethernet :: receive_thread (void* arg) 
{
    struct sockaddr_in addr;
    int length;
    int bytes_read;
	char *rc = 0;
	char *p_fatal_error = 0;
	Ethernet *pTgt = (Ethernet *)arg;
	static const char *sync_err = "synch error";
	int loop = 1;

    length = sizeof(addr);
    while (loop) {
   	bytes_read = recvfrom (pTgt->data_socket, (char *) pTgt->input_buffer, sizeof(pTgt->input_buffer), 0, (struct sockaddr*)&addr, &length);
        if(bytes_read < 0) {
            perror("recvfrom socket failed for metis_receive_thread");

			//char *p = (char *) malloc (strlen(strerror (errno))+1);
			//strcpy (p, strerror (errno));
			rc = _strdup(strerror (errno));
            break;
        }
		//LOG(("RECEIVED %d bytes: from [%s]\n", bytes_read, inet_ntoa ( ((struct sockaddr_in *)&addr)->sin_addr)) );

//		if (memcmp ( &((&addr)->sin_addr), &(pTgt->b_card_ip_address), sizeof(pTgt->b_card_ip_address)) == 0) {
//			LOG (("WARNING: ignoring fake answer coming from ourselves !\n"));
//			continue;
//		}

        if (pTgt->input_buffer[0] == 0xEF && pTgt->input_buffer[1] == 0xFE) {
            switch (pTgt->input_buffer[2]) {
                case 1:
                    {
                        // get the end point
                        pTgt->ep = pTgt->input_buffer[3] & 0xFF;

                        // get the sequence number
                        pTgt->sequence = ((pTgt->input_buffer[4]&0xFF)<<24)+((pTgt->input_buffer[5]&0xFF)<<16)+((pTgt->input_buffer[6]&0xFF)<<8)+(pTgt->input_buffer[7]&0xFF);
                        //LOG(("received data ep=%d sequence=%ld\n", pTgt->ep, pTgt->sequence));

                        switch (pTgt->ep) {
                            case 6:
                                // process the data !!!!!!!!!!!!!!!!!
                                //if (process_ozy_input_buffer(&(pTgt->input_buffer[8]))   < 0) {
								if ( pTgt->pFlow->processFromRadio (&(pTgt->input_buffer[8])) < 0) {
									//char *p = (char *) malloc (strlen(sync_err)+1);
									//strcpy (p, sync_err);
									p_fatal_error = _strdup(sync_err);
									loop = 0;
								}
                                //if (process_ozy_input_buffer(&(pTgt->input_buffer[520])) < 0) {
								if ( pTgt->pFlow->processFromRadio (&(pTgt->input_buffer[520])) < 0) {
									//char *p = (char *) malloc (strlen(sync_err)+1);
									//strcpy (p, sync_err);
									p_fatal_error = _strdup(sync_err);
									loop = 0;
								}
                                break;
                            case 4:
                                //LOGX("EP4 data\n");
                                break;
                            default:
                                LOG(("unexpected EP %d length=%d\n", pTgt->ep, bytes_read ));
                                break;
                        }
                    }
                    break;
                case 2:  // response to a discovery packet - hardware is not yet sending
                case 3:  // response to a discovery packet - hardware is already sending
#if 0
                    if(discovering) {
                        if(found<MAX_METIS_CARDS) {
                            // get MAC address from reply
                            sprintf(metis_cards[found].mac_address,"%02X:%02X:%02X:%02X:%02X:%02X",
                                       input_buffer[3]&0xFF,input_buffer[4]&0xFF,input_buffer[5]&0xFF,input_buffer[6]&0xFF,input_buffer[7]&0xFF,input_buffer[8]&0xFF);
                            LOGX("Radio MAC address %s\n",metis_cards[found].mac_address);
    
                            // get ip address from packet header
                            sprintf(metis_cards[found].ip_address,"%d.%d.%d.%d",
                                       addr.sin_addr.s_addr&0xFF,
                                       (addr.sin_addr.s_addr>>8)&0xFF,
                                       (addr.sin_addr.s_addr>>16)&0xFF,
                                       (addr.sin_addr.s_addr>>24)&0xFF);
                            LOGX("Radio IP address %s\n",metis_cards[found].ip_address);
                            metis_cards[found].code_version = input_buffer[9];
                            switch (input_buffer[10]) {
                               case 0x00:
                                  metis_cards[found].board_id = "Metis";
                                  break;
                               case 0x01:
                                  metis_cards[found].board_id = "Hermes";
                                  break;
                               case 0x02:
                                  metis_cards[found].board_id = "Griffin";
                                  break;
                               case 0x04:
                                  metis_cards[found].board_id = "Angelia";
                                  break;
                               default: 
                                  metis_cards[found].board_id = "unknown";
                                  break;
                            }       
                            LOGX("***** Board id: %s\r\n",metis_cards[found].board_id);
                            LOGX("***** version:  %1.2f\r\n",metis_cards[found].code_version /10.0);

                            found++;
                            if(input_buffer[2]==3) {
                                LOGX("%s", "Radio is sending\n");
                            }
                        } else {
                            LOGX("%s", "too many radio cards!\n");
                        }
                    } else {
#endif
                    LOGX("%s", "unexepected discovery response when not in discovery mode\n");
                    
                    break;
                default:
                    LOGX("unexpected packet type: 0x%02X\n", pTgt->input_buffer[2]);
                    break;
            }
        } else {
            LOG(("received bad header bytes on data port %02X,%02X\n", pTgt->input_buffer[0], pTgt->input_buffer[1] ));
        }

    }
	if (p_fatal_error) pTgt->FatalError (p_fatal_error);
    return rc;    
}

int Ethernet :: write (unsigned char ep, unsigned char* buffer, int length) {
    int i;

    if(offset==8) {

        send_sequence++;
        output_buffer[0]=0xEF;
        output_buffer[1]=0xFE;
        output_buffer[2]=0x01;
        output_buffer[3]=ep;
        output_buffer[4]=(send_sequence>>24)&0xFF;
        output_buffer[5]=(send_sequence>>16)&0xFF;
        output_buffer[6]=(send_sequence>>8)&0xFF;
        output_buffer[7]=(send_sequence)&0xFF;

        // copy the buffer over
        for (i = 0; i < 512; i++) {
            output_buffer[i+offset]=buffer[i];
        }
        offset=520;
    } else {
        // copy the buffer over
        for ( i = 0; i < 512; i++) {
            output_buffer[i+offset]=buffer[i];
        }
        offset=8;

        // send the buffer
        send_buffer (data_socket, &output_buffer[0], 1032);

    }

    return length;
}

void Ethernet :: send_buffer (int s, unsigned char* buffer, int length) {
//LOGX("metis_send_buffer: length=%d\n",length);
    if (sendto (s, (const char *)buffer, length, 0, (struct sockaddr*)&data_addr,data_addr_length)<0) {
		LOGX("sendto socket failed: %s\n", strerror(errno));
	}
}


void* Ethernet :: watchdog_thread (void* arg) 
{
	Ethernet *pTgt = (Ethernet *)arg;
	long last_sequence=-1;
	char *pTmoMsg = _strdup("Timeout receiving packets from hardware.\n");
	// sleep for 1 second
	// check if packets received
	LOGX("%s\n", "running metis_watchdog_thread\n");

	while(1) {
		//LOGX("watchdog sleeping...\n");
		Sleep (pTgt->watchdog_timeout_in_ms);
		if ( pTgt->sequence == last_sequence ) {
			LOG(("No metis packets for %d second: sequence=%ld\n", pTgt->watchdog_timeout_in_ms, pTgt->sequence ));
			//dump_metis_buffer ("last frame received", pTgt->sequence, pTgt->input_buffer);
			//dump_metis_buffer ("last frame sent", pTgt->send_sequence, pTgt->output_buffer);
			break;
		}
		last_sequence = pTgt->sequence;
	}

	pTgt->TransmissionTmo (pTmoMsg);

	LOGX("%s", "exit watchdog thread\n");
	return 0;
}

void	Ethernet :: stopReceive  ()
{
	void *rc;

	unsigned char xbuffer [128];

	// set up a packet to stop the stream
    xbuffer[0]=0xEF;
    xbuffer[1]=0xFE;
    xbuffer[2]=0x04;    // data send state (0x00=stop)
    xbuffer[3]=0x00;    // I/Q only STOP 

	for (int i=0; i<60; i++) xbuffer[i+4]=0x00;

	// send stop command
	send_buffer (data_socket, &xbuffer[0], 64);         
	LOGX("%s\n", "Stop command sent");
	// wait for watchdog to detect that hw has really stopped
	pthread_join (watchdog_thread_id, &rc);
	LOGX("%s\n", "Destroying receiver thread....");
	// destroy receiver thread
	closesocket (data_socket);
	pthread_join (receive_thread_id, &rc);
	data_socket = -1;

	LOGX("%s\n", (char *)rc);
}


#if 0
template < class T , int  N_SAMPLES, int N_BLOCKS >
bool Receiver< T , N_SAMPLES, N_BLOCKS> :: is_buffer_full (bool process_tx = false)
{
	// when we have enough samples send them to the clients
	if ( ns == N_SAMPLES ) {
		int d=0;
		// copy the samples into the output buffer
		for (int j=0; j < N_SAMPLES; j++)
			cb_buffer[ni][nb*(N_SAMPLES*2) + d++] = input_buffer[j+N_SAMPLES],  // I part of sample
			cb_buffer[ni][nb*(N_SAMPLES*2) + d++] = input_buffer[j];              // Q part of sample
			nb++;
		if (nb >= N_BLOCKS) {
			pr->process_iq_from_rx ((unsigned char *) cb_buffer[ni] , sizeof(cb_buffer[0]));
			// flips buffers
			nb = 0;
			ni = (ni + 1) % 2;
		}
		if (process_tx)
			pr->process_iq_audio_to_radio ( (unsigned char *)&output_buffer[0],           (unsigned char *)&output_buffer[N_SAMPLES], 
											(unsigned char *)&output_buffer[N_SAMPLES*2], (unsigned char *)&output_buffer[N_SAMPLES*3] );
		ns = 0;
		return true;
	} else
		return false;
}
#endif

const int Flow :: usableBufLen [9] = {
		0,			// filler
		512 - 0,	// 1 RX
		512 - 0,	// 2 RX
		512 - 4,
		512 - 10,
		512 - 24,
		512 - 10,
		512 - 20,
		512 - 4		// 8 RX
	};



int Flow :: processFromRadio  (unsigned char *b)
{
	// check for synchronization sequence
	if (!((b[0] == SC && b[1] == SC && b[2] == SC))) {
		LOGX("%s\n", "!!!!!!!!!!!!!!!!! SYNC ERROR !!!!!!!!!!!!!!!!!");
		DumpHpsdrBuffer ("SYNC ERROR", nrxp, b);
		return -1;
	}
	// process control data
	pr->getControlData((CtrlBuf *)(&b[3]));

	int n = 8;   // start to extract data after synch sequence (3) and control data (5)

	// extract the samples
	while ( n < usableBufLen [pr->getNumberOfRx()] ) {
		//int ls; // left sample
		//int rs; // right sample
		//int ms; // mic sample

		// extract each of the receivers samples data
		for (int r = 0; r < pr->getNumberOfRx(); r++) {
#if 0
			// samples from hardware are 24 bit signed integer
			// put them in a regular int
			ls  = (int)((signed char)   b[n++]) << 16;
			ls += (int)((unsigned char) b[n++]) << 8 ;
			ls += (int)((unsigned char) b[n++])      ;
			rs  = (int)((signed char)   b[n++]) << 16;
			rs += (int)((unsigned char) b[n++]) << 8 ;
			rs += (int)((unsigned char) b[n++])      ;
			// next, rescale to 32 bit
			ls <<= 8; rs <<= 8;
#endif
			HpsdrRxIQSample *pi =  (HpsdrRxIQSample *) &b[n];
			n = n + 3;
			HpsdrRxIQSample *pq =  (HpsdrRxIQSample *) &b[n];
			n = n + 3;
			// append into receiver buffer
			pr->rx[r].append_input_iq ( *pq, *pi ) ;
			if (r) pr->rx[r].next_sample();
		}
		// send to dspserver
		//ms  = (int)((signed char)   b[n++]) << 8;
		//ms += (int)((unsigned char) b[n++]);
		HpsdrMicSample *pms = (HpsdrMicSample *) &b[n];
		n = n + 2;
		pr->rx[0].append_input_mic (*pms);
		pr->rx[0].next_sample();

		// check for buffer full
		for (int r = 0; r < pr->getNumberOfRx(); r++) 
			if (r == 0 ) {
				int bf = pr->rx[r].is_buffer_full (r == 0);	// the receiver 0 contains the data for output stream
															// so signals it in order to trigger the output stream
				if (bf) processToRadio (0);
			} else {
				pr->rx[r].is_buffer_full (0);
			}
	}

	nrxp++;
	return 0;
}


int nxx = 0;

void Flow :: processToRadio (unsigned char *b)
{
	ob[0] = SC;
	ob[1] = SC;
	ob[2] = SC;

	CtrlBuf *pcb = (CtrlBuf *) &(ob [3]);

	pcb->c[0] = send_status << 1;
	pcb->c[1] = pcb->c[2] = pcb->c[3] = pcb->c[4] = 0;

	pr->setControlData(pcb);

	//
	// fill in TX I/Q and audio data TBD
	//
	// TBD
	for (int n = 8; n < sizeof(ob); ++n) ob[n] = 0;

	if ( (nxx < 32) /* || send_status == 2  */ ) { DumpHpsdrHeader ("sending to radio", 0 , ob); }
	nxx++;

	// send the buffer to hardware
	pl->write(0x02, ob, sizeof(ob));


	// simple round robin for control data TBD
	++send_status;
	if (send_status > 11) send_status = 0;
}


void Flow :: initialization (Link *pL)
{
	pl = pL;

	// send frames in order to initialize hardware (priming)
	// TBD
	for (int i = 0; i <16; ++i) {
		processToRadio (0);
		Sleep (10);
	}
}



void DumpHpsdrBuffer (char* rem, int np, unsigned char* b) {
	int i;
	LOG(("%s: packet #%d\n", rem, np));
	for(i = 0; i < Flow::O_BUF_SIZE; i += 16) {
		LOG(("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
				i,
				b[i+0], b[i+1],  b[i+2],  b[i+3],  b[i+4],  b[i+5],  b[i+6],  b[i+7],
				b[i+8], b[i+9],  b[i+10], b[i+11], b[i+12], b[i+13], b[i+14], b[i+15]
			));
	}
	LOG(("%s","\n"));
}


void DumpHpsdrHeader (char* remark, int np, unsigned char* b) {
	LOG(("%s: packet #%d\n", remark, np));
    int i = 0;
    LOG(("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
            i,
			b[i+0], b[i+1],  b[i+2],  b[i+3],  b[i+4],  b[i+5],  b[i+6],  b[i+7],
			b[i+8], b[i+9],  b[i+10], b[i+11], b[i+12], b[i+13], b[i+14], b[i+15]
		));
    LOG(("%s","\n"));
}
