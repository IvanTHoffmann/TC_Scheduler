#ifndef VECTORINTERFACE_H
#define VECTORINTERFACE_H

#include <RmlUi/Core.h>
#include "json11.hpp"
#include <vector>
#include <array>

using namespace std;
using namespace json11;

typedef int DepartmentID_t;
typedef int ServiceIndex_t;

template <class _Ty> class SelectedRangeInterface;

template <class _Ty>
class VectorInterface
{
private:
        vector<_Ty> *_target;
        SelectedRangeInterface<_Ty> *_interface;

public:
        using value_type = _Ty;

        VectorInterface(SelectedRangeInterface<_Ty> *interface) : _interface(interface) {}

        void setTarget(vector<_Ty> *newTarget)
        {
                _target = newTarget;
        }

        vector<_Ty>::iterator begin()
        {
                return _target->begin() + _interface->index;
        }

        vector<_Ty>::iterator end()
        {
                return _target->begin() + _interface->index + size();
        }

        _Ty *ptr();

        size_t size()
        {
                if (_target == nullptr)
                {
                        return 0;
                }
                if (_interface->index < 0)
                {
                        return 0;
                }
                if (_interface->index + _interface->size > _target->size())
                {
                        return 0;
                }
                return _interface->size;
        }
};

template <class _Ty>
_Ty *VectorInterface<_Ty>::ptr()
{
        return size() ? &(_target->at(_interface->index)) : nullptr;
}

template <class _Ty>
class SelectedRangeInterface
{
public:
        VectorInterface<_Ty> accessor;
        int index;
        int size;

        SelectedRangeInterface() : accessor(this), index(-1) {}
        void setTarget(vector<_Ty> *newTarget)
        {
                accessor.setTarget(newTarget);
        }
};

#endif // VECTORINTERFACE_H