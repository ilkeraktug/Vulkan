#pragma once

#define INPUT_FUNCTIONS_ZERO_INPUT(x) public: static bool x() { return s_Instance->x##Impl(); } protected: virtual bool x##Impl() const = 0;
#define INPUT_FUNCTIONS_ONE_INPUT(x) public: static bool x(int input) { return s_Instance->x##Impl(input); } protected: virtual bool x##Impl(int) const = 0;

class Input
{
public:
    virtual ~Input() = default;

    static Input* Get() { return s_Instance; }

//     static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
//     static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
//     static std::pair<uint32_t, uint32_t> GetMousePosition() {}
//     
//     static bool IsCtrlPressed() {}
//     static bool IsAltPressed()  {}
//     static bool IsShiftPressed() {}
//
// protected:
//     virtual bool IsKeyPressedImpl(int keycode) const = 0;
//     virtual bool IsMouseButtonPressedImpl(int button) const = 0;
//     virtual std::pair<uint32_t, uint32_t> GetMousePositionImpl() = 0;
//     
//     virtual bool IsCtrlPressedImpl() const = 0;
//     virtual bool IsAltPressedImpl() const  = 0;
//     virtual bool IsShiftPressedImpl() const = 0;
    static std::pair<double, double> GetMousePosition() { return s_Instance->GetMousePositionImp(); }
    static void SetCursorHidden(bool Hidden) { s_Instance->SetCursorHiddenImp(Hidden); }
    
    INPUT_FUNCTIONS_ONE_INPUT(IsKeyPressed)
    INPUT_FUNCTIONS_ONE_INPUT(IsMouseButtonPressed)
    INPUT_FUNCTIONS_ONE_INPUT(IsKeyReleased)
    INPUT_FUNCTIONS_ONE_INPUT(IsMouseButtonReleased)
    
    INPUT_FUNCTIONS_ZERO_INPUT(IsCtrlPressed)
    INPUT_FUNCTIONS_ZERO_INPUT(IsAltPressed)
    INPUT_FUNCTIONS_ZERO_INPUT(IsShiftPressed)

private:
    virtual std::pair<double, double> GetMousePositionImp() const = 0;
    virtual void SetCursorHiddenImp(bool Hidden) const = 0;
    
private:
    static Input* s_Instance;
};
