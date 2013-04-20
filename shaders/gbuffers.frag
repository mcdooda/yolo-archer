#version 150

uniform sampler2DShadow texDepth;
uniform vec3 light;

// output vectors (camera space)
in  vec3  eyeView;

in vec3 pos;
in vec3 normal;
in vec3 normalCam;
in vec4 posLight;

out vec4 outNormal;
out vec4 outPosition;
out vec4 outPositionLight;
out float outSlant;

void main() {
    vec4 shadCoord = posLight / 2 + vec4(0.5);

    outPositionLight = shadCoord;
    outSlant = normal.z;
    outNormal = vec4(normalize(normalCam),1.0);
    outPosition = vec4(pos,1.0) / 2 + 0.5;
}
