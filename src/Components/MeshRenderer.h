#pragma once
#include "../core/Component.h"
#include "../renderer/Model.h"

class MeshRenderer : public Component {
public:
    Model* model;

    explicit MeshRenderer(Model* targetModel) : model(targetModel) {}
};