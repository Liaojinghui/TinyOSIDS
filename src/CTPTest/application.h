/*
 * Copyright (c) 2008 Dimas Abreu Dutra
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the Stanford University nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL DIMAS ABREU
 * DUTRA OR HIS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @author Dimas Abreu Dutra
 */

#ifndef RSSIDEMOMESSAGES_H__
#define RSSIDEMOMESSAGES_H__

// CC2420 security enabled - HW security is fast
//#define CC2420_HW_SECURITY 1

#ifndef TOSH_DATA_LENGTH
#define TOSH_DATA_LENGTH 34
#endif

/**
 * Warning!
 * If you want to generate Java Messages by MIG (genJavaMsgs.sh) you need to comment line
 * #include <Ctp.h>
 * and uncomment line
 * #include <Ctp.h>
 * 
 * MIG has trouble to include some needed header files, so they are included in MUGhlp.h.
 */
#include <Ctp.h>
#include "printf.h"
//#include "MIGhlp.h"


// redefined cc2420 headers/metadata to support timestamping, etc...
#include "../TOSph4IDS/cc2420_ids.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

// basic message types
enum {
  AM_RSSIMSG = 10,
  AM_PINGMSG = 11,

  // new messages
  AM_MULTIPINGMSG = 12,
  AM_MULTIPINGRESPONSEMSG = 13,
  AM_COMMANDMSG = 14,
  
  AM_MULTIPINGRESPONSEREPORTMSG = 16,
  AM_MULTIPINGRESPONSETINYREPORTMSG = 17,
  
  AM_NOISEFLOORREADINGMSG = 18,
  AM_IDENTIFYMSG=40,
 
  AM_TIMESYNCMSG = 0xea,
  AM_CTPINFOMSG = 0xec,
  AM_CTPSENDREQUESTMSG = 0xee,
  AM_CTPRESPONSEMSG = 0xef,
  AM_CTPREPORTDATAMSG = 0xed,
  AM_COLLECTIONDEBUGMSG = 0x72
};

typedef struct serialqueue_element{
  nx_struct message_t * msg;
  // payload pointer
  void * payload;
  // address to send message to
  uint16_t addr;
  // length of message to send - parameter to AMSend.send = length of payload
  uint8_t len;
  // AM message type
  uint8_t id;
  // if 1=> radio packet, otherwise serial
  bool isRadioMsg;
} serialqueue_element_t;

typedef struct queueSenderQueue_element{
  // payload pointer
  void * payload;
  // address to send message to
  uint16_t addr;
  // length of message to send - parameter to AMSend.send = length of payload
  uint8_t len;
} queueSenderQueue_element_t;


// serial timesync
// timesync messages are sent over serial to sycnhronize global time according to 
// application
typedef nx_struct TimeSyncMsg{
	nx_uint8_t counter;
	nx_uint32_t high;
	nx_uint32_t low;
	nx_uint8_t flags;
}  TimeSyncMsg;

// ping response
// RssiMeassured
typedef nx_struct RssiMsg{
  nx_uint8_t nodeid;

  // destination node id
  // is this still needed??
  nx_uint8_t dstnodeid;

  // SEQ number
  nx_uint16_t counter;

  // RSSI from mobile node (intercept base will fill this in)
  // todo:
  //    optimize this, define new message, basestation will wrap this to new message
  //    time overhead???
  nx_int16_t rssi;

  // RSSI of ping message from base station
  nx_int16_t rssiFromBase;
} RssiMsg;

// standard ping-response protocol
// ping request
typedef nx_struct PingMsg{
	nx_uint8_t nodeid;
	nx_uint16_t counter;
	nx_uint8_t txpower;
	nx_uint8_t channel;
} PingMsg;

