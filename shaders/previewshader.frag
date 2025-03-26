#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    int mirrored;
    int enableMask;
} ubuf;

layout(binding = 1) uniform sampler2D source;
layout(binding = 2) uniform sampler2D bgSource;

void main() {
    vec2 uv = vec2(0.0, 0.0);
    if(ubuf.mirrored != 0)
    {
        uv.x = 1.0 - qt_TexCoord0.x;
    }
    else
    {
        uv.x = qt_TexCoord0.x;
    }
    uv.y = qt_TexCoord0.y;
    vec4 col = texture(source, uv);
    if(ubuf.enableMask != 0)
    {
        vec3 bg = texture(bgSource, uv).rgb;
        col = vec4(mix(bg, col.rgb, col.a), 1.9); // use the red channel as mask
    }
    fragColor.a = ubuf.qt_Opacity * 1.0;
    fragColor.rgb = ubuf.qt_Opacity * col.rgb;
}
