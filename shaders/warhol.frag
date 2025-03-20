// source: http://kodemongki.blogspot.com/2011/06/kameraku-custom-shader-effects-example.html
// modified: saeugetier

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
    y = y < 0.3 ? 0.0 : (y < 0.6 ? 0.5 : 1.0);
    if (y == 0.5)
        col = vec3(0.8, 0.0, 0.0);
    else if (y == 1.0)
        col = vec3(0.9, 0.9, 0.0);
    else
        col = vec3(0.0, 0.0, 0.0);
        
    fragColor.a = ubuf.qt_Opacity * 1.0;
    fragColor.rgb = ubuf.qt_Opacity * col;
}

