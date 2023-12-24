#pragma once

#include <memory>
#include "DynamicSceneObject.h"
#include "MyMesh.h"

class MagicRock
{
public:
    MagicRock();

    DynamicSceneObject *sceneObject() const;

	void update();
private:
    std::shared_ptr<DynamicSceneObject> my_dynamicSO;
};

