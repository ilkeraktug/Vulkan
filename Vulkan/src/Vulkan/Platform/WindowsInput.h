#pragma once
#include "Vulkan/Core/Input.h"

class MouseScrollEvent;
class Event;

class WindowsInput : public Input
{
public:
    WindowsInput();
    ~WindowsInput() override;
    
    static bool OnEvent(Event& e);

protected:
    bool IsKeyPressedImpl(int) const override;
    bool IsMouseButtonPressedImpl(int) const override;
    bool IsKeyReleasedImpl(int) const override;
    bool IsMouseButtonReleasedImpl(int) const override;
    bool IsCtrlPressedImpl() const override;
    bool IsAltPressedImpl() const override;
    bool IsShiftPressedImpl() const override;

    std::pair<double, double> GetMousePositionImp() const override;
    void SetCursorHiddenImp(bool Hidden) const override;

private:
    static bool OnMouseScrollEvent(MouseScrollEvent& e);
};
