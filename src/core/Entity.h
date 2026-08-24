#pragma once
#include <vector>
#include <memory>
#include <string>
#include <type_traits>
#include "Component.h"

class Entity {
private:
    std::vector<std::unique_ptr<Component>> components;
    bool isActive = true;

public:
    std::string name;

    explicit Entity(std::string entityName = "Entity") : name(std::move(entityName)) {}

    template <typename T, typename... TArgs>
    T* AddComponent(TArgs&&... mArgs) {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        //Create another component on the HEAP
        auto newComponent = std::make_unique<T>(std::forward<TArgs>(mArgs)...);

        newComponent->owner = this;
        T* rawPtr = newComponent.get(); //get pointer of the new component 

        //store the component
        components.push_back(std::move(newComponent));
        
        rawPtr->Awake();
        return rawPtr;
    }

    template <typename T>
    T* GetComponent() const {
        for (const auto& component : components) {
            if (T* cmp = dynamic_cast<T*>(component.get())) {
                return cmp;
            }
        }
        return nullptr;
    }

    void Update(float deltaTime) {
        if (!isActive) return;
        for (auto& component : components) {
            component->Update(deltaTime);
        }
    }
};