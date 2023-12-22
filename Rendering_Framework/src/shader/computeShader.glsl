#version 430 core

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct DrawCommand {
    uint count;
    uint instanceCount;
    uint firstIndex;
    uint baseVertex;
    uint baseInstance;
};

struct InstanceProperties {
    vec4 position;
    mat4 rotation;
};

layout(std430, binding = 3) buffer DrawCommandsBlock {
    DrawCommand commands[];
};

layout(std430, binding = 1) buffer InstanceData {
    InstanceProperties instanceProps[];
};

layout(std430, binding = 2) buffer CurrValidInstanceData {
    InstanceProperties currValidInstanceProps[];
};

layout(location = 0) uniform mat4 viewProjMat;
layout(location = 1) uniform int numMaxInstance;

void main() {
    const uint idx = gl_GlobalInvocationID.x;
    if(idx >= numMaxInstance){
        return;
    }
    vec4 clipSpaceV = viewProjMat * vec4(instanceProps[idx].position.xyz, 1.0) ;
    clipSpaceV = clipSpaceV / clipSpaceV.w ;
    bool frustumCulled = (clipSpaceV.x < -1.0) || (clipSpaceV.x > 1.0) || (clipSpaceV.y < -1.0) || (clipSpaceV.y > 1.0) || (clipSpaceV.z < -1.0) || (clipSpaceV.z > 1.0) ;
    
    if (frustumCulled == false) {
        // put data into valid-instance buffer
        // also update the instance count
        int cmdIdx = -1;
        for(int i = commands.length() - 1; i >= 0; --i){
            if(idx >= commands[i].baseInstance){
                cmdIdx = i;
                break;
            }
        }
        currValidInstanceProps[commands[cmdIdx].baseInstance + atomicAdd(commands[cmdIdx].instanceCount, 1)] = instanceProps[idx];
    }
}
