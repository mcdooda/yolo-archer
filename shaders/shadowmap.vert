#version 150

// input attributes
in vec3 position;

// input uniforms
uniform mat4 mvpMat;

uniform sampler2D texHeight;

out vec3 pos;

float altitude(vec2 p) {
    return texture(texHeight,p).r;
}

void main() {
    pos = position;
    pos.z = altitude(pos.xy / 2 + 0.5);
    gl_Position = mvpMat * vec4(pos, 1.0);
}
