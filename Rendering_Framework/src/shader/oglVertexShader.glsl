#version 430 core

layout(location=0) in vec3 v_vertex;
layout(location=1) in vec3 v_normal ;
layout(location=2) in vec3 v_uv ;
layout(location=3) in uint v_offsetIdx;
layout(location=4) in vec3 v_tangent;
layout(location=5) in vec3 v_bitangent;

out vec3 f_viewVertex ;
out vec3 f_uv ;

out VS_OUT{
	vec3 N;
	vec3 L;
	vec3 V;
} vs_out;

out mat4 f_viewMat;
out mat3 f_TBN;

layout(location = 0) uniform mat4 modelMat ;
layout(location = 5) uniform sampler2D elevationMap ;
layout(location = 6) uniform sampler2D normalMap ;
layout(location = 7) uniform mat4 viewMat ;
layout(location = 8) uniform mat4 projMat ;
layout(location = 9) uniform mat4 terrainVToUVMat;
layout(location = 1) uniform int vertexProcessIdx ;

struct InstanceProperties {
    vec4 position;
    mat4 rotation;
    vec4 sphere;
};

layout(std430, binding = 1) buffer InstanceData {
    InstanceProperties instanceProps[];
};

vec4 viewVertex;
vec4 viewNormal;

void commonProcess(){
	vec4 worldVertex = modelMat * vec4(v_vertex, 1.0) ;
	vec4 worldNormal = modelMat * vec4(v_normal, 0.0) ;

	viewVertex = viewMat * worldVertex ;
	viewNormal = viewMat * worldNormal ;
	
	f_viewVertex = viewVertex.xyz;
	f_uv = v_uv ;

	vec3 T = normalize(vec3(modelMat * vec4(v_tangent, 0.0)));
	vec3 B = normalize(vec3(modelMat * vec4(v_bitangent, 0.0)));
	vec3 N = normalize(vec3(modelMat * vec4(v_normal, 0.0)));
	f_TBN = mat3(T, B, N);

	gl_Position = projMat * viewVertex ;
}

void terrainProcess(){
	vec4 worldV = modelMat * vec4(v_vertex, 1.0) ;
	// calculate uv
	vec4 uv = terrainVToUVMat * worldV ;
	uv.y = uv.z ;
	// get height from map
	float h = texture(elevationMap, uv.xy).r ;
	worldV.y = h;		
	// get normal from map
	vec4 normalTex = texture(normalMap, uv.xy) ;
	// [0, 1] -> [-1, 1]
	normalTex = normalTex * 2.0 - 1.0 ;
		
	// transformation	
	viewVertex = viewMat * worldV ;
	viewNormal = viewMat * vec4(-normalTex.r, normalTex.g, -normalTex.b, 0) ;	
	
	f_viewVertex = viewVertex.xyz;
	f_uv = uv.xyz ;

	gl_Position = projMat * viewVertex ;
}

void offsetProcess(){
	vec3 offset = instanceProps[v_offsetIdx].position.xyz;
	mat4 rotation = instanceProps[v_offsetIdx].rotation;
	vec3 rotatedVertex = (rotation * vec4(v_vertex, 1.0)).xyz;
	vec4 worldVertex = vec4(rotatedVertex + offset, 1.0);
	vec4 worldNormal = rotation * vec4(v_normal, 0.0) ;

	viewVertex = viewMat * worldVertex ;
	viewNormal = viewMat * worldNormal ;
	
	f_viewVertex = viewVertex.xyz;
	f_uv = v_uv ;

	gl_Position = projMat * viewVertex ;
}

void main(){
	f_viewMat = viewMat;
	if(vertexProcessIdx == 0){
		commonProcess() ;
	}
	else if(vertexProcessIdx == 3){
		terrainProcess() ;
	}
	else if(vertexProcessIdx == 4){
		offsetProcess();
	}
	else{
		commonProcess() ;
	}
	vs_out.N = viewNormal.xyz;
	vs_out.L = (mat4(viewMat) * vec4(0.4, 0.5, 0.8, 0.0)).xyz;
	vs_out.V = -viewVertex.xyz;
}