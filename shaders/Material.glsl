
struct Material {
    sampler2D albedoMap;
    sampler2D specularMap;
    sampler2D normalMap;

    bool hasAlbedoMap;
    bool hasSpecularMap;
    bool hasNormalMap;

    vec3 baseAlbedo;   
    vec3 baseSpecular; 
    vec3 baseAmbient;

    float shininess;
};