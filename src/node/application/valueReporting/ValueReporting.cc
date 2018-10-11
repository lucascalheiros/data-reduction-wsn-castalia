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

#include "ValueReporting.h"

Define_Module(ValueReporting);

void ValueReporting::startup()
{
	maxSampleInterval = ((double)par("maxSampleInterval")) / 1000.0;
	minSampleInterval = ((double)par("minSampleInterval")) / 1000.0;
	currSentSampleSN = 0;
	randomBackoffIntervalFraction = genk_dblrand(0);
	sentOnce = false;
	setTimer(REQUEST_SAMPLE, maxSampleInterval * randomBackoffIntervalFraction);
	numNodes = par("numNodes");
	simTimeLimit = par("simTime");
	reductionType = par("reductionType").stringValue();
	timer = 0;
	actualNode = 0;
	sendToSink = 30;
	bufferFree = par("bufferSize");
	maxSize = par("bufferSize");
	reducedOutput = "";
}

void ValueReporting::timerFiredCallback(int index)
{

	if (isSink) {
		if (simTime()>timer) { 
			timer = simTime().dbl() + 0.1; // time to request a data from a node
			actualNode = actualNode%numNodes+1; 
			trace() << "Send me " << actualNode;
			ValueReportingDataPacket *packet2Net =
	    new ValueReportingDataPacket("Send me ", APPLICATION_PACKET);
			string nodeIdStr = to_string(actualNode);
			const char * nodeIdChar = nodeIdStr.c_str();
			toNetworkLayer(packet2Net, nodeIdChar);
			//toNetworkLayer(packet2Net, "-1");
		} 
		if (simTime()>simTimeLimit-10) {
			output();
		}
	}

	switch (index) {
		case REQUEST_SAMPLE:{
			requestSensorReading();
			setTimer(REQUEST_SAMPLE, minSampleInterval);
			break;
		}
	}


}

void ValueReporting::fromNetworkLayer(ApplicationPacket * genericPacket,
		 const char *source, double rssi, double lqi)
{

		ValueReportingDataPacket *rcvPacket = check_and_cast<ValueReportingDataPacket*>(genericPacket);
	ValueReportData theData = rcvPacket->getExtraData();
	if (isSink){
		string senderId = to_string(theData.nodeID);

		double sensedData = rcvPacket->getData();
		sinkBuffer[senderId].push_back(sensedData);
		trace() << "Sink received from: " << senderId << " \tvalue=" << sensedData;
		bufferFree--;

		if (bufferFree <= 5) {
			if(reductionType == "DropRandom") {
				dropRandom();
			}
			else if(reductionType == "DropFirst"){
				dropFirst();
			
			}
			else if(reductionType == "DropLast"){
				dropLast();
			}
			int currentSize = 0;
			
			for (auto& x: sinkBuffer) {
				currentSize+=x.second.size();;
			}
			bufferFree = maxSize - currentSize;
			trace() << "Sink buffer: " << currentSize << "/" << maxSize << endl;
		}
	}
	else {
		int nodeBufferSize = nodeBuffer.size();
		if (nodeBufferSize > 0) {

			sendToSink = nodeBufferSize < sendToSink ? nodeBufferSize: sendToSink; 

			for(int i=0; i<sendToSink; i++) {
				ValueReportData tmpData;
				tmpData.nodeID = (unsigned short)self;
				tmpData.locX = mobilityModule->getLocation().x;
				tmpData.locY = mobilityModule->getLocation().y;
				ValueReportingDataPacket *packet2Net =
					new ValueReportingDataPacket("Sending data to sink", APPLICATION_PACKET);
				packet2Net->setExtraData(tmpData);
				packet2Net->setData(nodeBuffer.front());
				packet2Net->setSequenceNumber(currSentSampleSN);
				nodeBuffer.pop();
				currSentSampleSN++;
				toNetworkLayer(packet2Net, SINK_NETWORK_ADDRESS);
			}

			trace() << sendToSink << "/"<< nodeBufferSize << " -->  (samples sent)/(samples in buffer)" << endl;
		}
	}
}

void ValueReporting::handleSensorReading(SensorReadingMessage * rcvReading)
{
	double sensValue = rcvReading->getSensedValue();

	if(!isSink){
		nodeBuffer.push(sensValue);
		//trace() << "Sensed = " << sensValue;
		sentOnce = true;
	}
}

void ValueReporting::output() {
	ofstream outputSinkBuffer;
	ofstream outputReduced;
	string strSinkBuffer = "";
	outputSinkBuffer.open (reductionType+"_Buffer.csv");

	for (auto& x: sinkBuffer) {
		strSinkBuffer.append(x.first+",");
		for(int i=0; i<x.second.size(); i++)
			strSinkBuffer.append(to_string(x.second[i])+",");
		strSinkBuffer.append("\n");
	}
	outputSinkBuffer << strSinkBuffer;
	outputSinkBuffer.close();
	outputReduced.open (reductionType);
	outputReduced << reducedOutput;
	outputReduced.close();
}

void ValueReporting::dropRandom() {
	vector <string> keys;

	for (auto& x: sinkBuffer) {
		if( x.second.size()>0)
			keys.push_back(x.first);
	}
	int dropPointer = rand() % keys.size();
	int dropPacket = rand() % sinkBuffer[keys[dropPointer]].size();
	double dropped = sinkBuffer[keys[dropPointer]].at(dropPacket);
	sinkBuffer[keys[dropPointer]].erase(sinkBuffer[keys[dropPointer]].begin()+dropPacket);
	reducedOutput += "Drop Random. From node: " + keys[dropPointer] + " Packet Value: " + to_string(dropped) + "\n";
	trace() << "Drop Random. From node: " << keys[dropPointer] << " Packet Value: "<<dropped<<"\n";
}

void ValueReporting::dropLast()  {
	vector <string> keys;

	for (auto& x: sinkBuffer) {
		if( x.second.size()>0)
			keys.push_back(x.first);
	}
	int dropPointer = rand() % keys.size();
	int dropPacket = sinkBuffer[keys[dropPointer]].size() - 1;
	double dropped = sinkBuffer[keys[dropPointer]].at(dropPacket);
	sinkBuffer[keys[dropPointer]].erase(sinkBuffer[keys[dropPointer]].begin()+dropPacket);
	reducedOutput += "Drop Last. From node: " + keys[dropPointer] + " Packet Value: " + to_string(dropped) + "\n";
	trace() << "Drop Last. From node: " << keys[dropPointer] << " Packet Value: "<<dropped<<"\n";
}

void ValueReporting::dropFirst()   {
	vector <string> keys;

	for (auto& x: sinkBuffer) {
		if( x.second.size()>0)
			keys.push_back(x.first);
	}
	int dropPointer = rand() % keys.size();
	int dropPacket = 0;
	double dropped = sinkBuffer[keys[dropPointer]].at(dropPacket);
	sinkBuffer[keys[dropPointer]].erase(sinkBuffer[keys[dropPointer]].begin()+dropPacket);
	reducedOutput += "Drop First. From node: " + keys[dropPointer] + " Packet Value: " + to_string(dropped) + "\n";
	trace() << "Drop First. From node: " << keys[dropPointer] << " Packet Value: "<<dropped<<"\n";
}
