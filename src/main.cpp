#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

const char* vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "void main()\n"
                                 "{\n"
                                 "gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0f);"
                                 "}\n";
const char* fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "{\n"
                                   "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
                                   "}\n";

int main() {
    glfwInit();
    // 设置主要和次要版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // 核心模式
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建一个窗口对象, 这个窗口对象存放了所有和窗口相关的数据，而且会被GLFW的其他函数频繁地用到
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "window create fail!" << "\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD!" << "\n";
        glfwTerminate();
        return -1;
    }

    // 设置视口
    glViewport(0, 0, 800, 600);

    // 注册视口变化监听函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 定义顶点数组，位于Cpu上
    // clang-format off
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    // clang-format on

    // 创建VBO（顶点缓冲对象）位于GPU上，
    unsigned int VBO;
    glGenBuffers(1, &VBO); // id

    // 绑定缓冲， GL_ARRAY_BUFFER是缓冲区用途的类型enum
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 填充数据，把cpu数据填充到GPU上，
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性指针 一句话区别：**VBO 管 "数据存了什么"，顶点属性指针管 "GPU 怎么读这些数据"。**
    //**VBO = 一串没有格式的原始数字**
    // 顶点属性指针 = 给 GPU 的 "读表规则"
    // 告诉 GPU：**"这堆数字，每 6 个算一个顶点；前 3 个是位置 (x,y,z)，后 3 个是颜色 (r,g,b)"**。
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    /*
    VBO,顶点属性，VAO，EBO的关系：
    VBO = 一串没有格式的原始数字
    顶点属性里包含了VBO，以及GPU取用VBO的规则，即几个元素算一个顶点
    VAO相当于顶点属性和EBO的array
    EBO相当于，要画一个图形，应该使用哪些顶点，有点像三角面片里的顶点索引，
    */

    // 创建顶点和片段着色器
    unsigned int vertexShader, fragmentShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // 附加着色器源码，c语言硬编码
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // 编译着色器源码
    glCompileShader(vertexShader);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // 检测是否编译成功
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR:Shader vertex compilation failed" << infoLog << "\n";
    }

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR:Shader fragment compilation failed" << infoLog << "\n";
    }

    // 创建着色器程序对象, 着色器程序对象，相当于把所有着色器链到一起。
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    // 将着色器附加到程序对象上
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // 链接
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR:shader_program_link failed" << infoLog << "\n";
    }

    // 使用着色器程序
    glUseProgram(shaderProgram);

    // 删除着色器(链接到程序里，编译好的shader就可以删掉了)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        // 键盘输入
        processInput(window);

        // 渲染指令
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 交换颜色缓冲，
        glfwSwapBuffers(window);
        // 监听用户输入事件
        glfwPollEvents();
    }

    // 释放资源
    glfwTerminate();

    return 0;
}

// 实现当窗口大小改变时，视口及时跟随变化
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Esc键退出
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        std::cout << "esc press" << "\n";
        glfwSetWindowShouldClose(window, true);
    }
}