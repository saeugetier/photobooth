// source: http://kodemongki.blogspot.com/2011/06/kameraku-custom-shader-effects-example.html

uniform sampler2D source;
uniform lowp float qt_Opacity;
varying vec2 qt_TexCoord0;

void main(void)
{
    vec2 uv = qt_TexCoord0.xy;
    vec3 col = texture2D(source, uv).bgr;
    float y = 0.3 *col.r + 0.59 * col.g + 0.11 * col.b;
    gl_FragColor = qt_Opacity * vec4(y + 0.15, y + 0.07, y - 0.12, 1.0) ;
}

