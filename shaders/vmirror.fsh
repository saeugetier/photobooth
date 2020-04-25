uniform sampler2D source;
uniform lowp float qt_Opacity;
varying vec2 qt_TexCoord0;

void main(void)
{
    vec2 uv = vec2(0.0, 0.0);
    uv.x = 1.0 - qt_TexCoord0.x;
    uv.y = qt_TexCoord0.y;
    vec3 col = texture2D(source, uv).rgb;
    gl_FragColor.a = qt_Opacity * 1.0;
    gl_FragColor.rgb = qt_Opacity * col;
}

