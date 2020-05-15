uniform sampler2D source;
uniform lowp float qt_Opacity;
varying vec2 qt_TexCoord0;

void main(void)
{
    vec2 uv = qt_TexCoord0.xy;
    vec3 col = texture2D(source, uv).rgb;
    float grey = dot(col, vec3(0.299, 0.587, 0.114));
    gl_FragColor.a = qt_Opacity * 1.0;
    gl_FragColor.rgb = qt_Opacity * vec3(grey,grey,grey);
}
