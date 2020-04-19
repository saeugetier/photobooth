// source: http://kodemongki.blogspot.com/2011/06/kameraku-custom-shader-effects-example.html

precision mediump float;
uniform sampler2D vTex;
varying vec2 vCoord;

void main(void)
{
    vec3 col = texture2D(vTex, vCoord).bgr;
    float y = 0.3 *col.r + 0.59 * col.g + 0.11 * col.b;
    y = y < 0.5 ? 0.0 : 1.0;
    gl_FragColor = vec4(y, y, y, 1.0) ;
}