// message to request multiple packets from destination
// 1:N packets
typedef nx_struct MultiPingMsg {
	// where to send ping message? single node or broadcast
	nx_uint16_t destination;
	
	// SEQ number ot this request
	nx_uint16_t counter;

	// tx power of destination
	nx_uint8_t txpower;

	// channel at which to send
	nx_uint8_t channel;

	// number of packets to send
	nx_uint16_t packets;

	// timer delay between message send in ms
	nx_uint16_t delay;
	
	// desired packet size in bytes
	nx_uint8_t size;
	
	// target = packets. CurPacket is incremented when:
	// TRUE => only on succ sent packet => sendDone()==SUCC
	// FALSE => on every Send()==SUCC
	nx_bool counterStrategySuccess;
	
	// if true then timer is started periodically and at each timer tick
	// message is sent 
	// if false new mesage is sent after previous message was successfully sent in 
	// sendDone()
	nx_bool timerStrategyPeriodic;
} MultiPingMsg;

// packet intended to be sent over serial line to indicate meassured noise floor
typedef nx_struct NoiseFloorReadingMsg {
	// SEQ number
	nx_uint16_t counter;

	// noise floor reading from node
	nx_uint16_t noise;
} NoiseFloorReadingMsg;

typedef nx_struct MultiPingResponseMsg {
	// SEQ number
	nx_uint16_t counter;

	// data field to support variable sized messages
	nx_uint8_t data[0];
} MultiPingResponseMsg;

#ifndef RSSI_QUEUE_LEN
#define RSSI_QUEUE_LEN 32
#endif

typedef nx_struct MultiPingResponseReportStruct {
	nx_uint16_t nodeid;
	nx_uint16_t nodecounter;
	nx_int16_t rssi;
	nx_uint8_t len;
} MultiPingResponseReportStruct_t;

// maximum size of report queue buffer
#define MAX_REPORT_QUEUE_SIZE 100

// max data packet size
// need to take into account TOSH_DATA_LENGTH
// packet structure size must be smaller than TOSH_DATA_LENGTH
// otherwise no packet will be send
#define MULTIPINGRESPONSEREPORT_MAXDATA 3

// boot up timer request
#define BOOTUPTIMER_FIRST 500
#define BOOTUPTIMER_NEXT 5000

// used by static nodes to report RSSI values of mobile node
typedef nx_struct MultiPingResponseReportMsg {
	// SEQ number
	nx_uint16_t counter;

	// number of correct data
	nx_uint8_t datanum;

	// unifiing structure
	// mig has problem to convert this message to java class
	// mig cannot work wih structures inside messages
	//MultiPingResponseReportStruct buff[8];
	nx_uint16_t nodeid[MULTIPINGRESPONSEREPORT_MAXDATA];
	nx_uint16_t nodecounter[MULTIPINGRESPONSEREPORT_MAXDATA];
	nx_int16_t rssi[MULTIPINGRESPONSEREPORT_MAXDATA];
	nx_int8_t len[MULTIPINGRESPONSEREPORT_MAXDATA];
} MultiPingResponseReportMsg;

// massReportPacket
typedef nx_struct MassReportMsg {
	// SEQ number
	nx_uint16_t counter;

	// number of correct data
	nx_uint8_t datanum;

	// unifiing structure
	// mig has problem to convert this message to java class
	// mig cannot work wih structures inside messages
	//MultiPingResponseReportStruct buff[8];
	nx_uint8_t nodeid[MULTIPINGRESPONSEREPORT_MAXDATA];
	nx_uint16_t nodecounter[MULTIPINGRESPONSEREPORT_MAXDATA];
	nx_int16_t rssi[MULTIPINGRESPONSEREPORT_MAXDATA];
} MassReportMsg;

// used by static nodes to report RSSI values of mobile node
// lightweight report message
typedef nx_struct MultiPingResponseTinyReportMsg {
	// SEQ number
	nx_uint16_t counter;

	// unifiing structure
	// mig has problem to convert this message to java class
	// mig cannot work wih structures inside messages
	//MultiPingResponseReportStruct buff[8];
	nx_uint16_t nodeid;
	nx_uint16_t nodecounter;
	nx_int16_t rssi;
} MultiPingResponseTinyReportMsg;

