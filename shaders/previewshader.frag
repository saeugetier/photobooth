#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec4 contentRect; // x, y, width, height of the content rect
    int mirrored;
    int enableMask;
} ubuf;

layout(binding = 1) uniform sampler2D source;
layout(binding = 2) uniform sampler2D bgSource;

void main() {
    vec2 uv = vec2(0.0, 0.0);
    vec2 bgUV = vec2(0.0, 0.0);

    if(ubuf.mirrored != 0)
    {
        uv.x = 1.0 - qt_TexCoord0.x;
    }
    else
    {
        uv.x = qt_TexCoord0.x;
    }
    uv.y = qt_TexCoord0.y;

    vec4 col = texture(source, uv);

    if(ubuf.enableMask != 0)
    {
        vec2 videoPosition = vec2(ubuf.contentRect.x, ubuf.contentRect.y);
        vec2 videoSize = vec2(ubuf.contentRect.z, ubuf.contentRect.w);

        bgUV = (uv - videoPosition) / videoSize;

        if (bgUV.x < 0.0 || bgUV.x > 1.0 || bgUV.y < 0.0 || bgUV.y > 1.0) {
                discard; // Discard fragments outside the 0-1 range
            }

        vec3 bg = texture(bgSource, bgUV).rgb;
        col = vec4(mix(bg, col.rgb, col.a), 1.9); // use the red channel as mask
    }
    fragColor.a = ubuf.qt_Opacity * 1.0;
    fragColor.rgb = ubuf.qt_Opacity * col.rgb;
}
