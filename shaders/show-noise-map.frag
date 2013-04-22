#version 150

out vec4 outBuffer;
uniform sampler2D texHeight;

void main() {

  // *** TODO: simply display the (uniform) shadow map for testing ***
    ivec2 loc = ivec2(gl_FragCoord.xy);
    outBuffer = vec4(texelFetch(texHeight,loc,0).xyz,1.0);
}