/**
 * Command message
 * BaseStation manages its network using this simple protocol. More tasks can be
 * handled using it. For more complex settings its better to use another protocol.
 *
 * Available commands:
 *  - abort
 *      abort current operation
 *      Applicable on mobile nodes or on static nodes.
 *  - identify
 *      Nodes should reply its identification (static/mobile) when receives this packet
 *      BaseStation would know wether nodes lives and which one does.
 *  - myid
 *      Nodes returns its identification
 *  - reset
 *      Reset all state variables. It should have same effect as pushing reset button on node
 *  - ACK
 *      ACKnowledges previous packet. May relate to another command sent by BS
 *  - NACK
 *      Negative ACKnowledge
 *  - setBS
 *      tells static nodes to set/change current base station
 *  - lock
 *      changes locking for static nodes
 *  - getReportingStatus
 *      gets actual reporting status of given node on MultiPingResponses
 *  - setReportingStatus
 *      tells node wether or not to send reports on MultiPingResponses
 *      usefull when analyzing environment by pinging static nodes. Sometimes
 *      may be wanted static nodes not to report anything
 *  - setOperationMode
 *      nodes may be various types, static, dynamic, dead, base station
 *  - setReportProtocol
 *      reportprotocol to be used when in "static" mode
 *
 * Another possible commands, not yet implemented. Not needed now.
 *  - setTx
 *  - setChannel
 *  - setDelay
 *  - setPacketCount
 *  - setRandomizedThreshold
 *  - etc...
 */
typedef nx_struct CommandMsg {
	// command
	nx_uint8_t command_code;

	// command version. Support for protocol versioning. Some nodes may use older
	// firmware. May be used as packet subtype field
	nx_uint8_t command_version;

	// unique command identifier
	// nodes would be able to ACK or NACK commands.
	// It poses more reliable communication.
	// @not-implemented-yet
	nx_uint16_t command_id;

	// in case of ACK command, it is used as answer on specific command
	// only sugar, this info could be stored in command_data_next
	nx_uint8_t reply_on_command;
	nx_uint16_t reply_on_command_id;

	// some data associated with command (parameters for example)s
	nx_uint16_t command_data;

	// for future use
	// may contain another parameters while command_data would tell subtype of protocol
	nx_uint16_t command_data_next[4];
} CommandMsg;


typedef nx_struct IdentifyMsg {
	// command
	nx_uint16_t counter;
	// my node id - announcement
	nx_uint16_t nodeId;
	// reply on some command?
	nx_uint8_t replyOn;
	// platform identification
	nx_uint8_t platformId;
	// number of identify messages sent after boot yet. If overflow
	// stops on maximum number.
	nx_uint8_t identifyAfterBoot;
	
	nx_uint8_t radioQueueLen;
	nx_uint8_t serialQueueLen;
	nx_uint8_t rssiQueueLen;
	nx_uint8_t failCount;

	// for future use
	// may contain another parameters while command_data would tell subtype of protocol
	nx_uint16_t command_data_next[4];
} IdentifyMsg;

/**
 * Defining available commands for nodes
 */
enum {
	COMMAND_NONE = 0,
	COMMAND_ABORT = 1,
	COMMAND_IDENTIFY = 2,
	COMMAND_RESET = 3,
	COMMAND_SETTX = 4,
	COMMAND_SETCHANNEL = 5,
	COMMAND_ACK = 6,
	COMMAND_NACK = 7,
	COMMAND_SETBS = 8,
	COMMAND_LOCK = 9,

	COMMAND_GETREPORTINGSTATUS = 10,

	COMMAND_SETREPORTINGSTATUS = 11,
	COMMAND_SETDORANDOMIZEDTHRESHOLDING = 12,
	COMMAND_SETQUEUEFLUSHTHRESHOLD = 13,
	COMMAND_SETTINYREPORTS = 14,
	COMMAND_SETOPERATIONMODE = 15,
	COMMAND_SETREPORTPROTOCOL = 16,
	COMMAND_FLUSHREPORTQUEUE = 17,
	COMMAND_SETNOISEFLOORREADING = 18,
	COMMAND_SETSAMPLESENSORREADING = 24,

	COMMAND_SETREPORTGAP = 19,

	// sensor readings
	COMMAND_GETSENSORREADING = 20,
	COMMAND_SENSORREADING = 21,

	// pinning
	COMMAND_SETPIN = 22,
	COMMAND_GETPIN = 23,

	// settings
	// Fetching is request sent to base station after booting node up. 
	// Base station will then re-send node settings from node register to 
	// booted node (can be after reset already)
	COMMAND_FETCHSETTINGS = 25,
	
