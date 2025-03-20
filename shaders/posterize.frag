#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
} ubuf;
layout(binding = 1) uniform sampler2D source;

const float gamma = 0.6;
const float numColors = 8.0;

void main()
{
  vec3 c = texture(source, qt_TexCoord0.xy).rgb;
  c = pow(c, vec3(gamma, gamma, gamma));
  c = c * numColors;
  c = floor(c);
  c = c / numColors;
  c = pow(c, vec3(1.0/gamma));
  fragColor = vec4(c, 1.0) * ubuf.qt_Opacity;
}
