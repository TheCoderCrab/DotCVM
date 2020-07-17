#ifndef DC_LINKED_LIST_H
#define DC_LINKED_LIST_H

#include <stdint.h>
#include <log.h>

template<typename T> struct LinkedListElement
{
    T* ptr;
    LinkedListElement<T>* next;
    LinkedListElement(T* _ptr, LinkedListElement<T>* _next)
    {
        ptr = _ptr;
        next = _next;
    }
    LinkedListElement()
    {
        ptr = nullptr;
        next = nullptr;
    }
};

template<typename T> class LinkedList
{
private:
    LinkedListElement<T>* m_FirstElement;
public:
    LinkedList()
    {
        m_FirstElement = nullptr;
    }
    LinkedListElement<T>& elementAt(size_t index)
    {
        LinkedListElement<T>* element = m_FirstElement;
        for(unsigned int i = 0; i < index; i++)
        {
            if(element == nullptr)
                err("Reached end of linked list, index: " << index << ", size: " << i);
            element = element->next;
        }
        return *element;
    }
    T& operator[](uint32_t index)
    {
        return *(elementAt(index).ptr);
    }
    size_t size()
    {
        LinkedListElement<T>* element = m_FirstElement;
        for(size_t i = 0; true; i++)
            if(element == nullptr)
                return i;
            else
                element = element->next;
    }
    LinkedListElement<T>& lastElement()
    {
        return this->elementAt(size() - 1);
    }
    T& lastItem()
    {
        return *lastElement().ptr;
    }
    void add(T element)
    {
        T* ptr;
#ifdef DC_LINKED_LIST_USE_MALLOC // This is for debug.cpp
        ptr = malloc(sizeof(T));
#else
        ptr = new T;
#endif
        *ptr = element;
        if(m_FirstElement == nullptr)
        {
#ifdef DC_LINKED_LIST_USE_MALLOC // This is for debug.cpp
        m_FirstElement = malloc(sizeof(LinkedList<T>));
#else
        m_FirstElement = new LinkedListElement<T>;
#endif
            return;
        }
        LinkedListElement<T>* lnElm;
#ifdef DC_LINKED_LIST_USE_MALLOC // This is for debug.cpp
        lnElm = malloc(sizeof(LinkedList<T>));
#else
        lnElm = new LinkedListElement<T>;
#endif
        lnElm->ptr = ptr;
        lastElement().next = lnElm;
        debug("Added new element");
    }
    void remove(size_t index)
    {
        LinkedListElement<T>* toRemove ;
        if(index == 0)
        {
            toRemove = m_FirstElement;
            m_FirstElement = m_FirstElement->next;
            debug("Removinf first element");
        }
        else if(index == size() - 1)
        {
            toRemove = &elementAt(index);
            elementAt(index - 1).next = nullptr;
            debug("Removinf last element");
        }
        else
        {
            toRemove = &elementAt(index);
            elementAt(index - 1).next = &elementAt(index + 1);
        }
        debug("Removing element at: " << index);
#ifdef DC_LINKED_LIST_USE_MALLOC // This is for debug.cpp
        free(toRemove);
#else
        delete  toRemove;
#endif
    }
};

#endif
