// source: http://kodemongki.blogspot.com/2011/06/kameraku-custom-shader-effects-example.html
// modified by: saeugetier

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
    vec3 col = texture(source, uv).bgr;
    float y = 0.3 *col.r + 0.59 * col.g + 0.11 * col.b;
    fragColor = ubuf.qt_Opacity * vec4(y + 0.15, y + 0.07, y - 0.12, 1.0) ;
}

