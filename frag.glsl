#version 450 core

in vec3 OutColor;
in vec2 TexCoord;
uniform sampler2D OurTex;

out vec4 FragColor;

void main()
{
   FragColor = texture(OurTex, TexCoord) * vec4(OutColor, 1.0);
}

