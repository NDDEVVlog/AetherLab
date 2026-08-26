#version 330 core
out vec4 FragColor;

#include "lights.glsl"

in vec3 FragPos;  
in vec3 Normal;  
in vec2 vTexCoord;
in vec4 FragPosLightSpace;

// Thay vì tạo Material phức tạp, dùng chung dữ liệu cơ bản
struct Material {
    sampler2D albedoMap;
    bool hasAlbedoMap;
    vec3 baseAlbedo;
    float shininess;
};

uniform Material material;
uniform sampler2D shadowMap;


const float rampThreshold = 0.5;      
const float rampSmoothness = 0.05;    
const float rimThreshold = 0.716;     
const float shadowThreshold = 0.5;    
const vec3 rimColor = vec3(1.0, 1.0, 1.0); 


float CalculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0) return 0.0;
    
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    // PCF mềm mại, nhưng với Toon ta sẽ bóp nó lại bằng smoothstep ở hàm dưới
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}

vec3 CalculateToonLight(URPLight light, vec3 normal, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = light.direction;
    

    float rawAttenuation = light.distanceAttenuation * light.shadowAttenuation;
    float toonShadow = smoothstep(shadowThreshold - 0.1, shadowThreshold + 0.1, rawAttenuation);

    float NdotL = max(dot(normal, lightDir), 0.0);
    float lightIntensity = NdotL * toonShadow;
    float diffuseIntensity = smoothstep(rampThreshold, rampThreshold + rampSmoothness, lightIntensity);
    

    vec3 toonLightColor = light.color * diffuseIntensity;
    vec3 diffuse = (albedo * toonLightColor) + (toonLightColor * 0.15);


    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float NdotH = max(dot(normal, halfwayDir), 0.0);
    float specPower = pow(NdotH, material.shininess);
    float specIntensity = smoothstep(0.005, 0.01, specPower) * toonShadow;
    vec3 specular = light.color * specIntensity;


    float fresnel = 1.0 - max(dot(normal, viewDir), 0.0);
    fresnel *= max(NdotL, 0.1); 
    float rimIntensity = smoothstep(rimThreshold - 0.01, rimThreshold + 0.01, fresnel) * toonShadow;
    vec3 rim = rimIntensity * rimColor * light.color;

    return diffuse + specular + rim;
}

void main()
{   
    vec3 albedo = material.hasAlbedoMap ? texture(material.albedoMap, vTexCoord).rgb : material.baseAlbedo;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(GetCameraPositionWS() - FragPos);
    

    vec3 result = u_AmbientColor * albedo;


    URPLight mainLgt = GetMainLight();
    if (mainLight.castShadows) {
        mainLgt.shadowAttenuation = 1.0 - CalculateShadow(FragPosLightSpace, norm, mainLgt.direction);
    }
    result += CalculateToonLight(mainLgt, norm, viewDir, albedo);


    int lightCount = GetAdditionalLightsCount();
    for (uint i = 0u; i < uint(lightCount); ++i) {
        URPLight addLight = GetAdditionalLight(i, FragPos);
        result += CalculateToonLight(addLight, norm, viewDir, albedo);
    }
    
    FragColor = vec4(result, 1.0);
}