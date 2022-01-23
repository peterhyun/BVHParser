//The unit node of the rig
#include "Shader.h"
#include "Camera.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>

const int MAX = 3;

class Bone {
private:
	char * boneName;
	int boneID;
	glm::mat4 R;
	glm::mat4 T;
	Bone * childBone[MAX];
	int nChildren = 0;
	int nChannel = 0;
public:
	float x, y, z;	//THIS Should be translated for intial T
	int * channelOrder;
	unsigned int * VAOs;
	//unsigned int * VBOs;	//Is it okay if I delete ths VBO? Test it later. Should be in private:.
	Bone() {
		this->R = glm::mat4(1.0f);
		this->T = glm::mat4(1.0f);
	}
	void setName(const char * boneName) {
		size_t length = strlen(boneName) + 1;
		this->boneName = new char[length];
		strcpy(this->boneName, boneName);
	}

	void setnChannel(int nChannel) {
		this->nChannel = nChannel;
	}

	void setID(int boneIDCounter) {
		boneID = boneIDCounter;
	}

	int returnnChannel() {
		return nChannel;
	}

	void setVAOs();

	void setChild(Bone * child) {
		childBone[nChildren] = child;
		nChildren++;
	}

	void setR(glm::mat4 R) {
		this->R = R;
	}
	void setT(glm::mat4 T) {
		this->T = T;
	}
	void setT() {
		glm::mat4 temp = glm::mat4(1.0f);
		this->T = glm::translate(temp,glm::vec3(x,y,z));
	}
	glm::mat4 returnR() {
		return R;
	}
	glm::mat4 returnT() {
		return T;
	}
	const char * returnBoneName() {
		return boneName;
	}
	int returnBoneID() {
		return boneID;
	}
	int returnnChildren() {
		return nChildren;
	}
	Bone * i_thChild(int index) {
		return childBone[index];
	}
	unsigned int returnVAOs(int i) {
		if (i >= nChildren) {
			std::cout << "ERROR. THAT VAO DOES NOT EXIST" << std::endl;
			return NULL;
		}
		else {
			return VAOs[i];
		}
	}
};

