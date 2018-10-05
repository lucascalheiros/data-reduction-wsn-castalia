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
	

	// TODO
	sampleRate = par("sampleRate");
	bufferSize = par("bufferSize");
	evaluation = par("evaluation").stringValue();
	samplingAlgorithm = par("samplingAlgorithm").stringValue();
	string simTimeToStr(ev.getConfig()->getConfigValue("sim-time-limit"));
	simTimeToStr.pop_back();
	timeLimit = stod(simTimeToStr);
	bufferFree = par("bufferSize");
	timer=0;
	actualNode=0;
}

void ValueReporting::timerFiredCallback(int index)
{

	if (isSink) {
		if (simTime()>timer) { 
			timer = simTime().dbl() + 1;
			ValueReportingDataPacket *packet2Net =
	    new ValueReportingDataPacket("Value reporting pck", APPLICATION_PACKET);
			actualNode = actualNode%20+1; 
			trace() <<"sendToMe node " << actualNode;
			string nodeIdStr = to_string(actualNode);
			const char * nodeIdChar = nodeIdStr.c_str();
			toNetworkLayer(packet2Net, nodeIdChar);
		} 
	}

	switch (index) {
		case REQUEST_SAMPLE:{
			requestSensorReading();
			setTimer(REQUEST_SAMPLE, maxSampleInterval);
			break;
		}
	}

	// TODO
	if(isSink && simTime() > timeLimit-maxSampleInterval) 
		outputSinkBuffer();
}

void ValueReporting::fromNetworkLayer(ApplicationPacket * genericPacket,
		 const char *source, double rssi, double lqi)
{

		ValueReportingDataPacket *rcvPacket = check_and_cast<ValueReportingDataPacket*>(genericPacket);
	ValueReportData theData = rcvPacket->getExtraData();
	if (isSink){
		string senderId = to_string(theData.nodeID);

		unordered_map<string, vector< pair<double,int> > >::const_iterator check = sinkBuffer.find(senderId); //Check: ID's already in the hash

		pair<double,int> sample(rcvPacket->getData(),rcvPacket->getSequenceNumber()); // Seq. number to avoid loss of (x,y) location

		if(check != sinkBuffer.end()) { // Hit the data location
			sinkBuffer.at(senderId).push_back(sample); 
		} else { // First data entry from this node
			vector< pair<double,int> > tempVector;
			tempVector.push_back(sample);

			sinkBuffer.insert({senderId, tempVector});
			trace() << "New hash entry added (!)";

		}
		trace() << "Sink received from: " << senderId << " \tvalue=" << rcvPacket->getData();
		bufferFree--;
		dropQueueAppend(senderId); // Add the current node to the drop queue

		if (bufferFree <= 10) {
			trace() << "Buffer is full.";
			sampleWith(samplingAlgorithm);
		}
	}
	else {
		if (nodeBuffer.size() > 0) {

			int bufferLength = nodeBuffer.size();
			int toSend = 10; 

			if (bufferLength < toSend) 
				toSend = bufferLength;

			for(int i=0; i<toSend; i++) {
				ValueReportingDataPacket* packet2Net = createDataPkt(nodeBuffer.front(), currSentSampleSN); //sensVal nodeBuffer[0]
				nodeBuffer.pop();
				currSentSampleSN++;
				toNetworkLayer(packet2Net, SINK_NETWORK_ADDRESS); // SENDING DATA HERE: ToNetworkLayer()
			}

			trace() << "[DONE!] " << toSend << " samples was sent successfully. / " << nodeBuffer.size() << " samples pending." << endl;
		}
	}
}

void ValueReporting::handleSensorReading(SensorReadingMessage * rcvReading)
{
	double sensValue = rcvReading->getSensedValue();

	if(!isSink){
		nodeBuffer.push(sensValue);
		trace() << "Sensed = " << sensValue;

		sentOnce = true;
	}
}

void ValueReporting::sampleWith(string name) { // Add here a call for new sampling algorithms

	if(name=="DropRandom") {
		sinkBuffer = dropRandom(sinkBuffer);
	}
	else if(name=="SampleCentral") { 

		for(int i=0; i<dropQueue.size()-1; i++) // Empty the drop queue with pending nodes (keep the last element because its data maybe incomplete)
			sinkBuffer = sampleCentral(sinkBuffer, sampleRate, dropQueue.at(i));

		if(dropQueue.size()==1) // If there is a single element, sample it
			sinkBuffer = sampleCentral(sinkBuffer, sampleRate, dropQueue.front());

		dropQueue.clear();
		sampleRate = par("sampleRate");
	} 

	else
		name = "None";

	trace() << "Sampling algorithm: " << name;
	updateFreeBuffer();
}

/********************************* COMMUNICATION *************************************************************/





ValueReportingDataPacket* ValueReporting::createControlPkt(string command)
{
	char* comm = strdup(command.c_str()); // coercing to the accepted type: char*
	int sensValue = 0;

	ValueReportData tmpData;
	tmpData.nodeID = (unsigned short)self;
	tmpData.locX = mobilityModule->getLocation().x;
	tmpData.locY = mobilityModule->getLocation().y;
	tmpData.command = comm;

	ValueReportingDataPacket *packet2Net =
	    new ValueReportingDataPacket(comm, APPLICATION_PACKET);

	packet2Net->setExtraData(tmpData);
	packet2Net->setData(sensValue);

	return(packet2Net);
}

