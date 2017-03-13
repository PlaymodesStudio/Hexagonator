#version 410

flat in vec4 hexColor;
//in vec4 hexColor;
uniform sampler2D uTexture;
out vec4 out_color;
// inputs
in vec2 vTexCoord;
uniform int u_useCubeColors;
uniform int u_modulo;



void main()
{
    if(u_modulo==7)
    {
        if (u_useCubeColors==1) out_color = vec4( hexColor.rgb, 1.0 );
        else out_color = texture(uTexture, vTexCoord);
    }
    else if(u_modulo==4) out_color = vec4(1.0,1.0,1.0,1.0);
    else if(u_modulo==16) out_color = vec4(1.0,1.0,0.0,1.0);
    else out_color = vec4(1.0,0.0,1.0,1.0);
//    out_color = texelFetch(uTexture,vTexCoord,0);
//    out_color = texelFetch(uTexture, ivec2(0.3,0.3),0);
//    out_color = texture(uTexture, ivec2(0.3,0.3));
    //    out_color = texture2D(uTexture, vTexCoord) + 0.25;
  
    
}
