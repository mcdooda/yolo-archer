#version 150

// input attributes
in vec3 position;
uniform sampler2D texHeight;

uniform mat4 mdvMat; // modelview matrix (constant for all the vertices)
uniform mat3 normalMat;   // normal matrix
uniform mat4 projMat; // projection matrix (constant for all the vertices)
uniform mat4 mvpMat; // mvp depth matrix

out vec3 pos;
out vec3 normal;
out vec3 normalCam;
out vec4 posLight;

float altitude(vec2 p) {
    return texture(texHeight,p).r;
}

vec3 computeNormal() {
    vec2 p = (position / 2 + 0.5).xy;
    ivec2 size = textureSize(texHeight, 0);
    float dx = 1.0 / size.x;
    float dy = 1.0 / size.y;
    vec2 g = vec2(
        (altitude(vec2(p.x + dx, p.y)) - altitude(p)) / dx,
        (altitude(vec2(p.x, p.y + dy)) - altitude(p)) / dy
    );
    return normalize(vec3(g, 1));
}

void main() {
    pos = position;
    pos.z = altitude(pos.xy / 2 + 0.5);
    posLight = mvpMat * vec4(pos, 1.0);
    normal = computeNormal();
    normalCam = normalMat * normal;
    gl_Position = projMat*mdvMat*vec4(pos,1.0);
}
