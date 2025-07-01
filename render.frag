#version 450

layout(location = 0) out vec4 outColor;
layout(set = 3, binding = 0) uniform uniformBuffer
{
    vec2 center;
    uvec2 viewport;
    float zoom;
};

const int MaxIterations = 1000;

int getIterations()
{
    float real = ((gl_FragCoord.x / viewport.x - 0.5f) * zoom + center.x) * 4.0f;
    float imag = ((gl_FragCoord.y / viewport.y - 0.5f) * zoom + center.y) * 4.0f;
    int iterations = 0;
    float constReal = real;
    float constImag = imag;
    while (iterations < MaxIterations)
    {
        float tmpReal = real;
        real = (real * real - imag * imag) + constReal;
        imag = (2.0f * tmpReal * imag) + constImag;
        float dist = real * real + imag * imag;
        if (dist > 4.0f)
        {
            break;
        }
        iterations++;
    }
    
    return iterations;
}

void main()
{
    int iterations = getIterations();
    if (iterations == MaxIterations)
    {
        outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return;
    }

    float t = float(iterations) / float(MaxIterations);
    float hue = 0.7f + 10.0f * t;
    float saturation = 1.0f;
    float value = 1.0f;
    vec4 k = vec4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
    vec3 p = abs(fract(vec3(hue) + k.xyz) * 6.0f - k.www);
    vec3 color = value * mix(vec3(1.0f), clamp(p - vec3(1.0f), 0.0f, 1.0f), saturation);
    outColor = vec4(color, 1.0f);
}