// source: http://kodemongki.blogspot.com/2011/06/kameraku-custom-shader-effects-example.html

uniform sampler2D source;
uniform lowp float qt_Opacity;
varying vec2 qt_TexCoord0;

void main(void)
{
    vec2 uv = qt_TexCoord0.xy;
    vec3 col = texture2D(source, uv).bgr;
    vec2 p = uv;
    float cr = pow(0.1, 2.0);
    float pt = pow(p.x - 0.5, 2.0) + pow(p.y - 0.5, 2.0);
    float d = pt - cr;
    float cf = 1.0;
    if (d > 0.0)
        cf = 1.0 - 2.0 * d;
    gl_FragColor.a = qt_Opacity * 1.0;
    gl_FragColor.rgb = qt_Opacity * cf * col;
}
