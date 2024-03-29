#version 450 core
out vec4 FragColor;

in vec3 outColor;
in vec2 TexCoord;
uniform sampler2D ourTex;
void main()
{
   FragColor = texture(ourTex, TexCoord) * vec4(outColor, 1.0);
}

