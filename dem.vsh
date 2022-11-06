attribute highp vec4 aPosition;
attribute lowp vec4 aColor;
attribute mediump vec2 aTexCoord;
varying lowp vec4 vColor;
varying mediump vec2 vTexCoord;
uniform highp mat4 uMatrix;
uniform bool uEnableTex;

void main(){
    vColor = aColor;
    vTexCoord = aTexCoord;
    gl_Position = uMatrix * aPosition;
}
