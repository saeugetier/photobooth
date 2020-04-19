// source: http://kodemongki.blogspot.com/2011/06/kameraku-custom-shader-effects-example.html

precision mediump float;

uniform sampler2D vTex;
varying vec2 vCoord;

void main(void)
{
    vec3 col = texture2D(vTex, vCoord).bgr;
    vec2 p = vCoord;
    float cr = pow(0.1, 2.0);
    float pt = pow(p.x - 0.5, 2.0) + pow(p.y - 0.5, 2.0);
    float d = pt - cr;
    float cf = 1.0;
    if (d > 0.0)
        cf = 1.0 - 2.0 * d;
    gl_FragColor.a = 1.0;
    gl_FragColor.rgb = cf * col;
}
