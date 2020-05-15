uniform sampler2D source;
uniform lowp float qt_Opacity;
varying vec2 qt_TexCoord0;

const float gamma = 0.6;
const float numColors = 8.0;

void main()
{
  vec3 c = texture2D(source, qt_TexCoord0.xy).rgb;
  c = pow(c, vec3(gamma, gamma, gamma));
  c = c * numColors;
  c = floor(c);
  c = c / numColors;
  c = pow(c, vec3(1.0/gamma));
  gl_FragColor = vec4(c, 1.0);
}
