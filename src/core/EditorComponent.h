#pragma once
#include "Component.h"

class EditorComponent : public Component {
public:
    virtual void UpdateEditor() = 0;
};