#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform LightBufferObject{
    vec4 lightPos;
    vec4 ambientColor;
} light;
layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float lightPower = 400.0;
//const vec3 ambientColor = vec3(0.1, 0.1, 0.1);
//const vec3 diffuseColor = vec3(0.5, 0.0, 0.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);
const float shininess = 128.0;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = light.lightPos.xyz - fragPos;

    float distance = length(lightDir);
      distance = distance * distance;
      lightDir = normalize(lightDir);
      float lambertian = max(dot(lightDir,normal), 0.0);
        float specular = 0.0;

        if(lambertian > 0.0) {

          vec3 viewDir = normalize(-fragPos);

          // this is blinn phong
          vec3 halfDir = normalize(lightDir + viewDir);
          float specAngle = max(dot(halfDir, normal), 0.0);
          specular = pow(specAngle, 10);

          // this is phong (for comparison)
//          if(mode == 2) {
//            vec3 reflectDir = reflect(-lightDir, normal);
//            specAngle = max(dot(reflectDir, viewDir), 0.0);
//            // note that the exponent is different here
//            specular = pow(specAngle, 10/4.0);
//          }
        }
        vec3 diffuseColor = texture(texSampler, fragTexCoord).xyz;
        vec3 colorLinear = light.ambientColor.xyz +
                           diffuseColor * lambertian * lightColor * lightPower / distance +
                           specColor * specular * lightColor * lightPower / distance;
        // apply gamma correction (assume ambientColor, diffuseColor and specColor
        // have been linearized, i.e. have no gamma correction in them)
//        vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0/screenGamma));
        // use the gamma corrected color in the fragment


        outColor = vec4(colorLinear,1.0f);
//    outColor = light.ambientColor * texture(texSampler, fragTexCoord);
}
