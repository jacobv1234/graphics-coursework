#version 400

in  vec2 ex_TexCoord; //texture coord arriving from the vertex
in  vec3 ex_Normal;  //normal arriving from the vertex

out vec4 out_Color;   //colour for the pixel
in vec3 ex_LightDir;  //light direction arriving from the vertex
in vec3 ex_SpotlightDir;
in vec3 ex_CupDir;

in float ex_y;

in vec3 ex_PositionEye;

uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

uniform vec4 spotlight_ambient;
uniform vec4 spotlight_diffuse;
uniform vec4 spotlight_specular;

uniform vec4 cup_ambient;
uniform vec4 cup_diffuse;
uniform vec4 cup_specular;

uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform float material_shininess;

uniform sampler2D DiffuseMap;

void main(void)
{
	//out_Color = texture(DiffuseMap, ex_TexCoord); //show texture values

	//out_Color = vec4(ex_Normal,1.0); //Show normals

	//out_Color = vec4(ex_TexCoord,0.0,1.0); //show texture coords

	// spotlight distance calculation
	float spot_distance = pow(ex_SpotlightDir.x,2) + pow(ex_SpotlightDir.y,2) + pow(ex_SpotlightDir.z,2); // Pythagoras
	float brightness_mult = 1000 / spot_distance; // Inverse square law
	vec4 eff_spot_ambient = spotlight_ambient * brightness_mult;
	vec4 eff_spot_diffuse = spotlight_diffuse * brightness_mult;
	vec4 eff_spot_specular = spotlight_specular * brightness_mult / 100; // apply inverse square law but also dim the specular just in general


	// cup distance calculation
	
	float cupMagnitude = pow(ex_CupDir.x,2) + pow(ex_CupDir.y,2) + pow(ex_CupDir.z,2); // Pythagoras
	float brightness_mult_cup = 1;
	if (cupMagnitude > 50 || ex_y > 0.21f) {
		brightness_mult_cup = 0;
	}
	vec4 eff_cup_ambient = cup_ambient * brightness_mult_cup;
	vec4 eff_cup_diffuse = cup_diffuse * brightness_mult_cup;
	vec4 eff_cup_specular = cup_specular * brightness_mult_cup / 100; // apply inverse square law but also dim the specular just in general


	//Calculate lighting
	vec3 n, L, sL, cL;
	vec4 color;
	float NdotL, sNdotL, cNdotL;
	
	n = normalize(ex_Normal);
	L = normalize(ex_LightDir);
	sL = normalize(ex_SpotlightDir);
	cL = normalize(ex_CupDir);

	vec3 v = normalize(-ex_PositionEye);
	vec3 r = normalize(-reflect(L, n));
	vec3 sr = normalize(-reflect(sL, n));
	vec3 cr = normalize(-reflect(cL, n));
	
	float RdotV = max(0.0, dot(r, v));
	float sRdotV = max(0.0, dot(sr, v));
	float cRdotV = max(0.0, dot(cr, v));

	NdotL = max(dot(n, L),0.0);
	sNdotL = max(dot(n, sL),0.0);
	cNdotL = max(dot(-n, cL),0.0f);

	color = (light_ambient + eff_spot_ambient + eff_cup_ambient) * material_ambient;
	
	if(NdotL > 0.0) 
	{
		color += (light_diffuse * material_diffuse * NdotL);
	}

	if(sNdotL > 0.0) 
	{
		color += (eff_spot_diffuse * material_diffuse * sNdotL);
	}

	if(cNdotL > 0.0) 
	{
		color += (eff_cup_diffuse * material_diffuse * cNdotL);
	}

	color += material_specular * light_specular * pow(RdotV, material_shininess);
	color += material_specular * eff_spot_specular * pow(sRdotV, material_shininess);
	color += material_specular * eff_cup_specular * pow(cRdotV, material_shininess);

	//out_Color = color;  //show just lighting

	//out_Color = texture(DiffuseMap, ex_TexCoord); //show texture only
    out_Color = color * texture(DiffuseMap, ex_TexCoord); //show texture and lighting
}