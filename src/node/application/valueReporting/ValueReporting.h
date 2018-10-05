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

#include "ValueReportingPacket_m.h"
#include "VirtualApplication.h"
#include <unordered_map>
#include <algorithm>

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

	// Output info
	int timeLimit;
	int sampleRate;
	int bufferSize;
	string evaluation;
	string samplingAlgorithm;

	// Sensing nodes
	bool sendNow; // default value should be: false
	bool displaySampleSensing;
	bool done;
	queue<double> nodeBuffer;

	double bufferFree;
	vector<string> dropQueue; //
	unordered_map<string, vector< pair<double,int> >> sinkBuffer; // pair<double, int> where -> #first: data sample | second: sequence number
	vector<vector<pair<double,int>>> sinkBuffer1;

 protected:
	void startup();

	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
	void handleSensorReading(SensorReadingMessage *);
	void dropQueueAppend(string ID);
	void timerFiredCallback(int);
	void sampleWith(string name);
	void outputSinkBuffer();
	void updateFreeBuffer();
	void avoidLoop();

	void outputSinkBufferSamples();
	string generateFileNamePrefix();

	ValueReportingDataPacket* createDataPkt(double sensValue, int seqNumber);
	ValueReportingDataPacket* createControlPkt(string command);
	unordered_map<string, vector< pair<double,int> >> dropRandom(unordered_map<string, vector< pair<double,int> >> sinkBuffer);
	unordered_map<string, vector< pair<double,int> >> sampleCentral(unordered_map<string, vector< pair<double,int> >> sinkBuffer, int sampleRate, string nodeID);

};
	bool compareSample(pair<double,int> s1, pair<double,int> s2);

#endif				// _VALUEREPORTING_APPLICATIONMODULE_H_
