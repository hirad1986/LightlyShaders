#version 110

uniform sampler2D sampler;

uniform vec2 expanded_size;
uniform vec2 frame_size;
uniform vec3 shadow_size;
uniform int radius;
uniform int shadow_sample_offset;
uniform bool draw_outline;
uniform bool dark_theme;
uniform float outline_strength;
uniform int squircle_ratio;
uniform bool is_squircle;

uniform mat4 modelViewProjectionMatrix;

uniform vec4 modulation;
uniform float saturation;

varying vec2 texcoord0;

//Used code from https://github.com/yilozt/rounded-window-corners project
float squircleBounds(vec2 p, vec2 center, float clip_radius)
{
    vec2 delta = abs(p - center);

    float pow_dx = pow(delta.x, squircle_ratio);
    float pow_dy = pow(delta.y, squircle_ratio);

    float dist = pow(pow_dx + pow_dy, 1.0 / squircle_ratio);

    return clamp(clip_radius - dist + 0.5, 0.0, 1.0);
}

//Used code from https://github.com/yilozt/rounded-window-corners project
float circleBounds(vec2 p, vec2 center, float clip_radius)
{
    vec2 delta = p - vec2(center.x, center.y);
    float dist_squared = dot(delta, delta);

    float outer_radius = clip_radius + 0.5;
    if(dist_squared >= (outer_radius * outer_radius))
        return 0.0;

    float inner_radius = clip_radius - 0.5;
    if(dist_squared <= (inner_radius * inner_radius))
        return 1.0;

    return outer_radius - sqrt(dist_squared);
}

vec4 shapeWindow(vec4 tex, vec2 p, vec2 center, float clip_radius)
{
    float alpha;
    if(is_squircle) {
        alpha = squircleBounds(p, center, clip_radius);
    } else {
        alpha = circleBounds(p, center, clip_radius);
    }
    return vec4(tex.rgb*alpha, min(alpha, tex.a));
}

vec4 shapeShadowWindow(vec2 start, vec4 tex, vec2 p, vec2 center, float clip_radius)
{
    vec2 ShadowHorCoord = vec2(texcoord0.x, start.y);
    vec2 ShadowVerCoord = vec2(start.x, texcoord0.y);

    vec4 texShadowHorCur = texture2D(sampler, ShadowHorCoord);
    vec4 texShadowVerCur = texture2D(sampler, ShadowVerCoord);
    vec4 texShadow0 = texture2D(sampler, start);

    vec4 texShadow = texShadowHorCur + (texShadowVerCur - texShadow0);

    float alpha;
    if(is_squircle) {
        alpha = squircleBounds(p, center, clip_radius);
    } else {
        alpha = circleBounds(p, center, clip_radius);
    }

    if(alpha == 0.0) {
        return texShadow;
    } else if(alpha < 1.0) {
        return mix(vec4(tex.rgb*alpha, min(alpha, tex.a)), texShadow, 1.0-alpha);
    } else {
        return tex;
    }
}

vec4 cornerOutline(vec4 outColor, vec4 outline_color, vec2 coord0, float radius, vec2 center, int invert)
{
    float outline_alpha;
    float outline_alpha_inner;
    float outline_alpha_outer;
    float delta1 = -1.0;
    float delta2 = 1.0;
    if(invert==1) {
        delta1 = 1.0;
    }
    if(is_squircle) {
        outline_alpha_inner = squircleBounds(coord0, vec2(center.x, center.y), radius);
        outline_alpha_outer = squircleBounds(coord0, vec2(center.x+delta1, center.y+delta2), radius);
    } else {
        outline_alpha_inner = circleBounds(coord0, vec2(center.x, center.y), radius);
        outline_alpha_outer = circleBounds(coord0, vec2(center.x+delta1, center.y+delta2), radius);
    }
    outline_alpha = 1.0 - clamp(abs(outline_alpha_outer - outline_alpha_inner), 0.0, 1.0);
    outColor = mix(outColor, vec4(outline_color.rgb,1.0), (1.0-outline_alpha) * outline_color.a);
    return outColor;
}

