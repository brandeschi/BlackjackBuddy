#version 450 core

in vec3 OutColor;
in vec2 TexCoord;
in float TexId;
uniform sampler2D CardTex;
uniform sampler2D FontTex;

out vec4 FragColor;

void main()
{
  // TODO: Utilize an array for samplers
  if (TexId == 0.0)
  {
   FragColor = texture(CardTex, TexCoord) * vec4(OutColor, 1.0);
  }
  else
  {
   FragColor = texture(FontTex, TexCoord) * vec4(OutColor, 1.0);
  }
}

