#pragma once

enum class EventType 
{
    None = 0,
    MouseScrolled
};

class Event
{
    friend class EventDispatcher;
public:
    virtual ~Event() {}

    virtual EventType GetEventType() const = 0;

private:
    
    bool m_Handled = false;
};

class MouseScrollEvent : public Event
{
public:
    MouseScrollEvent(double xScroll, double yScroll)
        : m_XScroll(xScroll), m_YScroll(yScroll) {}
    
    static EventType GetStaticType()  { return EventType::MouseScrolled; }
    virtual EventType GetEventType() const override { return GetStaticType(); }

    std::string ToString() const
    {
        std::stringstream ss;
        ss << "xScroll " << m_XScroll << " yScroll " << m_YScroll << " \n";
        return ss.str();
    }

    double GetXScroll() const { return m_XScroll; }
    double GetYScroll() const { return m_YScroll; }
private:
    double m_XScroll = 0.0;
    double m_YScroll = 0.0;
};

class EventDispatcher
{
    template<class T>
    using EventFn = std::function<bool(T&)>;
    
public:
    EventDispatcher(Event& e)
        : m_Event(e) {}

    template<class T>
    bool Dispatch(EventFn<T> fn)
    {
        if (m_Event.GetEventType() == T::GetStaticType())
        {
            m_Event.m_Handled = fn(*(T*)&m_Event);
            return true;
        }

        return false;
    }

    Event& m_Event;
};