	// base station settings - forwarding from radio to serial?
	COMMAND_FORWARDING_RADIO_ENABLED = 26,
	// base station settings - forwarding from serial to radio?
	COMMAND_FORWARDING_SERIAL_ENABLED = 27,
	
	// base station settings - forwarding from radio to serial? default=without specific wiring
	COMMAND_DEFAULT_FORWARDING_RADIO_ENABLED = 28,
	// base station settings - forwarding from serial to radio? setRadioSnoopEnabled
	COMMAND_DEFAULT_FORWARDING_SERIAL_ENABLED = 29,
	
	// base station settings - whether forward messages received on snoop interface?
	COMMAND_RADIO_SNOOPING_ENABLED = 30,
	// base station settings - address recognition? if false then mote will sniff foreign messages
	COMMAND_RADIO_ADDRESS_RECOGNITION_ENABLED = 31,
	
	// set node as CTP root
	COMMAND_SET_CTP_ROOT=32,
	
	// call CTP route recomputing command - depending on data, CtpInfo interface is used
	// data=1 -> CtpInfo.triggerRouteUpdate()
	// data=2 -> CtpInfo.triggerImmediateRouteUpdate()
	// data=3 -> CtpInfo.recomputeRoutes()
	// data=4 -> RouterReinit re-init neighbour table
	// data=5 -> LinkEstimator re-init neighbour table
	COMMAND_CTP_ROUTE_UPDATE=33,
	
	// gets basic CTP info from CtpInfo interface
	// data=0 -> returns parent, etx, neighbors count in data[0], data[1], data[2]
	// data=1 -> info about neighbor specified in data[0]. Returned addr, link quality, route
    //				quality, congested bit
	COMMAND_CTP_GETINFO=34,
	
	// other CTP controling, can set TX power for packets
	// data=0 -> set tx power for OUTPUT messages for CTP protocol.
	// 				if data[0] == 1	-> set TXpower for ROUTE messages on data[1] level
	//			 	if data[0] == 2 -> set TXpower for DATA messages on data[1] level
	//				if data[0] == 3 -> set TXpower for both ROUTE, DATA messages on data[1] level
	COMMAND_CTP_CONTROL=35,

	// invokes request on global time for every node which heard this request
	COMMAND_TIMESYNC_GETGLOBAL=36, 
	
	// request response protocol to meassure RTT of channel, should be as fast as possible
	COMMAND_PING=37,
	
	// send timesync get_global command to radio broadcast
	COMMAND_TIMESYNC_GETGLOBAL_BCAST=38, 
};

// node ID boudnary for mobile nodes
// nodes with ID>=MOBILE_NODE_ID_BOUDNARY are considered as mobile by default.
// this can be changed via commands
#define MOBILE_NODE_ID_BOUDNARY 200

/**
 * Identifications for COMMAND_IDENTIFY
 */
enum {
	NODE_REPORTING = 1,
	NODE_TALKING = 2,
	NODE_BS = 3,
	NODE_DEAD = 4
};

/**
 * reporting protocols
 */
enum {
	REPORTING_MEDIUM = 1,
	REPORTING_TINY = 2,
	REPORTING_MASS = 3
};

/**
 * sensor readings
 */
enum {
	/*******************************************************************************\
	 * ConditionTypeValide =	000x xxxx xxxx xxxx 					*
	 * 	Last line =	000x 0000 0000 0000 => (A & 1000) / 1000		*
	 * 	typeId =	0000 xxxx 0000 0000 => (A & 0F00) / 0100		*
	 * 	verbId =	0000 0000 xxxx 0000 => (A & 00F0) / 0010		*
	 * 	msg or logic =	0000 0000 0000 x000 => (A & 0008) / 0008 		*
	 * 	msg or logic =	0000 0000 0000 0xxx => (A & 0007) / 0001 		*
	 \*******************************************************************************/
	SCALE_LAST_LINE = 0x1000,
	//4096;
	MASK_LAST_LINE = 0x1000,
	//4096;
	SCALE_TYPE = 0x0100,
	//256;
	MASK_TYPE = 0x0F00,
	//3840;
	SCALE_VERB = 0x0010,
	//16;
	MASK_VERB = 0x00F0,
	//240;
	SCALE_SELECT_MSG = 0x0008,
	//8;
	MASK_SELECT_MSG = 0x0008,
	//8;
	SCALE_MSG_LOGIC = 0x0001,
	//1;
	MASK_MSG_LOGIC = 0x0007,
	//7; 

