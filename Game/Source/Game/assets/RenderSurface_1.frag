#version 150

in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D texFramebuffer;
uniform float time = 0;

void main() {
    outColor = vec4(texture(texFramebuffer, Texcoord));
    outColor.rgb += vec3(Texcoord.y-0.25 + (sin(time+Texcoord.x*5)+1)*0.1)*0.5f;
}
