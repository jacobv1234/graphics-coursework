#version 400

uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

in  vec3 in_Position;  // Position coming in
in  vec2 in_TexCoord;  // texture coordinate coming in
in  vec3 in_Normal;    // vertex normal used for lighting

uniform vec4 LightPos;  // light position
uniform vec4 SpotlightCoord; // spotlight coord
uniform vec4 CupPos;

out vec2 ex_TexCoord;  // exiting texture coord
out vec3 ex_Normal;    // exiting normal transformed by the normal matrix
out vec3 ex_PositionEye; 
out vec3 ex_LightDir;
out vec3 ex_SpotlightDir;
out vec3 ex_CupDir;
out float ex_y;

void main(void)
{
	gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(in_Position, 1.0);
	
	ex_TexCoord = in_TexCoord;
		
	ex_Normal = NormalMatrix*in_Normal; 
	//ex_Normal = in_Normal; 

	ex_PositionEye = vec3((ModelViewMatrix * vec4(in_Position, 1.0)));

	ex_LightDir = vec3(ViewMatrix * LightPos);

	vec4 SpotDir = SpotlightCoord - vec4(in_Position, 1.0);

	//ex_SpotlightDir = vec3(ViewMatrix * SpotDir); // tied to camera location

	ex_SpotlightDir = vec3(SpotDir);

	vec4 CupDir = (ModelViewMatrix * CupPos - ModelViewMatrix * vec4(in_Position, 1.0));
	//vec4 CupDir = CupPos - ModelMatrix * vec4(in_Position, 1.0); // convert from local model space to world space

	ex_CupDir = vec3(CupDir);

	ex_y = in_Position.y;
}