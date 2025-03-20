#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
} ubuf;
layout(binding = 1) uniform sampler2D source;

void main(void)
{
    vec2 uv = qt_TexCoord0.xy;
    vec3 col = texture(source, uv).rgb;
    float grey = dot(col, vec3(0.299, 0.587, 0.114));
    fragColor.a = ubuf.qt_Opacity * 1.0;
    fragColor.rgb = ubuf.qt_Opacity * vec3(grey,grey,grey);
}