	/*******************************************************************************\
	 * readingLong =		0000 xxxx xxxx xxxx 					*
	 * 	type reading =	0000 xx00 0000 0000 => (A & 0C00) / 0400		*
	 * 	number alert =	0000 00xx xxxx xxxx => (A & 03FF) / 0001		*
	 \*******************************************************************************/
	SCALE_TYPE_READ = 0x0400,
	//1024
	MASK_TYPE_READ = 0x1C00,
	//3072
	SCALE_NUM_ALERT = 0x0001,
	//1
	MASK_NUM_ALERT = 0x03FF //1023
};

typedef nx_struct CtpResponseMsg {
    nx_uint16_t origin;
    nx_uint16_t seqno;
    nx_uint16_t parent;
    nx_uint16_t metric;
    nx_uint8_t dataType;
    nx_uint16_t data;
} CtpResponseMsg;

// message to request multiple packets from destination, CTP protocol
// 1:N packets
enum {
	CTP_SEND_REQUEST_COUNTER_STRATEGY_SUCCESS = 0x1,
	CTP_SEND_REQUEST_TIMER_STRATEGY_PERIODIC = 0x2,
	CTP_SEND_REQUEST_PACKETS_UNLIMITED = 0x4,
};

typedef nx_struct CtpSendRequestMsg {
	// SEQ number ot this request, identifier
	nx_uint16_t counter;

	// number of packets to send
	nx_uint16_t packets;

	// timer delay between message send in ms
	nx_uint16_t delay;
	
	// percentage of delay +- variability, if 0 no variability is used
	nx_uint16_t delayVariability;
	
	// desired packet size in bytes
	nx_uint8_t size;
	
	// datasource of CtpMessage - can be random/sensor reading
	nx_uint8_t dataSource;
	
	// flags field
	// 0x1 		counterStrategySuccess
	//				CurPacket is incremented when:
	// 				TRUE => only on succ sent packet => sendDone()==SUCC
	// 				FALSE => on every Send()==SUCC
	//
	// 0x2 		timerStrategyPeriodic
	//				 if true then timer is started periodically and at each timer tick
	// 				 message is sent 
	// 				 if false new mesage is sent after previous message was successfully sent in 
	// 				 sendDone()
	//
	// 0x4		unlimited packet count
	
	nx_uint16_t flags;
} CtpSendRequestMsg;

// ctp spoof report message, all collected information available
typedef nx_struct CtpReportDataMsg {
	nx_struct CtpResponseMsg response;
	ctp_data_header_t ctpDataHeader;
	
	// source of message
	nx_am_addr_t amSource;
	
	// rssi of received packet
	nx_int16_t rssi;
	
	// LSB
	// 1. bit 0x1 = spoofed boolean
	// 2. bit 0x2 = normal CTP reception == TRUE, otherwise was tapped
	// 3. bit 0x4 = message was sent from node - reporting of sending by same node
	//			in this case rssi, ctpDataHeader has no meaning, thus must be nulled 
	nx_uint8_t flags;
} CtpReportDataMsg;


typedef nx_struct CtpInfoMsg {
    nx_uint8_t type;
    nx_union {        
        nx_struct {
        	nx_uint16_t data[6];
       	} data;
        
        nx_struct {
        	nx_uint16_t parent;
        	nx_uint16_t etx;
        	nx_uint8_t neighbors;
        	nx_uint8_t serialQueueSize;
        	nx_uint16_t ctpSeqNo;
        	nx_uint8_t ctpBusyCount;
        	nx_uint16_t flags;
        } status;
        
        nx_struct {
        	nx_uint8_t num;
        	nx_uint16_t addr;
        	nx_uint16_t linkQuality;
        	nx_uint16_t routeQuality;		
        	nx_uint16_t flags;
        } neighInfo;

    } data;
} CtpInfoMsg;