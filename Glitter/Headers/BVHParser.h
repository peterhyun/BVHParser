#ifndef BVHParser_H
#define BVHParser_H

#include "BoneForMotionData.h"
#include <string>
#include <fstream>
#include <iostream>

#define Xposition 1
#define Yposition 2
#define Zposition 3
#define Zrotation 4
#define Xrotation 5
#define Yrotation 6


class BVHParser
{
public:
	BVHParser(const char * BVHFilePath);
	void setHierarchy();
	~BVHParser();
	void readBoneMotion(double * frameTime);
	std::ifstream fileReader;
	Bone * root;
	Bone* settingBoneRelation(std::ifstream& fileReader);
	void setBoneVAOs(Bone * root);
	int boneIDCounter = 0;
	int * VISITED;
	float * motion;
	int totalChannels = 0;
	void setFrame(Bone * root, int frameIndex);
	void setMotion(Bone * root, int frameIndex);
	void clearVISITED();
	int Frames;
	void resetMatrices(Bone * root);
	int motionIndex = 0;
};

BVHParser::BVHParser(const char * BVHFilePath)
{
	fileReader.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		fileReader.open(BVHFilePath);
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::BVHParser::BVHFile_NOT_SUCCESFULLY_READ" << std::endl;
	}
}

void BVHParser::setHierarchy() {
	std::string temp;
	while (fileReader.good()) {
		fileReader >> temp;
		if (temp.compare("ROOT") == 0) {
			root = settingBoneRelation(fileReader);
			break;
		}
	}
}

Bone* BVHParser::settingBoneRelation(std::ifstream& fileReader) {
	//Right now give joint data here
	std::string temp, temp2, temp3;
	Bone * newBone = new Bone();
	fileReader >> temp;
	newBone->setName(temp.c_str());
	newBone->setID(boneIDCounter++);
	while (fileReader.good()) {
		fileReader >> temp;
		if (temp.compare("OFFSET") == 0) {
			fileReader >> newBone->x >> newBone->y >> newBone->z;
			newBone->setT();
		}
		else if (temp.compare("CHANNELS") == 0) {
			int channelNum;
			fileReader >> channelNum;
			totalChannels += channelNum;
			newBone->setnChannel(channelNum);
			newBone->channelOrder = new int[channelNum];
			for (int i = 0; i < channelNum; i++) {
				fileReader >> temp;
				if (temp.compare("Xposition") == 0)
					newBone->channelOrder[i] = Xposition;
				else if (temp.compare("Yposition") == 0)
					newBone->channelOrder[i] = Yposition;
				else if (temp.compare("Zposition") == 0)
					newBone->channelOrder[i] = Zposition;
				else if (temp.compare("Zrotation") == 0)
					newBone->channelOrder[i] = Zrotation;
				else if (temp.compare("Xrotation") == 0)
					newBone->channelOrder[i] = Xrotation;
				else if (temp.compare("Yrotation") == 0)
					newBone->channelOrder[i] = Yrotation;
			}
		}
		else if (temp.compare("JOINT") == 0){
			//set child node data!
			Bone * childBone = settingBoneRelation(fileReader);
			newBone->setChild(childBone);
		}
		else if (temp.compare("End") == 0) {
			//set end node data! (which is gonna be the child node)
			Bone * childBone = new Bone();
			childBone->setName(temp.c_str());
			childBone->setID(boneIDCounter++);
			fileReader >> temp >> temp2 >> temp3;	// Site { OFFSET
			fileReader >> childBone->x >> childBone->y >> childBone->z;
			fileReader >> temp; //get rid of 1 }
			newBone->setChild(childBone);
		}
		else if (temp.compare("}") == 0) {
			return newBone;
		}
	}
}

void BVHParser::clearVISITED() {
	VISITED = new int[boneIDCounter];
	for (int i = 0; i < boneIDCounter; i++) {
		VISITED[i] = 0;
	}
}

//now set the vertex data of each bones
void BVHParser::setBoneVAOs(Bone * root) {
	root->setVAOs();
	VISITED[root->returnBoneID()] = 1;
	for (int i = 0; i < root->returnnChildren(); i++) {
		if(!VISITED[root->i_thChild(i)->returnBoneID()])
			setBoneVAOs(root->i_thChild(i));
	}
}

void BVHParser::readBoneMotion(double * frameTime) {
	std::string temp;
	while (fileReader.good()) {
		fileReader >> temp;
		if (temp.compare("MOTION")==0) {
			break;
		}
	}
	fileReader >> temp >> Frames;
	std::cout << "Frames read as : "<< Frames << std::endl;
	std::cout << "totalChannels are : " << totalChannels << std::endl;
	motion = new float[Frames * totalChannels];
	fileReader >> temp >> temp >> *frameTime;
	int i = 0;
	while (i!=Frames*totalChannels) {
		fileReader >> motion[i];
		i++;
	}
}

void BVHParser::resetMatrices(Bone * root) {
	root->setR(glm::mat4(1.0f));
	root->setT();
	VISITED[root->returnBoneID()] = 1;
	for (int i = 0; i < root->returnnChildren(); i++) {
		if (!VISITED[root->i_thChild(i)->returnBoneID()])
			resetMatrices(root->i_thChild(i));
	}
}

//gonna set all the translation matrices and the rotation matrices here
//read the frameIndex-frame values (which are 60 per frame)
void BVHParser::setFrame(Bone * root, int frameIndex) {
	clearVISITED();
	resetMatrices(root);
	clearVISITED();
	motionIndex = frameIndex * totalChannels;
	setMotion(root, frameIndex);
}

void BVHParser::setMotion(Bone * root, int frameIndex) {
	int * channelOrder = root->channelOrder;
	for (int i = 0; i < root->returnnChannel(); i++) {
		if (channelOrder[i] == Xposition) {
			root->setT(glm::translate(root->returnT(), glm::vec3(motion[motionIndex], 0.0f, 0.0f)));
			motionIndex++;
		}
		else if (channelOrder[i] == Yposition) {
			root->setT(glm::translate(root->returnT(), glm::vec3(0.0f, motion[motionIndex], 0.0f)));
			motionIndex++;
		}
		else if (channelOrder[i] == Zposition) {
			root->setT(glm::translate(root->returnT(), glm::vec3(0.0f, 0.0f, motion[motionIndex])));
			motionIndex++;
		}
		else if (channelOrder[i] == Xrotation) {
			root->setR(glm::rotate(root->returnR(), glm::radians(motion[motionIndex]), glm::vec3(1.0f, 0.0f, 0.0f)));
			motionIndex++;
		}
		else if (channelOrder[i] == Yrotation) {
			root->setR(glm::rotate(root->returnR(), glm::radians(motion[motionIndex]), glm::vec3(0.0f, 1.0f, 0.0f)));
			motionIndex++;
		}
		else if (channelOrder[i] == Zrotation) {	//Zrotation
			root->setR(glm::rotate(root->returnR(), glm::radians(motion[motionIndex]), glm::vec3(0.0f, 0.0f, 1.0f)));
			motionIndex++;
		}
	}
	VISITED[root->returnBoneID()] = 1;
	for (int i = 0; i < root->returnnChildren(); i++) {
		if (!VISITED[root->i_thChild(i)->returnBoneID()])
			setMotion(root->i_thChild(i), frameIndex);
	}
}

BVHParser::~BVHParser()
{
	fileReader.close();
}
#endif
