#pragma once
#include <vector>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include "Component.h"

class Entity {
private:
    std::unordered_map<std::type_index, std::unique_ptr<Component>> componentMap;
    std::vector<Component*> updateList;
    bool isActive = true;

public:
    std::string name;

    explicit Entity(std::string entityName = "Entity") : name(std::move(entityName)) {}

    template <typename T, typename... TArgs>
    T* AddComponent(TArgs&&... mArgs) {
        static_assert(std::is_base_of<Component, T>::value);
        
        auto newComponent = std::make_unique<T>(std::forward<TArgs>(mArgs)...);
        newComponent->owner = this;
        T* rawPtr = newComponent.get(); 

        componentMap[std::type_index(typeid(T))] = std::move(newComponent);
        updateList.push_back(rawPtr);
        
        rawPtr->Awake();
        return rawPtr;
    }

    template <typename T>
    T* GetComponent() const {
        auto it = componentMap.find(std::type_index(typeid(T)));
        if (it != componentMap.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    template <typename T>
    std::vector<T*> GetComponents() const {
        std::vector<T*> result;
        for (auto* component : updateList) {
            if (T* casted = dynamic_cast<T*>(component)) {
                result.push_back(casted);
            }
        }
        return result;
    }

    void Update(float deltaTime) {
        if (!isActive) return;
        for (auto* component : updateList) {
            component->Update(deltaTime);
        }
    }
    
    bool IsActive() const { return isActive; }
    void SetActive(bool state) { isActive = state; }
};