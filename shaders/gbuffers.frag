#version 150

//uniform sampler2DShadow texDepth;
uniform sampler2DShadow texDepthLight;
uniform vec3 light;

// output vectors (camera space)
in  vec3  eyeView;

in vec3 pos;
in vec3 normal;
in vec3 normalCam;
in vec4 posLight;
in float visibility;

out vec4 outNormal;
out vec4 outPosition;
out vec4 outPositionLight;
out float outSlant;
out float outVisibility;

vec2 poissonDisk[16] = vec2[](
            vec2( -0.94201624, -0.39906216 ),
            vec2( 0.94558609, -0.76890725 ),
            vec2( -0.094184101, -0.92938870 ),
            vec2( 0.34495938, 0.29387760 ),
            vec2( -0.91588581, 0.45771432 ),
            vec2( -0.81544232, -0.87912464 ),
            vec2( -0.38277543, 0.27676845 ),
            vec2( 0.97484398, 0.75648379 ),
            vec2( 0.44323325, -0.97511554 ),
            vec2( 0.53742981, -0.47373420 ),
            vec2( -0.26496911, -0.41893023 ),
            vec2( 0.79197514, 0.19090188 ),
            vec2( -0.24188840, 0.99706507 ),
            vec2( -0.81409955, 0.91437590 ),
            vec2( 0.19984126, 0.78641367 ),
            vec2( 0.14383161, -0.14100790 )
            );

float random(vec3 seed, int i){
    vec4 seed4 = vec4(seed,i);
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

float getVisibility() {
    vec4 shadCoord = posLight / 2.0 + 0.5;
    float bias = 0.01;
    float v = 1;

    for (int i=0;i<16;i++){
        int ind = int(16.0*random(gl_FragCoord.xyy, i))%16;
        if (texture(texDepthLight,vec3(shadCoord.xy+poissonDisk[ind] / 400,(shadCoord.z-bias)/shadCoord.w)) < 1){
            v-=0.1;
        }
    }

    if (v < 0)
        v = 0;

    return v * 0.6 + 0.4;
}

void main() {
    outPositionLight = posLight / 2.0 + 0.5;
    outSlant = normal.z;
    outVisibility = getVisibility();
    outNormal = vec4(normalize(normalCam),1.0) / 2.0 + 0.5;
    outPosition = vec4(pos,1.0) / 2.0 + 0.5;
}
