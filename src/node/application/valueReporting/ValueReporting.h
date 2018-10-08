/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Dimosthenis Pediaditakis, Yuriy Tselishchev                  *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *
 ****************************************************************************/

#ifndef _VALUEREPORTING_H_
#define _VALUEREPORTING_H_

#include "VirtualApplication.h"
#include "ValueReportingPacket_m.h"
#include <map>
#include <algorithm>

using namespace std;

enum ValueReportingTimers {
	REQUEST_SAMPLE = 1,
	SEND_DATA = 2,
};

class ValueReporting: public VirtualApplication { 
 private:
	double maxSampleInterval;
	double minSampleInterval;

	int routingLevel;
	double lastSensedValue;
	int currSentSampleSN;

	double randomBackoffIntervalFraction;
	bool sentOnce;
	double timer;
	int actualNode;
	int numNodes;
	int simTimeLimit;
	int bufferSize;
	string reductionType;
	int sendToSink;
	map<string, vector<double>> sinkBuffer; 
	queue<double> nodeBuffer;
	double bufferFree;
	string reducedOutput;

 protected:
	void startup();
	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
	void handleSensorReading(SensorReadingMessage *);
	void timerFiredCallback(int);
	void output();
	void updateFreeBuffer();
	void dropRandom(); 
	void dropFirst();
	void dropLast();

};
////////////////////////////////
#endif				// _VALUEREPORTING_APPLICATIONMODULE_H_
