/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * main.cpp
 *
 * Entry point for Desktop (Windows/GLFW) and WebGL (Emscripten) builds.
 * Android uses NativeTemplate.cpp + JNI instead of this file.
 *
 * main.cpp communicates only with Renderer::Instance(), which manages
 * Scene3D and SceneHUD. It is responsible for application startup,
 * the render loop, and forwarding mouse/touch input to the renderer.
 *
 * The left mouse button is mapped to the same touch interface used on
 * Android (down / move / up), providing consistent interaction across
 * Desktop, Web, and Android builds. Mouse clicks interact with the HUD
 * buttons and perform fan picking within the 3D scene.
 *
 * Emscripten supports two windowing backends (selected at compile time):
 *   USE_GLFW defined  → GLFW via -s USE_GLFW=3 (default)
 *   USE_GLFW absent   → SDL2 via -s USE_SDL=2
 *
 * Quiz 3 demonstrates a split architecture consisting of:
 *   - Scene3D: Renders the fan, ground plane, and Phong lighting.
 *   - SceneHUD: Renders the screen-space control buttons and speed bar.
 *
 * Ported from OpenGL ES 3.0 Cookbook – Chapter 2, GLPI Framework Intro.
 */

#include "Platform.h"
#include "Renderer.h"

// ==========================================================================
// WebGL / Emscripten
// ==========================================================================
#ifdef PLATFORM_EMSCRIPTEN

// --------------------------------------------------------------------------
// GLFW backend (default – build with -DUSE_GLFW -s USE_GLFW=3)
// --------------------------------------------------------------------------
#ifdef USE_GLFW

static GLFWwindow* g_window = nullptr;
static bool g_mouseDown = false;

static void fbsize_cb(GLFWwindow* /*win*/, int w, int h)
{
    Renderer::Instance().resize(w, h);
}

static void mouse_button_cb(GLFWwindow* win, int button, int action, int /*mods*/)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    double x, y;
    glfwGetCursorPos(win, &x, &y);
    if (action == GLFW_PRESS) {
        g_mouseDown = true;
        Renderer::Instance().touchDown((float)x, (float)y);
    } else if (action == GLFW_RELEASE) {
        g_mouseDown = false;
        Renderer::Instance().touchUp();
    }
}

static void cursor_pos_cb(GLFWwindow* /*win*/, double x, double y)
{
    if (g_mouseDown) {
        Renderer::Instance().touchMove((float)x, (float)y);
    }
}

static void main_loop()
{
    if (glfwWindowShouldClose(g_window)) {
        emscripten_cancel_main_loop();
        glfwDestroyWindow(g_window);
        glfwTerminate();
        return;
    }
    glfwPollEvents();
    Renderer::Instance().render();
    glfwSwapBuffers(g_window);
}

int main()
{
    LOGI("Android Programming Quiz 3 - Fan Dashboard HUD (GLFW)");
    LOGI( "+SPD / -SPD (left buttons) : change the fan's spin speed (0-20)");
    LOGI( "+ZOOM / -ZOOM (right buttons) : move the camera closer/farther");
    LOGI( "Click directly on any part of the fan to highlight it (3D picking)");
    LOGI( "ESC to quit.");
    if (!glfwInit()) { LOGE("glfwInit failed"); return -1; }

    glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    g_window = glfwCreateWindow(900, 700, "GLPI Framework Intro", nullptr, nullptr);
    if (!g_window) { LOGE("glfwCreateWindow failed"); glfwTerminate(); return -1; }

    glfwMakeContextCurrent(g_window);
    glfwSetFramebufferSizeCallback(g_window, fbsize_cb);
    glfwSetMouseButtonCallback(g_window, mouse_button_cb);
    glfwSetCursorPosCallback(g_window, cursor_pos_cb);

    Renderer::Instance().initializeRenderer();
    int w, h;
    glfwGetFramebufferSize(g_window, &w, &h);
    Renderer::Instance().resize(w, h);

    emscripten_set_main_loop(main_loop, 0, 1);
    return 0;
}

// --------------------------------------------------------------------------
// SDL2 backend (opt-in – build without -DUSE_GLFW, use -s USE_SDL=2)
// --------------------------------------------------------------------------
#else // !USE_GLFW

static SDL_Window*   g_window = nullptr;
static SDL_GLContext g_glctx  = nullptr;
static bool g_mouseDown = false;

static void main_loop()
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                emscripten_cancel_main_loop();
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT) {
                    g_mouseDown = true;
                    Renderer::Instance().touchDown((float)ev.button.x, (float)ev.button.y);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (ev.button.button == SDL_BUTTON_LEFT) {
                    g_mouseDown = false;
                    Renderer::Instance().touchUp();
                }
                break;
            case SDL_MOUSEMOTION:
                if (g_mouseDown) {
                    Renderer::Instance().touchMove((float)ev.motion.x, (float)ev.motion.y);
                }
                break;
            default:
                break;
        }
    }
    Renderer::Instance().render();
    SDL_GL_SwapWindow(g_window);
}

int main()
{
    LOGI("GLPI Framework Intro : WebGL / Emscripten (SDL2)");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOGE("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    g_window = SDL_CreateWindow(
        "GLPI Framework Intro",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        900, 700,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!g_window) { LOGE("SDL_CreateWindow failed: %s", SDL_GetError()); return -1; }

    g_glctx = SDL_GL_CreateContext(g_window);
    if (!g_glctx) { LOGE("SDL_GL_CreateContext failed: %s", SDL_GetError()); return -1; }

    Renderer::Instance().initializeRenderer();
    Renderer::Instance().resize(900, 700);

    emscripten_set_main_loop(main_loop, 0, 1);
    return 0;
}

#endif // USE_GLFW

// ==========================================================================
// Desktop – Windows / GLFW
// ==========================================================================
#elif defined(PLATFORM_WINDOWS)

#include <iostream>

static bool g_mouseDown = false;

static void framebufferSizeCB(GLFWwindow* /*win*/, int w, int h)
{
    Renderer::Instance().resize(w, h);
}

static void mouseButtonCB(GLFWwindow* win, int button, int action, int /*mods*/)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    double x, y;
    glfwGetCursorPos(win, &x, &y);
    if (action == GLFW_PRESS) {
        g_mouseDown = true;
        Renderer::Instance().touchDown((float)x, (float)y);
    } else if (action == GLFW_RELEASE) {
        g_mouseDown = false;
        Renderer::Instance().touchUp();
    }
}

static void cursorPosCB(GLFWwindow* /*win*/, double x, double y)
{
    if (g_mouseDown) {
        Renderer::Instance().touchMove((float)x, (float)y);
    }
}

int main()
{
    std::cout << "Android Programming Quiz 3 - Fan Dashboard HUD (GLFW)\n";
    std::cout
        << "+SPD / -SPD (left buttons) : change the fan's spin speed (0-20)\n"
        << "+ZOOM / -ZOOM (right buttons) : move the camera closer/farther\n"
        << "Click directly on any part of the fan to highlight it (3D picking)\n"
        << "ESC to quit.\n";

    if (!glfwInit()) { std::cerr << "glfwInit failed\n"; return -1; }

    glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_ANY_PROFILE);

    GLFWwindow* window = glfwCreateWindow(900, 700, "GLPI Framework Intro", nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "glewInit failed\n"; return -1; }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCB);
    glfwSetMouseButtonCallback(window, mouseButtonCB);
    glfwSetCursorPosCallback(window, cursorPosCB);

    Renderer::Instance().initializeRenderer();
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    Renderer::Instance().resize(w, h);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        Renderer::Instance().render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

#endif // PLATFORM_WINDOWS