void main()
{
    vec4 tex = texture2D(sampler, texcoord0);
    vec2 coord0;
    vec4 outColor;
    float start_x;
    float start_y;

    float f_shadow_sample_offset = float(shadow_sample_offset);
    float f_radius = float(radius);

    float dark_outline_strength = outline_strength;
    if(dark_theme) {
        dark_outline_strength = 1.0;
    }

    vec4 outline_color = vec4(1.0,1.0,1.0,outline_strength);
    vec4 dark_outline_color = vec4(0.0,0.0,0.0,dark_outline_strength);

    //Window without shadow
    if(expanded_size == frame_size) {
        coord0 = vec2(texcoord0.x*frame_size.x, texcoord0.y*frame_size.y);
        //Left side
        if (coord0.x < f_radius) {
            //Bottom left corner
            if (coord0.y < f_radius) {
                outColor = shapeWindow(tex, coord0, vec2(f_radius, f_radius), f_radius);

                //Outline
                if(draw_outline) {
                    //Light outline
                    outColor = cornerOutline(outColor, outline_color, coord0, f_radius, vec2(f_radius+1.0, f_radius+1.0), 1);
                    //Dark outline
                    outColor = cornerOutline(outColor, dark_outline_color, coord0, f_radius, vec2(f_radius, f_radius), 1);
                }
            //Top left corner
            } else if (coord0.y > frame_size.y - f_radius) {
                outColor = shapeWindow(tex, coord0, vec2(f_radius, frame_size.y - f_radius), f_radius);

                //Outline
                if(draw_outline) {
                    //Light outline
                    outColor = cornerOutline(outColor, outline_color, coord0, f_radius, vec2(f_radius + 2.0, frame_size.y - f_radius-2.0), 0);
                    //Dark outline
                    outColor = cornerOutline(outColor, dark_outline_color, coord0, f_radius, vec2(f_radius+1.0, frame_size.y - f_radius-1.0), 0);
                }
            //Center
            } else {
                outColor = tex;

                //Outline
                if(coord0.y > f_radius && coord0.y < frame_size.y - f_radius && draw_outline) {
                    if(coord0.x >= 1.0 && coord0.x <= 2.0) {
                        outColor = mix(outColor, vec4(outline_color.rgb,1.0), outline_color.a);
                    }

                    //Dark outline
                    if(
                        (coord0.x >= 0.0 && coord0.x <= 1.0)
                    ) {
                        outColor = mix(outColor, vec4(dark_outline_color.rgb,1.0), dark_outline_color.a);
                    }
                }
            }
        //Right side
        } else if (coord0.x > frame_size.x - f_radius) {
            //Bottom right corner
            if (coord0.y < f_radius) {
                outColor = shapeWindow(tex, coord0, vec2(frame_size.x - f_radius, f_radius), f_radius);

                //Outline
                if(draw_outline) {
                    //Light outline
                    outColor = cornerOutline(outColor, outline_color, coord0, f_radius, vec2(frame_size.x - f_radius-1.0, f_radius+1.0), 0);
                    //Dark outline
                    outColor = cornerOutline(outColor, dark_outline_color, coord0, f_radius, vec2(frame_size.x - f_radius, f_radius), 0);
                }
            //Top right corner
            } else if (coord0.y > frame_size.y - f_radius) {
                outColor = shapeWindow(tex, coord0, vec2(frame_size.x - f_radius, frame_size.y - f_radius), f_radius);

                //Outline
                if(draw_outline) {
                    //Light outline
                    outColor = cornerOutline(outColor, outline_color, coord0, f_radius, vec2(frame_size.x - f_radius-2.0, frame_size.y - f_radius-2.0), 1);
                    //Dark outline
                    outColor = cornerOutline(outColor, dark_outline_color, coord0, f_radius, vec2(frame_size.x - f_radius-1.0, frame_size.y - f_radius-1.0), 1);
                }
            //Center
            } else {
                outColor = tex;

                //Outline
                if(coord0.y > f_radius && coord0.y < frame_size.y - f_radius && draw_outline) {
                    if(coord0.x >= frame_size.x -2.0 && coord0.x <= frame_size.x -1.0) {
                        outColor = mix(outColor, vec4(outline_color.rgb,1.0), outline_color.a);
                    }

                    //Dark outline
                    if(
                        (coord0.x >= frame_size.x -1.0 && coord0.x <= frame_size.x )
                    ) {
                        outColor = mix(outColor, vec4(dark_outline_color.rgb,1.0), dark_outline_color.a);
                    }
                }
            }
        //Center
        } else {
            outColor = tex;

            //Outline
            if(coord0.x > f_radius && coord0.x < frame_size.x - f_radius && draw_outline) {
                if(
                    (coord0.y >= frame_size.y -2.0 && coord0.y <= frame_size.y-1.0 )
                    || (coord0.y >= 1.0 && coord0.y <= 2.0)
                ) {
                    outColor = mix(outColor, vec4(outline_color.rgb,1.0), outline_color.a);
                }

                //Dark outline
                if(
                    (coord0.y >= frame_size.y -1.0  && coord0.y <= frame_size.y)
                    || (coord0.y >= 0.0 && coord0.y <= 1.0)
                ) {
                    outColor = mix(outColor, vec4(dark_outline_color.rgb,1.0), dark_outline_color.a);
                }
            }
        }
    //Window with shadow
    } else if(expanded_size != frame_size) {
        coord0 = vec2(texcoord0.x*expanded_size.x, texcoord0.y*expanded_size.y);
        //Left side
        if (coord0.x > shadow_size.x - f_shadow_sample_offset && coord0.x < f_radius + shadow_size.x) {
            //Top left corner
            if (coord0.y > frame_size.y + shadow_size.z - f_radius && coord0.y < frame_size.y + shadow_size.z + f_shadow_sample_offset) {
                start_x = (shadow_size.x - f_shadow_sample_offset)/expanded_size.x;
                start_y = (shadow_size.z + frame_size.y + f_shadow_sample_offset)/expanded_size.y;
                
                outColor = shapeShadowWindow(vec2(start_x, start_y), tex, coord0, vec2(f_radius + shadow_size.x, frame_size.y + shadow_size.z - f_radius), f_radius);

                //Outline
                if(draw_outline) {
                    //Light outline
                    outColor = cornerOutline(outColor, outline_color, coord0, f_radius, vec2(f_radius + shadow_size.x+1.0, frame_size.y + shadow_size.z - f_radius-1.0), 0);
                    //Dark outline
                    outColor = cornerOutline(outColor, dark_outline_color, coord0, f_radius, vec2(f_radius + shadow_size.x, frame_size.y + shadow_size.z - f_radius), 0);
                }
            //Bottom left corner
            } else if (coord0.y > shadow_size.z - f_shadow_sample_offset && coord0.y < f_radius + shadow_size.z) {
                start_x = (shadow_size.x - f_shadow_sample_offset)/expanded_size.x;
                start_y = (shadow_size.z - f_shadow_sample_offset)/expanded_size.y;

                outColor = shapeShadowWindow(vec2(start_x, start_y), tex, coord0, vec2(f_radius + shadow_size.x, shadow_size.z + f_radius), f_radius);

                //Outline
                if(draw_outline) {
                    //Light outline
                    outColor = cornerOutline(outColor, outline_color, coord0, f_radius, vec2(f_radius + shadow_size.x, shadow_size.z + f_radius), 1);
                    //Dark outline
                    outColor = cornerOutline(outColor, dark_outline_color, coord0, f_radius, vec2(f_radius + shadow_size.x-1.0, shadow_size.z + f_radius-1.0), 1);
                }
            //Center
            } else {
                outColor = tex;

                //Outline and shadow
                if(coord0.y > f_radius + shadow_size.z && coord0.y < shadow_size.z + frame_size.y - f_radius) {
                    //Left shadow padding
                    if(coord0.x > shadow_size.x - f_shadow_sample_offset && coord0.x <= shadow_size.x) {
                        start_x = (shadow_size.x - f_shadow_sample_offset)/expanded_size.x;                        
                        vec4 texShadowEdge = texture2D(sampler, vec2(start_x, texcoord0.y));
                        outColor = texShadowEdge;
                    }

                    if(draw_outline) {
                        //Light outline
                        if(coord0.x >= shadow_size.x && coord0.x <= shadow_size.x+1.0) {
                            outColor = mix(outColor, vec4(outline_color.rgb,1.0), outline_color.a);
                        }

                        //Dark outline
                        if(
                            (coord0.x >= shadow_size.x-1.0 && coord0.x <= shadow_size.x)
                        ) {
                            outColor = mix(outColor, vec4(dark_outline_color.rgb,1.0), dark_outline_color.a);
                        }
                    }
                }
            }
        //Right side
        } else if (coord0.x > shadow_size.x + frame_size.x - f_radius && coord0.x < shadow_size.x + frame_size.x + f_shadow_sample_offset) {
            //Top right corner
            if (coord0.y > frame_size.y + shadow_size.z - f_radius && coord0.y < frame_size.y + shadow_size.z + f_shadow_sample_offset) {
                start_x = (shadow_size.x + frame_size.x + f_shadow_sample_offset)/expanded_size.x;
                start_y = (shadow_size.z + frame_size.y + f_shadow_sample_offset)/expanded_size.y;

                outColor = shapeShadowWindow(vec2(start_x, start_y), tex, coord0, vec2(shadow_size.x + frame_size.x - f_radius, frame_size.y + shadow_size.z - f_radius), f_radius);

                //Outline
                if(draw_outline) {
                    //Light outline
                    outColor = cornerOutline(outColor, outline_color, coord0, f_radius, vec2(shadow_size.x + frame_size.x - f_radius-1.0, frame_size.y + shadow_size.z - f_radius-1.0), 1);
                    //Dark outline
                    outColor = cornerOutline(outColor, dark_outline_color, coord0, f_radius, vec2(shadow_size.x + frame_size.x - f_radius, frame_size.y + shadow_size.z - f_radius), 1);
                }
            //Bottom right corner
            } else if (coord0.y > shadow_size.z - f_shadow_sample_offset && coord0.y < f_radius + shadow_size.z) {
                start_x = (shadow_size.x + frame_size.x + f_shadow_sample_offset)/expanded_size.x;
                start_y = (shadow_size.z - f_shadow_sample_offset)/expanded_size.y;

                outColor = shapeShadowWindow(vec2(start_x, start_y), tex, coord0, vec2(shadow_size.x + frame_size.x - f_radius, shadow_size.z + f_radius), f_radius);

                //Outline
                if(draw_outline) {
                    //Light outline
                    outColor = cornerOutline(outColor, outline_color, coord0, f_radius, vec2(shadow_size.x + frame_size.x - f_radius, shadow_size.z + f_radius), 0);
                    //Dark outline
                    outColor = cornerOutline(outColor, dark_outline_color, coord0, f_radius, vec2(shadow_size.x + frame_size.x - f_radius + 1.0, shadow_size.z + f_radius - 1.0), 0);
                }
            //Center
            } else {
                outColor = tex;

                //Outline and shadow
                if(coord0.y > f_radius + shadow_size.z && coord0.y < shadow_size.z + frame_size.y - f_radius) {
                    //Right shadow padding
                    if(coord0.x >= shadow_size.x + frame_size.x && coord0.x < shadow_size.x + frame_size.x + f_shadow_sample_offset) {
                        start_x = (shadow_size.x + frame_size.x + f_shadow_sample_offset)/expanded_size.x;
                        vec4 texShadowEdge = texture2D(sampler, vec2(start_x, texcoord0.y));
                        outColor = texShadowEdge;
                    }

                    if(draw_outline) {
                        //Light outline
                        if(coord0.x >= frame_size.x + shadow_size.x-1.0 && coord0.x <= frame_size.x + shadow_size.x) {
                            outColor = mix(outColor, vec4(outline_color.rgb,1.0), outline_color.a);
                        }

                        //Dark outline
                        if(
                            (coord0.x >= frame_size.x + shadow_size.x && coord0.x <= frame_size.x + shadow_size.x+1.0)
                        ) {
                            outColor = mix(outColor, vec4(dark_outline_color.rgb,1.0), dark_outline_color.a);
                        }
                    }
                }
            }
        //Center
        } else {
            outColor = tex;

            //Outline and shadow
            if(coord0.x > f_radius + shadow_size.x && coord0.x < shadow_size.x + frame_size.x - f_radius) {
                //Top shadow padding
                if(coord0.y >= frame_size.y + shadow_size.z && coord0.y < frame_size.y + shadow_size.z + f_shadow_sample_offset) {
                    start_y = (shadow_size.z + frame_size.y + f_shadow_sample_offset)/expanded_size.y;
                    vec4 texShadowEdge = texture2D(sampler, vec2(texcoord0.x, start_y));
                    outColor = texShadowEdge;
                //Bottom shadow padding
                } else if(coord0.y <= shadow_size.z && coord0.y > shadow_size.z - f_shadow_sample_offset) {
                    start_y = (shadow_size.z - f_shadow_sample_offset)/expanded_size.y;
                    vec4 texShadowEdge = texture2D(sampler, vec2(texcoord0.x, start_y));
                    outColor = texShadowEdge;
                }

                if(draw_outline) {
                    //Light outline
                    if(
                        (coord0.y >= frame_size.y + shadow_size.z-1.0 && coord0.y <= frame_size.y + shadow_size.z)
                        || (coord0.y >= shadow_size.z && coord0.y <= shadow_size.z+1.0)
                    ) {
                        outColor = mix(outColor, vec4(outline_color.rgb,1.0), outline_color.a);
                    }

                    //Dark outline
                    if(
                        (coord0.y >= frame_size.y + shadow_size.z && coord0.y <= frame_size.y + shadow_size.z + 1.0)
                        || (coord0.y >= shadow_size.z-1.0 && coord0.y <= shadow_size.z)
                    ) {
                        outColor = mix(outColor, vec4(dark_outline_color.rgb,1.0), dark_outline_color.a);
                    }
                }
            }
        }
    }


    //Support opacity
    if (saturation != 1.0) {
        vec3 desaturated = outColor.rgb * vec3( 0.30, 0.59, 0.11 );
        desaturated = vec3( dot( desaturated, outColor.rgb ));
        outColor.rgb = outColor.rgb * vec3( saturation ) + desaturated * vec3( 1.0 - saturation );
    }
    outColor *= modulation;

    //Output result
    gl_FragColor = outColor;
    //gl_FragColor = vec4(tex.r, tex.g, 1.0, tex.a);
}
