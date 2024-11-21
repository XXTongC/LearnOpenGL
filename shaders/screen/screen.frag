#version 460 core
out vec4 FragColor;
in vec2 uv;

uniform sampler2D screenTextureSampler;
uniform float texWidth;
uniform float texHeight;

vec3 blur();
vec3 colorInvert(vec3 color);
vec3 gray(vec3 color);
vec3 grayCorrect(vec3 color);

void main()
{
    vec3 color = texture(screenTextureSampler,uv).rgb;
    //vec3 color = blur();
    FragColor = vec4(color, 1.0f);
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