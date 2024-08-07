#include "pch.h"
#include "VNameComponent.h"

void VNameComponent::SetName(const char* name)
{
    for(m_LastIndex = 0; name[m_LastIndex] != '\0'; ++m_LastIndex)
    {
        m_Name[m_LastIndex] = name[m_LastIndex];
    }
    
    m_Name[m_LastIndex] = '\0';
}

void VNameComponent::Append(const char* str)
{
    for(int i = 0; str[i] != '\0'; ++i)
    {
        m_Name[m_LastIndex++] = str[i];
    }
    
    m_Name[m_LastIndex] = '\0';
}

const char* VNameComponent::GetName()
{
    return m_Name;
}
