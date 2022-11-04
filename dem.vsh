precision mediump float;
attribute highp vec4 aPosition;
attribute highp vec4 aColor;
varying highp vec4 vColor;
uniform highp mat4 uMatrix;

void main(){
    vColor = aColor;
    gl_Position = uMatrix * aPosition;
}
