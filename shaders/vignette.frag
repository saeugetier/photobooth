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
    vec2 p = uv;
    float cr = pow(0.1, 2.0);
    float pt = pow(p.x - 0.5, 2.0) + pow(p.y - 0.5, 2.0);
    float d = pt - cr;
    float cf = 1.0;
    if (d > 0.0)
        cf = 1.0 - 2.0 * d;
    fragColor.a = ubuf.qt_Opacity * 1.0;
    fragColor.rgb = ubuf.qt_Opacity * cf * col;
}
