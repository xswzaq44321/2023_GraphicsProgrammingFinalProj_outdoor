#version 430 core

in vec3 f_viewVertex ;
in vec3 f_uv ;

in VS_OUT{
	vec3 N;
	vec3 L;
	vec3 V;
	mat3 TBN;
	// #NEW
	vec3 vertex;
	vec3 normal;
	//
} fs_in;

layout (location = 0) out vec4 fragColor ;

layout(location = 2) uniform int pixelProcessId;
layout(location = 4) uniform sampler2D albedoTexture ;
layout(location = 6) uniform sampler2D normalMap ;
layout(location = 10) uniform sampler2DArray albedoTextureArray;
layout(location = 11) uniform int mode; // 0 = NONE, 1 = POS, 2 = NORMAL, 3 = DIFFUSE = AMBIENT, 4 = SPECULAR


vec4 withFog(vec4 color){
	const vec4 FOG_COLOR = vec4(0.0, 0.0, 0.0, 1) ;
	const float MAX_DIST = 400.0 ;
	const float MIN_DIST = 350.0 ;
	
	float dis = length(f_viewVertex) ;
	float fogFactor = (MAX_DIST - dis) / (MAX_DIST - MIN_DIST) ;
	fogFactor = clamp(fogFactor, 0.0, 1.0) ;
	fogFactor = fogFactor * fogFactor ;
	
	vec4 colorWithFog = mix(FOG_COLOR, color, fogFactor) ;
	return colorWithFog ;
}

vec4 phong(vec3 N, vec3 L, vec3 V, int shininess, vec3 S){
	vec3 Ia = vec3(0.2, 0.2, 0.2);
	vec3 Id = vec3(0.64, 0.64, 0.64);
	vec3 Is = vec3(0.16, 0.16, 0.16);

	N = normalize(N);
	L = normalize(L);
	V = normalize(V);
	vec3 H = normalize(L + V);

	vec3 ambient = Ia;
	vec3 diffuse = max(dot(N, L), 0.0) * Id;
	vec3 specular = pow(max(dot(N, H), 0.0), shininess) * Is * S;

	return vec4(ambient + diffuse + specular, 1.0);
}

vec4 phong(vec3 N, vec3 L, vec3 V) {
	return phong(N, L, V, 1, vec3(0.0, 0.0, 0.0));
}

void terrainPass(){
	vec4 texel = texture(albedoTexture, f_uv.xy) ;
	fragColor = withFog(texel * phong(fs_in.N, fs_in.L, fs_in.V)); 
	fragColor.a = 1.0;
}

void pureColor(){
	fragColor = withFog(vec4(1.0, 0.0, 0.0, 1.0)) ;
}

void texturePass(){
	vec4 texel = texture(albedoTexture, f_uv.xy) ;
	fragColor = withFog(texel * phong(fs_in.N, fs_in.L, fs_in.V)); 
	fragColor.a = 1.0;	
}

void planePass(){
	vec4 texel = texture(albedoTexture, f_uv.xy) ;
	vec3 specular = vec3(1.0, 1.0, 1.0);
	fragColor = withFog(texel * phong(fs_in.N, fs_in.L, fs_in.V, 32, specular)); 
	fragColor.a = 1.0;	
}

void stonePass(){
	vec4 texel = texture(albedoTexture, f_uv.xy);
	vec3 specular = vec3(1.0, 1.0, 1.0);
	fragColor = withFog(texel * phong(fs_in.N, fs_in.L, fs_in.V, 32, specular)); 
	fragColor.a = 1.0;	
}

void stonePassTex(){
	vec4 texel = texture(albedoTexture, f_uv.xy);
	vec3 N = texture(normalMap, f_uv.xy).xyz;
	// [0, 1] -> [-1, 1]
	N = N * 2.0 - 1.0 ;
	N = normalize(mat3(fs_in.TBN) * N);
	vec3 specular = vec3(1.0, 1.0, 1.0);
	fragColor = withFog(texel * phong(N, fs_in.L, fs_in.V, 32, specular)); 
	fragColor.a = 1.0;	
}

void texArrPass(){
	vec4 texel = texture(albedoTextureArray, f_uv);
	if(texel.a < 0.5){
		discard;
	}
	fragColor = withFog(texel * phong(fs_in.N, fs_in.L, fs_in.V)); 
	fragColor.a = 1.0;	
}

void main(){
	if (mode == 0){
		if(pixelProcessId == 5){
			pureColor() ;
		}
		else if(pixelProcessId == 7){
			terrainPass() ;
		}
		else if(pixelProcessId == 9){
			texturePass();
		}
		else if(pixelProcessId == 10){
			stonePass();
		}
		else if(pixelProcessId == 13){
			stonePassTex();
		}
		else if(pixelProcessId == 11){
			texArrPass();
		}
		else if(pixelProcessId == 12){
			planePass();
		}
		else{
			pureColor() ;
		}
		fragColor.rgb = pow(fragColor.rgb, vec3(0.5));
	}
	// #NEW
	if (mode == 1) {
		fragColor = normalize(vec4(fs_in.vertex, 1.0)) * 0.5 + 0.5;
	}
	if (mode == 2) {
		if(pixelProcessId == 13){
			vec3 N = texture(normalMap, f_uv.xy).xyz;
			// [0, 1] -> [-1, 1]
			N = N * 2.0 - 1.0 ;
			N = normalize(mat3(fs_in.TBN) * N);
			fragColor = normalize(vec4(N, 1.0)) * 0.5 + 0.5;
		}else{
			fragColor = normalize(vec4(fs_in.normal, 1.0)) * 0.5 + 0.5;
		}
	}
	if (mode == 3) {
		fragColor = vec4(texture(albedoTexture, f_uv.xy).xyz, 1.0);
		if (pixelProcessId == 11) {
			fragColor = vec4(texture(albedoTextureArray, f_uv).xyz, 1.0);
		}
	}
	if (mode == 4) {
		fragColor = vec4(0.0, 0.0, 0.0, 1.0);
		if (pixelProcessId == 10 || pixelProcessId == 12 || pixelProcessId == 13) {
			fragColor = vec4(1.0);
		}
	}
	//
}