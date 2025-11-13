//---------------------------------------------------------------------------- 
// This file is the modified version of skeletal_animation.cpp from the
// learnOpenGL github repositoty
//----------------------------------------------------------------------------

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>

#include <iostream>
#include <cmath>

//#define MOVEMENT_DEBUG ;


bool kickFlag = false;
bool punchFlag = false;
bool fightingFlag = false;

bool sprintFlag = false;

// callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);


// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// camera (ใช้แค่ Zoom จาก class เดิม)
Camera camera(glm::vec3(0.0f, 0.0f, 2.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// orbit camera parameters (กล้องหมุนรอบตัวละคร)
glm::vec3 target(0.0f, 0.0f, 0.0f);  // player position at origin
float orbitRadius = 2.0f;            // distance from player
float orbitYaw = -90.0f;          // horizontal angle
float orbitPitch = 20.0f;           // vertical angle

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

enum AnimState {
    IDLE = 1,
    IDLE_PUNCH,
    PUNCH_IDLE,
    IDLE_KICK,
    KICK_IDLE,
    IDLE_WALK,
    WALK_IDLE,
    WALK,
    WALK_SPRINT,
    SPRINT,
    SPRINT_WALK,
    IDLE_BACK,
    BACK,
    BACK_IDLE,
    IDLE_JUMP,
    JUMP,
    JUMP_IDLE
};

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("anim_model.vs", "anim_model.fs");

    // load models
    // -----------
    // 
    // idle 3.3, walk 2.06, run 0.83, punch 1.03, kick 1.6

    Model ourModel(FileSystem::getPath("resources/objects/lewis/lewis.dae"));
    Animation idleAnimation(FileSystem::getPath("resources/objects/lewis/idle.dae"), &ourModel);
    Animation walkAnimation(FileSystem::getPath("resources/objects/lewis/walk.dae"), &ourModel);
    Animation runAnimation(FileSystem::getPath("resources/objects/lewis/run.dae"), &ourModel);
    Animation punchAnimation(FileSystem::getPath("resources/objects/lewis/punch.dae"), &ourModel);
    Animation kickAnimation(FileSystem::getPath("resources/objects/lewis/kick.dae"), &ourModel);
    Animation walkBackAnimation(FileSystem::getPath("resources/objects/lewis/walkback.dae"), &ourModel);

    Animator animator(&idleAnimation);
    enum AnimState charState = IDLE;
    float blendAmount = 0.0f;
    float blendRate = 0.035f;

    bool isAnimationDone = false;

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    printf("Entering Render loop\n");

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        fightingFlag = kickFlag || punchFlag;

        //--------------------------------------------
        //-- Animation State Executor
        //--------------------------------------------

        switch (charState) {
        case IDLE:
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && fightingFlag == false && sprintFlag == false) {
                blendAmount = 0.0f;
                animator.PlayAnimation(&idleAnimation, &walkAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = IDLE_WALK;
            }
            else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && fightingFlag == false && sprintFlag == false) {
                blendAmount = 0.0f;
                animator.PlayAnimation(&idleAnimation, &walkBackAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = IDLE_BACK;
            }
            else if (punchFlag == true) {
                blendAmount = 0.0f;
                animator.PlayAnimation(&idleAnimation, &punchAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = IDLE_PUNCH;
            }
            else if (kickFlag == true) {
                blendAmount = 0.0f;
                animator.PlayAnimation(&idleAnimation, &kickAnimation, animator.m_CurrentTime, 0.0f, blendAmount);
                charState = IDLE_KICK;
            }
#ifdef MOVEMENT_DEBUG
            printf("idle \n");
#endif
            break;
        case IDLE_WALK:
            blendAmount += blendRate;
            blendAmount = fmod(blendAmount, 1.0f);
            animator.PlayAnimation(&idleAnimation, &walkAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount > 0.9f) {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&walkAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = WALK;
            }
#ifdef MOVEMENT_DEBUG
            printf("idle_walk \n");
#endif
            break;
        case WALK:
            animator.PlayAnimation(&walkAnimation, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (glfwGetKey(window, GLFW_KEY_W) != GLFW_PRESS || fightingFlag) {
                charState = WALK_IDLE;
            }
            else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                sprintFlag = true;
                charState = WALK_SPRINT;
            }
#ifdef MOVEMENT_DEBUG
            printf("walking\n");
#endif
            break;
        case WALK_IDLE:
            blendAmount += blendRate;
            blendAmount = fmod(blendAmount, 1.0f);
            animator.PlayAnimation(&walkAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount > 0.9f) {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = IDLE;
            }
#ifdef MOVEMENT_DEBUG
            printf("walk_idle \n");
#endif
            break;

        case IDLE_BACK:
            blendAmount += blendRate;
            blendAmount = fmod(blendAmount, 1.0f);
            animator.PlayAnimation(&idleAnimation, &walkBackAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount > 0.9f) {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&walkBackAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = BACK;
            }
#ifdef MOVEMENT_DEBUG
            printf("idle_walk \n");
#endif
            break;
        case BACK:
            animator.PlayAnimation(&walkBackAnimation, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (glfwGetKey(window, GLFW_KEY_S) != GLFW_PRESS || fightingFlag) {
                charState = BACK_IDLE;
            }
#ifdef MOVEMENT_DEBUG
            printf("walking\n");
#endif
            break;
        case BACK_IDLE:
            blendAmount += blendRate;
            blendAmount = fmod(blendAmount, 1.0f);
            animator.PlayAnimation(&walkBackAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount > 0.9f) {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = IDLE;
            }
#ifdef MOVEMENT_DEBUG
            printf("walk_idle \n");
#endif
            break;
        case WALK_SPRINT:
            blendAmount += blendRate;
            blendAmount = fmod(blendAmount, 1.0f);
            animator.PlayAnimation(&walkAnimation, &runAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount > 0.9f) {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&runAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = SPRINT;
            }
#ifdef MOVEMENT_DEBUG
            printf("walk_sprint \n");
#endif
            break;
        case SPRINT:
            animator.PlayAnimation(&runAnimation, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS || fightingFlag) {
                charState = SPRINT_WALK;
            }

#ifdef MOVEMENT_DEBUG
            printf("sprint \n");
#endif
            break;
        case SPRINT_WALK:
            blendAmount += blendRate;
            blendAmount = fmod(blendAmount, 1.0f);
            animator.PlayAnimation(&runAnimation, &walkAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount > 0.9f) {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&walkAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = WALK;
                sprintFlag = false;
            }

#ifdef MOVEMENT_DEBUG
            printf("sprint_run \n");
#endif
            break;
        case IDLE_PUNCH:
            blendAmount += blendRate;
            blendAmount = fmod(blendAmount, 1.0f);
            animator.PlayAnimation(&idleAnimation, &punchAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount > 0.9f) {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&punchAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = PUNCH_IDLE;
            }

#ifdef MOVEMENT_DEBUG
            printf("idle_punch\n");
#endif
            break;
        case PUNCH_IDLE:
            if (animator.m_CurrentTime > 0.7f) {
                blendAmount += blendRate;
                blendAmount = fmod(blendAmount, 1.0f);
                animator.PlayAnimation(&punchAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
                if (blendAmount > 0.9f) {
                    blendAmount = 0.0f;
                    float startTime = animator.m_CurrentTime2;
                    animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
                    punchFlag = false;
                    charState = IDLE;
                }

#ifdef MOVEMENT_DEBUG
                printf("punch_idle \n");
#endif
            }

#ifdef MOVEMENT_DEBUG
            else {
                // punching
                printf("punching \n");
            }
#endif
            break;
        case IDLE_KICK:
            blendAmount += blendRate;
            blendAmount = fmod(blendAmount, 1.0f);
            animator.PlayAnimation(&idleAnimation, &kickAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
            if (blendAmount > 0.9f) {
                blendAmount = 0.0f;
                float startTime = animator.m_CurrentTime2;
                animator.PlayAnimation(&kickAnimation, NULL, startTime, 0.0f, blendAmount);
                charState = KICK_IDLE;
            }
#ifdef MOVEMENT_DEBUG
            printf("idle_kick\n");
#endif
            break;
        case KICK_IDLE:
            if (animator.m_CurrentTime > 1.0f) {
                blendAmount += blendRate;
                blendAmount = fmod(blendAmount, 1.0f);
                animator.PlayAnimation(&kickAnimation, &idleAnimation, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
                if (blendAmount > 0.9f) {
                    blendAmount = 0.0f;
                    float startTime = animator.m_CurrentTime2;
                    animator.PlayAnimation(&idleAnimation, NULL, startTime, 0.0f, blendAmount);
                    kickFlag = false;
                    charState = IDLE;
                }
#ifdef MOVEMENT_DEBUG
                printf("kick_idle \n");
#endif
            }
#ifdef MOVEMENT_DEBUG
            else {
                // kicking
                printf("kicking \n");
            }
#endif
            break;
        }

        animator.UpdateAnimation(deltaTime);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view/projection transformations
        // 1) คำนวณตำแหน่งกล้องจาก yaw/pitch/radius
        float radYaw = glm::radians(orbitYaw);
        float radPitch = glm::radians(orbitPitch);

        float camX = target.x + orbitRadius * std::cos(radPitch) * std::cos(radYaw);
        float camY = target.y + orbitRadius * std::sin(radPitch);
        float camZ = target.z + orbitRadius * std::cos(radPitch) * std::sin(radYaw);

        glm::vec3 cameraPos(camX, camY, camZ);

        // 2) projection
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // 3) view: กล้องมองไปที่ตัวละครที่ origin
        glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));

        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.4f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(.5f, .5f, .5f)); // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        // glfw: swap buffers and poll IO events
        // -------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input
// -----------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed
// --------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves
// ------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // reversed since y-coordinates go from bottom to top

    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    orbitYaw += xoffset;
    orbitPitch += yoffset;

    if (orbitPitch > 89.0f)  orbitPitch = 89.0f;
    if (orbitPitch < -89.0f) orbitPitch = -89.0f;
}

// glfw: whenever the mouse scroll wheel scrolls
// ---------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    //orbitRadius -= (float)yoffset * 0.3f;
    //if (orbitRadius < 1.0f)  orbitRadius = 1.0f;
    //if (orbitRadius > 15.0f) orbitRadius = 15.0f;
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        punchFlag = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        kickFlag = true;
    }
}
