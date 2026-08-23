#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>
#include "EventEmitter.h"

enum class KeyState {
    None,
    Pressed,
    Held,
    Released
};

class InputManager {
public:
    using KeyEvent = EventEmitter<>;
    using MouseEvent = EventEmitter<float, float>;
    using ScrollEvent = EventEmitter<float>;

    explicit InputManager(GLFWwindow* window);
    ~InputManager() = default;

    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    void Update();

    KeyEvent::ListenerID SubscribePress(int keyCode, KeyEvent::Action action);
    KeyEvent::ListenerID SubscribeHold(int keyCode, KeyEvent::Action action);
    KeyEvent::ListenerID SubscribeRelease(int keyCode, KeyEvent::Action action);
    
    MouseEvent::ListenerID SubscribeMouseMove(MouseEvent::Action action);
    ScrollEvent::ListenerID SubscribeScroll(ScrollEvent::Action action);

private:
    struct KeyData {
        KeyState state = KeyState::None;
        KeyEvent onPress;
        KeyEvent onHold;
        KeyEvent onRelease;
    };

    GLFWwindow* m_window;
    std::unordered_map<int, KeyData> m_keyRegistry;
    MouseEvent m_onMouseMove;
    ScrollEvent m_onScroll;

    bool m_firstMouse = true;
    float m_lastX = 0.0f;
    float m_lastY = 0.0f;

    void ProcessKeyState(int keyCode, KeyData& data);
    
    static void MouseCallbackThunk(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallbackThunk(GLFWwindow* window, double xoffset, double yoffset);
    
    void HandleMouseMove(double xpos, double ypos);
    void HandleScroll(double yoffset);
};