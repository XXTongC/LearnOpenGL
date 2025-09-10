#version 460 core

// epsilon number
const float EPSILON = 0.00001f;

out vec4 FragColor;
in vec2 uv;

uniform sampler2D screenTextureSampler;
uniform sampler2D depthTextureSampler;
uniform float texWidth;
uniform float texHeight;

// calculate floating point numbers equality accurately
bool isApproximatelyEqual(float a, float b);
// get the max value between three values
float max3(vec3 v);
vec3 blur();
vec3 colorInvert(vec3 color);
vec3 gray(vec3 color);
vec3 grayCorrect(vec3 color);
vec3 toneMappingReinhard(vec3 hdrColor);
vec3 toneMappingExposure(vec3 hdrColor);

void main()
{
/*
     // fragment coordination
    ivec2 coords = ivec2(gl_FragCoord.xy);

    // fragment revealage
    float revealage = texelFetch(depthTextureSampler, coords, 0).r;

    // save the blending and color texture fetch cost if there is not a transparent fragment
    if (isApproximatelyEqual(revealage, 1.0f))
        discard;

    // fragment color
    vec4 accumulation = texelFetch(screenTextureSampler, coords, 0);

    // suppress overflow
    if (isinf(max3(abs(accumulation.rgb))))
        accumulation.rgb = vec3(accumulation.a);

    // prevent floating point precision bug
    vec3 average_color = accumulation.rgb / max(accumulation.a, EPSILON);

    // blend pixels
    FragColor = vec4(average_color, 1.0f - revealage);
*/
    vec3 color = texture(screenTextureSampler,uv).rgb;
    color = toneMappingExposure(color);
    // 1 将sRGB转换为RGB
    //color = pow(color,vec3(2.2));
    // 2 与光照进行计算、

    // 3 最终计算结果要抵抗屏幕gamma
   
    color = pow(color,vec3(1.0/2.2));
    //vec3 color = blur();
    FragColor = vec4(color, 1.0f);
    
}

uniform float exposure;
vec3 toneMappingExposure(vec3 hdrColor)
{
    return (vec3(1.0) - exp(-hdrColor * exposure));
}

vec3 toneMappingReinhard(vec3 hdrColor)
{
    return hdrColor / (hdrColor + vec3(1.0));
}

// calculate floating point numbers equality accurately
bool isApproximatelyEqual(float a, float b)
{
    return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

// get the max value between three values
float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

vec3 colorInvert(vec3 color)
{
    vec3 invertColor = vec3(1.0f) - color;
    return invertColor;
}

vec3 gray(vec3 color)
{
    float avg = (color.r+color.g+color.b)/3.0f;
    return vec3(avg);
}
//人眼对绿色敏感度高，蓝色低，故增加绿色权重，降低蓝色权重
vec3 grayCorrect(vec3 color)
{
    float avg = color.r * 0.2126f + color.g * 0.7152 + color.b * 0.0722;
    return vec3(avg);
}

vec3 blur()
{
    float du = 1/texWidth;
    float dv = 1/texHeight;
    //偏移值数组
    vec2 offsets[9] = vec2[](
        vec2(-du,dv),   //左上
        vec2(0.0f,dv),  //正上
        vec2(du,dv),    //右上
        vec2(-du,0.0f), //左
        vec2(0.0f,0.0f),//不动
        vec2(du,0.0f),  //右
        vec2(-du,-dv),  //左下
        vec2(0.0f,-dv), //下
        vec2(du,-dv)   //右下
    );

    //卷积核
    float kernel[9] = float[](
        1.0f,2.0f,1.0f,
        2.0f,4.0f,2.0f,
        1.0f,2.0f,1.0f
    );

    //加权相加
    vec3 sumColor = vec3(0.0f);
    for(int i = 0;i<9;i++)
    {
        vec3 sampleColor = texture(screenTextureSampler, uv + offsets[i]).rgb;
        sumColor += sampleColor * kernel[i];
    }
    sumColor /= 16.0f;
    return sumColor;
}