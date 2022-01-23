#include "BVHParser.h"
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void drawBones(Shader& shader, Bone* rootBone, glm::mat4 model);
// settings
const unsigned int SCR_WIDTH = 2560;
const unsigned int SCR_HEIGHT = 1920;

// camera
glm::vec3 cameraLoc = glm::vec3(0.0f, 50.0f, 100.0f);
Camera camera(cameraLoc);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
//deltaTime is for mouse speed
double deltaTime = 0;
//deltaTime2 is for FPS speed
double deltaTime2 = 0;
double lastFrame = 0;
//frameIndex is for reading in the BVH file's i-th frame data
int frameIndex = 0;
//FPSShower shows how many frames passed by for 1 second.
int FPSShower = 0;
//How many times are the render settings updated?
int update = 0;
double timer = 0;

unsigned int * VISITED;

bool moveFlag = false;
bool resetMatrices = false;

double limitFPS = 0;

int main(int argc, char **argv)
{
    if (argc<=1 || argc>2){
        std::cout << "Start this program with a bvh file name you want to read in the command line!\nPress any key to end program"<<std::endl;
		std::cin.get();
		return -1;
    }
	
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

// glfw window creation
// --------------------
	
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "BVHParser", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
    //char s1[50] = "MotionData/";
    char * s2 = argv[1];
    //strcat(s1,s2);
	BVHParser bvh(s2);
	bvh.setHierarchy();
	bvh.clearVISITED();
	bvh.setBoneVAOs(bvh.root);
	//get the FPS value!
	bvh.readBoneMotion(&limitFPS);


	// build and compile shaders
	Shader shader("AnimationVertexShader.vs", "AnimationFragmentShader.fs");
	
	shader.use();
	// render loop
	// -----------

	lastFrame = glfwGetTime();
	timer = lastFrame;
	
	while (!glfwWindowShouldClose(window))
	{
		
		// render
		// ------
		glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// per-frame time logic
		// --------------------
		VISITED = new unsigned int[bvh.boneIDCounter];
		for (int i = 0; i < bvh.boneIDCounter; i++) {
			VISITED[i] = 0;
		}

		//get nowTime
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		deltaTime2 += (currentFrame - lastFrame) / limitFPS;
		lastFrame = currentFrame;
        
        processInput(window);

		// update settings here
		while (deltaTime2 >= 1.0) {
			shader.setVec3("light.direction", 0.0f, -0.1f, 0.0f);
			shader.setVec3("viewPos", camera.Position);
			shader.setVec3("light.ambient", 0.3f, 0.3f, 0.2f);
			shader.setVec3("light.diffuse", 0.7f, 0.7f, 0.7f);
			shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
			shader.setVec3("material.ambient", 0.2f, 0.9f, 0.0f);
			shader.setVec3("material.diffuse", 0.2f, 0.9f, 0.0f);
			shader.setVec3("material.specular", 0.2f, 0.9f, 0.0f); // specular lighting doesn't have full effect on this object's material
			shader.setFloat("material.shininess", 32.0f);

			if (moveFlag == true) {
				bvh.setFrame(bvh.root, frameIndex);
				frameIndex++;
				if (frameIndex == bvh.Frames) {
					frameIndex = 0;
				}
			}
			if (resetMatrices == true) {
				bvh.clearVISITED();
				bvh.resetMatrices(bvh.root);
				resetMatrices = false;
				moveFlag = false;
				frameIndex = 0;
			}
			//update will probably be 120.
			update++;
			deltaTime2--;
		}

		//Render here. FPSShower shows maximum possible frames that were renderable!
		glm::mat4 model = glm::mat4(1.0f);
		drawBones(shader, bvh.root, model);
		FPSShower++;

		//Shows how many frames passed for 1 second!
		if (glfwGetTime() - timer > 1.0) {
			timer++;
			std::cout << "FPS: " << FPSShower << " Updated: " << update << " times" << std::endl;
			FPSShower = 0;
			update = 0;
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}


void drawBones(Shader& shader, Bone* rootBone, glm::mat4 model) {	//Where DFS happens
	model = model * rootBone->returnT() * rootBone->returnR();
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000000.0f);
	glm::mat4 view = camera.GetViewMatrix();
	shader.setMat4("model", model);
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	for (int i = 0; i < rootBone->returnnChildren(); i++) {
		glBindVertexArray(rootBone->returnVAOs(i));
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	VISITED[rootBone->returnBoneID()] = 1;
	for (int i = 0; i < rootBone->returnnChildren(); i++) {
		if (!VISITED[rootBone->i_thChild(i)->returnBoneID()])
			drawBones(shader, rootBone->i_thChild(i), model);
	}
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	//If we press spacebar, motion happens
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		moveFlag = true;
		resetMatrices = false;
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		resetMatrices = true;
		moveFlag = false;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
