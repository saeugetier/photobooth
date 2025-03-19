#version 440
layout(location = 0) in vec2 coord;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
};
layout(binding = 1) uniform sampler2D src;
void main() {
    vec2 uv = vec2(0.0, 0.0);
    uv.x = 1.0 - coord.x;
    uv.y = coord.y;
    vec3 col = texture(src, uv).rgb;
    fragColor.a = qt_Opacity * 1.0;
    fragColor.rgb = qt_Opacity * col;
}
