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
    vec4 sphere;
};

layout(std430, binding = 3) buffer DrawCommandsBlock {
    DrawCommand commands[];
};

layout(std430, binding = 1) buffer InstanceData {
    InstanceProperties instanceProps[];
};

layout(std430, binding = 2) buffer CurrValidInstanceIdx {
    uint currValidInstanceIndices[];
};

layout(location = 0) uniform mat4 viewProjMat;
layout(location = 1) uniform uint numMaxInstance;
layout(location = 2) uniform mat4 viewMat;
layout(location = 3) uniform vec4 v_nearFace;
layout(location = 4) uniform vec4 v_farFace;
layout(location = 5) uniform vec4 v_rightFace;
layout(location = 6) uniform vec4 v_leftFace;
layout(location = 7) uniform vec4 v_topFace;
layout(location = 8) uniform vec4 v_bottomFace;


void main() {
    const uint idx = gl_GlobalInvocationID.x;
    if(idx >= numMaxInstance){
        return;
    }
    vec4 clipSpaceV = viewProjMat * vec4(instanceProps[idx].position.xyz, 1.0) ;
    clipSpaceV = clipSpaceV / clipSpaceV.w ;
    bool frustumCulled = (clipSpaceV.x < -1.0) || (clipSpaceV.x > 1.0) || (clipSpaceV.y < -1.0) || (clipSpaceV.y > 1.0) || (clipSpaceV.z < -1.0) || (clipSpaceV.z > 1.0) ;
    float radius = instanceProps[idx].sphere.w;
    vec4 cameraSpaceV = viewMat * vec4(instanceProps[idx].position.xyz, 1.0) + vec4(instanceProps[idx].sphere.xyz, 0.0);
    bool sphereWithin = (dot(v_nearFace.xyz, cameraSpaceV.xyz) - v_nearFace.w) < radius;
    sphereWithin = sphereWithin && (dot(v_farFace.xyz, cameraSpaceV.xyz) - v_farFace.w) < radius;
    sphereWithin = sphereWithin && (dot(v_rightFace.xyz, cameraSpaceV.xyz)) < radius;
    sphereWithin = sphereWithin && (dot(v_leftFace.xyz, cameraSpaceV.xyz)) < radius;
    sphereWithin = sphereWithin && (dot(v_topFace.xyz, cameraSpaceV.xyz)) < radius;
    sphereWithin = sphereWithin && (dot(v_bottomFace.xyz, cameraSpaceV.xyz)) < radius;

    if (frustumCulled == false || sphereWithin) {
        // put data into valid-instance buffer
        // also update the instance count
        int cmdIdx = -1;
        for(int i = commands.length() - 1; i >= 0; --i){
            if(idx >= commands[i].baseInstance){
                cmdIdx = i;
                break;
            }
        }
        currValidInstanceIndices[commands[cmdIdx].baseInstance + atomicAdd(commands[cmdIdx].instanceCount, 1)] = idx;
    }
}
