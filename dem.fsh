varying lowp vec4 vColor;
varying mediump vec2 vTexCoord;
uniform sampler2D uSampler;
uniform bool uEnableTex;

void main(void)
{
    if(uEnableTex) {
        gl_FragColor = texture2D(uSampler, vTexCoord);
    } else {
        gl_FragColor = vColor;
    }
}
