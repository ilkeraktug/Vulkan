#pragma once
#include <glm/fwd.hpp>

class VNameComponent
{
public:
    VNameComponent() = default;
    
    void SetName(const char* name);
    void Append(const char* str);
    const char* GetName();
private:
    char m_Name[16];

    uint32_t m_LastIndex = 0;
};
