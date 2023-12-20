#pragma once

#include <memory>
#include "DynamicSceneObject.h"
#include "MyMesh.h"

class Plane
{
public:
    Plane();

    DynamicSceneObject *sceneObject() const;
private:
    std::shared_ptr<DynamicSceneObject> my_dynamicSO;
    MyMesh mesh;
};

