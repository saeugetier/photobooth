#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
} ubuf;
layout(binding = 1) uniform sampler2D source;
void main() {
    vec2 uv = vec2(0.0, 0.0);
    uv.x = 1.0 - qt_TexCoord0.x;
    uv.y = qt_TexCoord0.y;
    vec3 col = texture(source, uv).rgb;
    fragColor.a = ubuf.qt_Opacity * 1.0;
    fragColor.rgb = ubuf.qt_Opacity * col;
}