void Bone::setVAOs() {
	VAOs = new unsigned int[nChildren];
	unsigned int * VBOs = new unsigned int[nChildren];
	glm::vec3 parentPos = glm::vec3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < nChildren; i++) {
		float a = childBone[i]->x;
		float b = childBone[i]->y;
		float c = childBone[i]->z;
		glm::vec3 childPos = glm::vec3(a, b, c);
		glm::vec3 parent2Child = childPos - parentPos;
		glm::vec3 cross1 = normalize(glm::cross(parent2Child, glm::vec3(0.0f, 0.1f, -1.0f)));	//0.0f, 0.1f, -1.0f leads to NaN of foot and toes cause cross product becomes zero.
		glm::vec3 cross2 = normalize(glm::cross(parent2Child, cross1));
		glm::vec3 minus1 = glm::vec3(-cross1.x, -cross1.y, -cross1.z);
		glm::vec3 minus2 = glm::vec3(-cross2.x, -cross2.y, -cross2.z);
		float thick = 2.0f;
		float vertices[] = {
			//Back Face
			parentPos.x + thick * cross1.x + thick * minus2.x, parentPos.y + thick * cross1.y + thick * minus2.y, parentPos.z + thick * cross1.z + thick * minus2.z,
			childPos.x + thick * minus1.x + thick * minus2.x, childPos.y + thick * minus1.y + thick * minus2.y, childPos.z + thick * minus1.z + thick * minus2.z,
			parentPos.x + thick * minus1.x + thick * minus2.x, parentPos.y + thick * minus1.y + thick * minus2.y, parentPos.z + thick * minus1.z + thick * minus2.z,
			childPos.x + thick * minus1.x + thick * minus2.x, childPos.y + thick * minus1.y + thick * minus2.y, childPos.z + thick * minus1.z + thick * minus2.z,
			parentPos.x + thick * cross1.x + thick * minus2.x, parentPos.y + thick * cross1.y + thick * minus2.y, parentPos.z + thick * cross1.z + thick * minus2.z,
			childPos.x + thick * cross1.x + thick * minus2.x, childPos.y + thick * cross1.y + thick * minus2.y, childPos.z + thick * cross1.z + thick * minus2.z,

			//Front face
			parentPos.x + thick * cross1.x + thick * cross2.x, parentPos.y + thick * cross1.y + thick * cross2.y, parentPos.z + thick * cross1.z + thick * cross2.z,
			parentPos.x + thick * minus1.x + thick * cross2.x, parentPos.y + thick * minus1.y + thick * cross2.y, parentPos.z + thick * minus1.z + thick * cross2.z,
			childPos.x + thick * minus1.x + thick * cross2.x, childPos.y + thick * minus1.y + thick * cross2.y, childPos.z + thick * minus1.z + thick * cross2.z,
			childPos.x + thick * minus1.x + thick * cross2.x, childPos.y + thick * minus1.y + thick * cross2.y, childPos.z + thick * minus1.z + thick * cross2.z,
			childPos.x + thick * cross1.x + thick * cross2.x, childPos.y + thick * cross1.y + thick * cross2.y, childPos.z + thick * cross1.z + thick * cross2.z,
			parentPos.x + thick * cross1.x + thick * cross2.x, parentPos.y + thick * cross1.y + thick * cross2.y, parentPos.z + thick * cross1.z + thick * cross2.z,

			//Left face
			childPos.x + thick * cross1.x + thick * cross2.x, childPos.y + thick * cross1.y + thick * cross2.y, childPos.z + thick * cross1.z + thick * cross2.z,
			childPos.x + thick * cross1.x + thick * minus2.x, childPos.y + thick * cross1.y + thick * minus2.y, childPos.z + thick * cross1.z + thick * minus2.z,
			parentPos.x + thick * cross1.x + thick * minus2.x, parentPos.y + thick * cross1.y + thick * minus2.y, parentPos.z + thick * cross1.z + thick * minus2.z,
			parentPos.x + thick * cross1.x + thick * minus2.x, parentPos.y + thick * cross1.y + thick * minus2.y, parentPos.z + thick * cross1.z + thick * minus2.z,
			parentPos.x + thick * cross1.x + thick * cross2.x, parentPos.y + thick * cross1.y + thick * cross2.y, parentPos.z + thick * cross1.z + thick * cross2.z,
			childPos.x + thick * cross1.x + thick * cross2.x, childPos.y + thick * cross1.y + thick * cross2.y, childPos.z + thick * cross1.z + thick * cross2.z,

			//Right face
			childPos.x + thick * minus1.x + thick * cross2.x, childPos.y + thick * minus1.y + thick * cross2.y, childPos.z + thick * minus1.z + thick * cross2.z,
			parentPos.x + thick * minus1.x + thick * minus2.x, parentPos.y + thick * minus1.y + thick * minus2.y, parentPos.z + thick * minus1.z + thick * minus2.z,
			childPos.x + thick * minus1.x + thick * minus2.x, childPos.y + thick * minus1.y + thick * minus2.y, childPos.z + thick * minus1.z + thick * minus2.z,
			parentPos.x + thick * minus1.x + thick * minus2.x, parentPos.y + thick * minus1.y + thick * minus2.y, parentPos.z + thick * minus1.z + thick * minus2.z,
			childPos.x + thick * minus1.x + thick * cross2.x, childPos.y + thick * minus1.y + thick * cross2.y, childPos.z + thick * minus1.z + thick * cross2.z,
			parentPos.x + thick * minus1.x + thick * cross2.x, parentPos.y + thick * minus1.y + thick * cross2.y, parentPos.z + thick * minus1.z + thick * cross2.z,

			//Bottom face
			parentPos.x + thick * cross1.x + thick * minus2.x, parentPos.y + thick * cross1.y + thick * minus2.y, parentPos.z + thick * cross1.z + thick * minus2.z,
			parentPos.x + thick * minus1.x + thick * minus2.x, parentPos.y + thick * minus1.y + thick * minus2.y, parentPos.z + thick * minus1.z + thick * minus2.z,
			parentPos.x + thick * minus1.x + thick * cross2.x, parentPos.y + thick * minus1.y + thick * cross2.y, parentPos.z + thick * minus1.z + thick * cross2.z,
			parentPos.x + thick * minus1.x + thick * cross2.x, parentPos.y + thick * minus1.y + thick * cross2.y, parentPos.z + thick * minus1.z + thick * cross2.z,
			parentPos.x + thick * cross1.x + thick * cross2.x, parentPos.y + thick * cross1.y + thick * cross2.y, parentPos.z + thick * cross1.z + thick * cross2.z,
			parentPos.x + thick * cross1.x + thick * minus2.x, parentPos.y + thick * cross1.y + thick * minus2.y, parentPos.z + thick * cross1.z + thick * minus2.z,

			//Top face
			childPos.x + thick * cross1.x + thick * minus2.x, childPos.y + thick * cross1.y + thick * minus2.y, childPos.z + thick * cross1.z + thick * minus2.z,
			childPos.x + thick * minus1.x + thick * cross2.x, childPos.y + thick * minus1.y + thick * cross2.y, childPos.z + thick * minus1.z + thick * cross2.z,
			childPos.x + thick * minus1.x + thick * minus2.x, childPos.y + thick * minus1.y + thick * minus2.y, childPos.z + thick * minus1.z + thick * minus2.z,
			childPos.x + thick * minus1.x + thick * cross2.x, childPos.y + thick * minus1.y + thick * cross2.y, childPos.z + thick * minus1.z + thick * cross2.z,
			childPos.x + thick * cross1.x + thick * minus2.x, childPos.y + thick * cross1.y + thick * minus2.y, childPos.z + thick * cross1.z + thick * minus2.z,
			childPos.x + thick * cross1.x + thick * cross2.x, childPos.y + thick * cross1.y + thick * cross2.y, childPos.z + thick * cross1.z + thick * cross2.z
		};
		float normals[] = {
			minus2.x, minus2.y, minus2.z,
			minus2.x, minus2.y, minus2.z,
			minus2.x, minus2.y, minus2.z,
			minus2.x, minus2.y, minus2.z,
			minus2.x, minus2.y, minus2.z,
			minus2.x, minus2.y, minus2.z,

			cross2.x, cross2.y, cross2.z,
			cross2.x, cross2.y, cross2.z,
			cross2.x, cross2.y, cross2.z,
			cross2.x, cross2.y, cross2.z,
			cross2.x, cross2.y, cross2.z,
			cross2.x, cross2.y, cross2.z,

			cross1.x, cross1.y, cross1.z,
			cross1.x, cross1.y, cross1.z,
			cross1.x, cross1.y, cross1.z,
			cross1.x, cross1.y, cross1.z,
			cross1.x, cross1.y, cross1.z,
			cross1.x, cross1.y, cross1.z,

			minus1.x, minus1.y, minus1.z,
			minus1.x, minus1.y, minus1.z,
			minus1.x, minus1.y, minus1.z,
			minus1.x, minus1.y, minus1.z,
			minus1.x, minus1.y, minus1.z,
			minus1.x, minus1.y, minus1.z,

			-parent2Child.x, -parent2Child.y, -parent2Child.z,
			-parent2Child.x, -parent2Child.y, -parent2Child.z,
			-parent2Child.x, -parent2Child.y, -parent2Child.z,
			-parent2Child.x, -parent2Child.y, -parent2Child.z,
			-parent2Child.x, -parent2Child.y, -parent2Child.z,
			-parent2Child.x, -parent2Child.y, -parent2Child.z,

			parent2Child.x, parent2Child.y, parent2Child.z,
			parent2Child.x, parent2Child.y, parent2Child.z,
			parent2Child.x, parent2Child.y, parent2Child.z,
			parent2Child.x, parent2Child.y, parent2Child.z,
			parent2Child.x, parent2Child.y, parent2Child.z,
			parent2Child.x, parent2Child.y, parent2Child.z
		};
		glGenVertexArrays(1, &VAOs[i]);
		glGenBuffers(1, &VBOs[i]);
		glBindVertexArray(VAOs[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 216, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 108, vertices);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 108, sizeof(float) * 108, normals);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(sizeof(float)*108));
	}
}
