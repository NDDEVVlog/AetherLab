#version 330 core
out vec4 FragColor;

#include "lights.glsl"
#include "Material.glsl"

in vec3 FragPos;  
in vec3 Normal;  
in vec2 vTexCoord;

uniform Material material;

vec3 GetPerturbedNormal(vec3 fragPos, vec2 texCoord, vec3 vertexNormal) {
    if (!material.hasNormalMap) {
        return normalize(vertexNormal);
    }
    
    vec3 nMap = texture(material.normalMap, texCoord).rgb * 2.0 - 1.0; 
    
    vec3 pos_dx = dFdx(fragPos);
    vec3 pos_dy = dFdy(fragPos);
    vec2 tex_dx = dFdx(texCoord);
    vec2 tex_dy = dFdy(texCoord);
    
    vec3 t = (tex_dy.t * pos_dx - tex_dx.t * pos_dy) / (tex_dx.s * tex_dy.t - tex_dy.s * tex_dx.t);
    vec3 ng = normalize(vertexNormal);
    t = normalize(t - ng * dot(ng, t));
    vec3 b = normalize(cross(ng, t));
    mat3 tbn = mat3(t, b, ng);
    
    return normalize(tbn * nMap);
}

vec3 CalculateLightContribution(URPLight light, vec3 normal, vec3 viewDir, vec3 albedo, vec3 specMap) {
    vec3 lightDir = light.direction;
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    vec3 diffuse = light.color * diff * albedo;
    vec3 specular = light.color * spec * specMap;
    
    return (diffuse + specular) * light.distanceAttenuation * light.shadowAttenuation;
}

void main()
{   
    vec3 albedo = material.hasAlbedoMap ? texture(material.albedoMap, vTexCoord).rgb : material.baseAlbedo;
    vec3 specMap = material.hasSpecularMap ? texture(material.specularMap, vTexCoord).rgb : material.baseSpecular;
    
    vec3 norm = GetPerturbedNormal(FragPos, vTexCoord, Normal);
    vec3 viewDir = normalize(GetCameraPositionWS() - FragPos);
    
    vec3 result = u_AmbientColor * albedo;

    URPLight mainLgt = GetMainLight();
    result += CalculateLightContribution(mainLgt, norm, viewDir, albedo, specMap);

    int lightCount = GetAdditionalLightsCount();
    for (uint i = 0u; i < uint(lightCount); ++i) {
        URPLight addLight = GetAdditionalLight(i, FragPos);
        result += CalculateLightContribution(addLight, norm, viewDir, albedo, specMap);
    }
    
    FragColor = vec4(result, 1.0);
}