ValueReportingDataPacket* ValueReporting::createDataPkt(double sensValue, int seqNumber)
{
	ValueReportData tmpData;
	tmpData.nodeID = (unsigned short)self;
	tmpData.locX = mobilityModule->getLocation().x;
	tmpData.locY = mobilityModule->getLocation().y;
	tmpData.command = "data_pkt";

	ValueReportingDataPacket *packet2Net =
	    new ValueReportingDataPacket("data_pkt", APPLICATION_PACKET);
	packet2Net->setExtraData(tmpData);
	packet2Net->setData(sensValue);
	packet2Net->setSequenceNumber(currSentSampleSN);

	return(packet2Net);
}

void ValueReporting::updateFreeBuffer()
{
	int currentSize=0;
	int maxSize=par("bufferSize");
	
	for (auto& x: sinkBuffer) {
		for(int i=0; i<x.second.size(); i++)
			currentSize++;
	}

	if(currentSize <= maxSize){
		bufferFree = maxSize - currentSize; // Update the buffer-free control variable ONLY HERE
	}
	else
		trace() << "ERROR! SINK BUFFER OVERHEAD.";

	trace() << "Current size: " << currentSize << "/ From: " << maxSize << " units" << endl;
}

void ValueReporting::dropQueueAppend(string ID)
{
	bool addToQueue=true;

 	for(int i=0; i<dropQueue.size(); i++) { // Check if this ID is already in queue 

		if(ID == dropQueue.at(i)) { // Avoid duplicate IDs
			addToQueue=false;
			break;
		}
	}

	if(addToQueue)
		dropQueue.push_back(ID); // Add the last sending node to drop queue
}

string ValueReporting::generateFileNamePrefix()
{

	trace() << "Obs.: REMEMBER TO CREATE SUBFOLDER /RemainingIndexes IN SIMULATION FOLDER (!)";
	string fileNamePrefix = "RemainingIndexes/";

	if (evaluation == "varyBuffer")
		fileNamePrefix.append(evaluation+"-"+to_string(bufferSize)+"-");

	if (evaluation == "varySampleRate")
		fileNamePrefix.append(evaluation+"-"+to_string(bufferSize)+"-"+to_string(sampleRate)+"-");
	
	fileNamePrefix.append(samplingAlgorithm+"-");

	return fileNamePrefix;
}

/************************************** EXPORT *************************************************************/

void ValueReporting::outputSinkBuffer() // OBS.: The first element from each line is the respective node ID
{
	// Exporting remaining indexes to match with initial hash
	ofstream outputSinkBuffer;
	string strSinkBuffer = "";
	string fileName = generateFileNamePrefix()+"remainingIndexes.csv";
	
	outputSinkBuffer.open (fileName);

	for (auto& x: sinkBuffer) {
		strSinkBuffer.append(x.first+","); // First element of line,  x.first, is a string containing the node ID:
		for(int i=0; i<x.second.size(); i++)
			strSinkBuffer.append(to_string(get<1>(x.second[i]))+","); // x.second[i] is a pair<double,int> | Get the indexes with get<1>

		strSinkBuffer.append("\n"); 
	}

	outputSinkBuffer << strSinkBuffer;
	outputSinkBuffer.close();

	trace() << "Last state of sink buffer was exported to " << fileName;
}

void ValueReporting::outputSinkBufferSamples() // OBS.: The first element from each line is the respective node ID
{
	ofstream outputSinkBuffer;

	// Exporting samples
	string strSinkBuffer = "";
	string fileName = generateFileNamePrefix()+"remainingSamples.csv";
	outputSinkBuffer.open (fileName);

	for (auto& x: sinkBuffer) {
		strSinkBuffer.append(x.first+","); // First element of line is the node
		for(int i=0; i<x.second.size(); i++)
			strSinkBuffer.append(to_string(get<0>(x.second[i]))+","); // x.second[i] is a pair<double,int> | Get the samples with get<0>

		strSinkBuffer.append("\n"); 
	}

	outputSinkBuffer << strSinkBuffer;
	outputSinkBuffer.close();

	trace() << "Last state of sink buffer was exported to " << fileName;

}

unordered_map<string, vector< pair<double,int> >> ValueReporting::dropRandom(unordered_map<string, vector< pair<double,int> >> sinkBuffer) {

	trace() << "Running Drop Random." << endl;
	unordered_map<string, vector< pair<double,int> >> reducedBuffer = sinkBuffer;
	vector <string> keys;

	for (auto& x: reducedBuffer)
		keys.push_back(x.first); // Check the keys inside the hash

	int dropPointer = rand() % keys.size(); // Point to a random key (node)

	reducedBuffer.erase(keys[dropPointer]); // Drop it
	trace() << keys[dropPointer] << " Removed." << endl;

	return reducedBuffer;
}

unordered_map<string, vector< pair<double,int> >> ValueReporting::sampleCentral(unordered_map<string, vector< pair<double,int> >> sinkBuffer, int sampleRate, string nodeID)
{
	trace() << "Running Sampling Central." << endl;
	unordered_map<string, vector< pair<double,int> >> reducedBuffer = sinkBuffer;
	vector < pair<double,int> > newData;

	int dataLength = reducedBuffer.at(nodeID).size();

	sort(reducedBuffer.at(nodeID).begin(), reducedBuffer.at(nodeID).end(), compareSample);
	
	int newSize = dataLength*sampleRate/100; // Length of reduced sample vector
	if(!newSize)
		newSize++;

	int sampleIndex = (dataLength - newSize)/2; // Pointer to the index which will start the sampling

	for(int i=sampleIndex; i<(sampleIndex+newSize); i++) // Sampling central data
		newData.push_back(reducedBuffer.at(nodeID)[i]);

	reducedBuffer.at(nodeID) = newData;
	newData.clear();

	return reducedBuffer;
}

bool compareSample(pair<double,int> s1, pair<double,int> s2) {

	double i = get<0>(s1);
	double j = get<0>(s2);

	return (i<j);
}
