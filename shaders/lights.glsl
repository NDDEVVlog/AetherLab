#define MAX_ADDITIONAL_LIGHTS 16

struct URPLight {
    vec3 direction;
    vec3 color;
    float distanceAttenuation;
    float shadowAttenuation;
};

struct MainLightData {
    vec3 direction;
    vec3 color;
};

struct AdditionalLightData {
    int type;
    vec3 position;
    vec3 direction;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

uniform MainLightData mainLight;
uniform int u_AdditionalLightsCount;
uniform vec3 u_CameraPositionWS;
uniform AdditionalLightData u_AdditionalLights[MAX_ADDITIONAL_LIGHTS];
uniform vec3 u_AmbientColor;

int GetAdditionalLightsCount() {
    return u_AdditionalLightsCount;
}

vec3 GetCameraPositionWS() {
    return u_CameraPositionWS;
}

URPLight GetMainLight() {
    URPLight light;
    light.direction = normalize(-mainLight.direction);
    light.color = mainLight.color;
    light.distanceAttenuation = 1.0;
    light.shadowAttenuation = 1.0; 
    return light;
}

URPLight GetAdditionalLight(uint i, vec3 positionWS) {
    URPLight light;
    AdditionalLightData data = u_AdditionalLights[i];

    vec3 lightVector = data.position - positionWS;
    float distance = length(lightVector);
    light.direction = normalize(lightVector);

    float attenuation = 1.0 / (data.constant + data.linear * distance + data.quadratic * (distance * distance));


    // Spot Light
    if (data.type == 1) { 
        float theta = dot(light.direction, normalize(-data.direction));
        float epsilon = data.cutOff - data.outerCutOff;
        float spotIntensity = clamp((theta - data.outerCutOff) / epsilon, 0.0, 1.0);
        attenuation *= spotIntensity;
    }

    light.color = data.color;
    light.distanceAttenuation = attenuation;
    light.shadowAttenuation = 1.0; 

    return light;
}