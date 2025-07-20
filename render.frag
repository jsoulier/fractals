#version 450

layout(location = 0) out vec4 outColor;
layout(set = 3, binding = 0) uniform uniformBuffer
{
    vec2 center;
    uvec2 viewport;
    uint type;
    float zoom;
};

const int MaxIterations = 1000;

int GetMandelbrotIterations(float real, float imag)
{
    int iterations = 0;
    float constReal = real;
    float constImag = imag;
    while (iterations < MaxIterations)
    {
        float tmpReal = real;
        real = (real * real - imag * imag) + constReal;
        imag = (2.0f * tmpReal * imag) + constImag;
        if ((real * real + imag * imag) > 4.0f)
        {
            break;
        }
        iterations++;
    }
    
    return iterations;
}

int GetJuliaIterations(float real, float imag)
{
    int iterations = 0;
    float constReal = -0.7f;
    float constImag = 0.27015f;
    while (iterations < MaxIterations)
    {
        float tmpReal = real;
        real = (real * real - imag * imag) + constReal;
        imag = (2.0f * tmpReal * imag) + constImag;
        if ((real * real + imag * imag) > 4.0f)
        {
            break;
        }
        iterations++;
    }

    return iterations;
}

int GetMultibrot3Iterations(float real, float imag)
{
    int iterations = 0;
    float constReal = real;
    float constImag = imag;
    while (iterations < MaxIterations)
    {
        float real2 = real * real;
        float imag2 = imag * imag;
        float newReal = real * real2 - 3.0f * real * imag2 + constReal;
        float newImag = 3.0f * real2 * imag - imag * imag2 + constImag;
        real = newReal;
        imag = newImag;
        if ((real * real + imag * imag) > 4.0f)
        {
            break;
        }
        iterations++;
    }

    return iterations;
}

int GetPhoenixIterations(float real, float imag)
{
    int iterations = 0;
    float constReal = -0.5f;
    float constImag = 0.0f;
    float p = -0.5f;
    float oldImag = 0.0f;
    while (iterations < MaxIterations)
    {
        float tmpReal = real;
        float newReal = real * real - imag * imag + constReal;
        float newImag = 2.0f * tmpReal * imag + constImag + p * oldImag;
        oldImag = imag;
        real = newReal;
        imag = newImag;
        if ((real * real + imag * imag) > 4.0f)
        {
            break;
        }
        iterations++;
    }

    return iterations;
}

int GetNewtonIterations(float real, float imag)
{
    int iterations = 0;
    while (iterations < MaxIterations)
    {
        float r2 = real * real;
        float i2 = imag * imag;
        float fReal = real * (r2 - 3.0f * i2) - 1.0f;
        float fImag = imag * (3.0f * r2 - i2);
        float fpReal = 3.0f * (r2 - i2);
        float fpImag = 6.0f * real * imag;
        float denom = fpReal * fpReal + fpImag * fpImag;
        if (denom == 0.0f)
        {
            break;
        }
        float newReal = real - (fReal * fpReal + fImag * fpImag) / denom;
        float newImag = imag - (fImag * fpReal - fReal * fpImag) / denom;
        float diff = (newReal - real) * (newReal - real) + (newImag - imag) * (newImag - imag);
        real = newReal;
        imag = newImag;
        if (diff < 0.0001f)
        {
            break;
        }
        iterations++;
    }

    return iterations;
}

void main()
{
    float real = ((gl_FragCoord.x / float(viewport.x) - 0.5f) * zoom + center.x) * 4.0f;
    float imag = ((gl_FragCoord.y / float(viewport.y) - 0.5f) * zoom + center.y) * 4.0f;
    int iterations = MaxIterations;
    switch (type)
    {
    case 0:
        iterations = GetMandelbrotIterations(real, imag);
        break;
    case 1:
        iterations = GetJuliaIterations(real, imag);
        break;
    case 2:
        iterations = GetMultibrot3Iterations(real, imag);
        break;
    case 3:
        iterations = GetPhoenixIterations(real, imag);
        break;
    case 4:
        iterations = GetNewtonIterations(real, imag);
        break;
    }
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