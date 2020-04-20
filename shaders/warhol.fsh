// source: http://kodemongki.blogspot.com/2011/06/kameraku-custom-shader-effects-example.html

uniform sampler2D source;
uniform lowp float qt_Opacity;
varying vec2 qt_TexCoord0;

void main(void)
{
    vec2 uv = qt_TexCoord0.xy;
    vec3 col = texture2D(source, uv).bgr;
    float y = 0.3 *col.r + 0.59 * col.g + 0.11 * col.b;
    y = y < 0.3 ? 0.0 : (y < 0.6 ? 0.5 : 1.0);
    if (y == 0.5)
        col = vec3(0.8, 0.0, 0.0);
    else if (y == 1.0)
        col = vec3(0.9, 0.9, 0.0);
    else
        col = vec3(0.0, 0.0, 0.0);
        
    gl_FragColor.a = qt_Opacity * 1.0;
    gl_FragColor.rgb = qt_Opacity * col;
}

