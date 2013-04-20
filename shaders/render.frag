#version 150

out vec4 outBuffer;
uniform sampler2D texNormal;
uniform sampler2D texSlant;
uniform sampler2D texPosition;
uniform sampler2D texPositionLight;
uniform sampler2D texDepthCamera;
uniform sampler2DShadow texDepthLight;
uniform sampler2D texColor;
uniform vec3 light;

// used for the last question
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

// used for the last question
float random(vec3 seed, int i){
    vec4 seed4 = vec4(seed,i);
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

float shadowLevel() {
    ivec2 screenSize = textureSize(texNormal, 0);
    float x = float(gl_FragCoord.x) / screenSize.x;
    float y = float(gl_FragCoord.y) / screenSize.y;
    vec4 shadCoord = texture(texPositionLight,vec2(x, y),0);
    float bias = 0.008;
    /*
    int radius = 50;
    float dx = 1 / textureSize(texDepthLight, 0).x;
    float dy = 1 / textureSize(texDepthLight, 0).y;
    float mean = 0;
    int n = 0;
    for (x = -dx * radius; x <= dx * radius; x++) {
        for (y = -dy * radius; y <= dy * radius; y++) {
            mean += texture(texDepthLight, vec3(shadCoord.xy + vec2(x, y),(shadCoord.z-bias)/shadCoord.w));
            n++;
        }
    }
    mean /= n;
    */

    //float v = 1.0; // visibility

    // *** TODO: compute visibility by comparing current depth and the depth in the shadow map ***

    //float t = texture(texDepthLight,vec3(shadCoord.xy,(shadCoord.z-bias)/shadCoord.w));
    float v = 1;

    for (int i=0;i<16;i++){
        int ind = int(16.0*random(gl_FragCoord.xyy, i))%16;
        if (texture(texDepthLight,vec3(shadCoord.xy+poissonDisk[ind] / 300,(shadCoord.z-bias)/shadCoord.w)) < 1){
            v-=0.1;
        }
    }
    if (v < 0.2) v = 0.2;

    return v;
}

void main(void)
{
    ivec2 loc = ivec2(gl_FragCoord.xy);
    vec4 color;

    vec4 normal = texelFetch(texNormal,loc,0);

    vec4 position = texelFetch(texPosition, loc, 0) * 2 - 1;

    //vec4 fragmentColor = vec4(0.4,0.7,0.4,1.0) * (0.5 + texelFetch(texPosition,loc,0).z / 2);
    vec4 fragmentColor = texture(texColor, vec2(position.z, position.x + position.y));
    vec3 ambientColor = vec3(1.0,1.0,1.0);
    vec3 diffuseColor = vec3(1.0,1.0,1.0);
    vec3 specularColor = vec3(1.0,1.0,1.0);
    vec3 n = texelFetch(texNormal,loc,0).xyz;
    vec3 r = vec3(0.0,0.0,1.0);


    color = fragmentColor * vec4(ambientColor + diffuseColor*dot(n,light) + specularColor*pow(dot(reflect(light,n),r),2),1.0);

    float v = shadowLevel();

    outBuffer = color * v;
}