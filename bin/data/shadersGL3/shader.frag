#version 410



flat in vec4 hexColor;
//in vec4 hexColor;
uniform sampler2D uTexture;
out vec4 out_color;
// inputs
in vec2 out_vTexCoord;

in vec4 out_vertexColor;
in vec4 out_colorMix;
in vec4 out_colorA;

uniform int u_useCubeColors;
uniform int u_modulo;
uniform vec4 u_colorA;
uniform vec4 u_colorB;
uniform int u_textureSource;
uniform int u_source;



//enum sourceTextureType
//{
//    HEX_TEXTURE_IMAGE = 0,
//    HEX_TEXTURE_VIDEO = 1,
//    HEX_TEXTURE_SYPHON = 2,
//    HEX_TEXTURE_SYPHON_MAX = 3,
//    HEX_TEXTURE_CUBES = 4,
//    HEX_TEXTURE_COLOR = 5
//};
//enum sourceType
//{
//    HEX_SOURCE_TEXTURE = 0,
//    HEX_SOURCE_QUADS = 1,
//    HEX_SOURCE_CUCS = 2,
//    HEX_SOURCE_RANDOM = 3
//};

void main()
{
    if(u_source==0)
    {
        if((u_textureSource==0)||(u_textureSource==1)||(u_textureSource==2)||(u_textureSource==3))
        {
            out_color = texture(uTexture, out_vTexCoord);
            
        }
        else if (u_textureSource==4)
        {
            out_color = vec4( hexColor.rgb, 1.0 );
        }
        else if (u_textureSource==5)
        {
            out_color = out_colorMix;
        }
        
    }
    else if(u_source==1)
    {
        out_color = vec4(1.0,0.0,0.5,1.0);
        out_color = out_colorA;
    }
    else if(u_source==2)
    {
        out_color = vec4(1.0,0.5,0.0,1.0);
        out_color = out_colorA;;
    }
    else if(u_source==3)
    {
        //out_color = vec4(0.0,0.5,1.0,1.0);
        //out_color = out_colorA;
        out_color = out_vertexColor;
    }
    

    
//    else if(u_modulo==4) out_color = vec4(1.0,1.0,1.0,1.0);
//    else if(u_modulo==96) out_color = vec4(1.0,1.0,1.0,1.0);
//    else out_color = vec4(1.0,0.0,1.0,1.0);
//    out_color = texelFetch(uTexture,vTexCoord,0);
//    out_color = texelFetch(uTexture, ivec2(0.3,0.3),0);
//    out_color = texture(uTexture, ivec2(0.3,0.3));
//    out_color = texture2D(uTexture, vTexCoord) + 0.25;
  
    
}
