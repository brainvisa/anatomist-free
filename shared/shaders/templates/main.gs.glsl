#define MAX_TEXTURE_UNITS 8

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

in VertexData
{
  vec4 v_color;
  vec3 v_normal;
  vec3 v_texcoord[MAX_TEXTURE_UNITS];
  vec4 v_eyeVertexPosition;
  vec3 v_directionLight;
} gs_in[3];


out VertexData
{
  vec4 v_color;
  vec3 v_normal;
  vec3 v_texcoord[MAX_TEXTURE_UNITS];
  vec4 v_eyeVertexPosition;
  vec3 v_directionLight;
};

out float gl_ClipDistance[]; 


uniform int u_paletteTexUnit;

void emitFull(int s, float zoneVal)
{
  gl_Position         = gl_in[s].gl_Position;
  gl_ClipDistance[0]  = gl_in[s].gl_ClipDistance[0];
  gl_ClipDistance[1]  = gl_in[s].gl_ClipDistance[1];
  gl_ClipDistance[2]  = gl_in[s].gl_ClipDistance[2];
  v_color             = gs_in[s].v_color;
  v_normal            = gs_in[s].v_normal;
  v_eyeVertexPosition = gs_in[s].v_eyeVertexPosition;
  v_directionLight    = gs_in[s].v_directionLight;
  for(int t = 0; t < MAX_TEXTURE_UNITS; ++t)
      v_texcoord[t] = (t == u_paletteTexUnit)
                      ? vec3(zoneVal, 0.0, 0.0)
                      : gs_in[s].v_texcoord[t];
  EmitVertex();
}

void emitMid(int i, int j, float zoneVal)
{
  gl_Position = 0.5 * (gl_in[i].gl_Position + gl_in[j].gl_Position);
  gl_ClipDistance[0]  = 0.5 * (gl_in[i].gl_ClipDistance[0] + gl_in[j].gl_ClipDistance[0]);
  gl_ClipDistance[1]  = 0.5 * (gl_in[i].gl_ClipDistance[1] + gl_in[j].gl_ClipDistance[1]);
  gl_ClipDistance[2]  = 0.5 * (gl_in[i].gl_ClipDistance[2] + gl_in[j].gl_ClipDistance[2]);
  v_color = 0.5 * (gs_in[i].v_color + gs_in[j].v_color);
  v_normal = normalize(gs_in[i].v_normal + gs_in[j].v_normal);
  v_eyeVertexPosition = 0.5 * (gs_in[i].v_eyeVertexPosition + gs_in[j].v_eyeVertexPosition);
  v_directionLight = gs_in[0].v_directionLight;
  for(int t = 0; t < MAX_TEXTURE_UNITS; ++t)
      v_texcoord[t] = (t == u_paletteTexUnit)
                      ? vec3(zoneVal, 0.0, 0.0)
                      : 0.5 * (gs_in[i].v_texcoord[t] + gs_in[j].v_texcoord[t]);
  EmitVertex();
}

void emitCenter(float zoneVal)
{
  gl_Position = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3.0;
  gl_ClipDistance[0]  = (gl_in[0].gl_ClipDistance[0] + gl_in[1].gl_ClipDistance[0] + gl_in[2].gl_ClipDistance[0]) / 3.0;
  gl_ClipDistance[1]  = (gl_in[0].gl_ClipDistance[1] + gl_in[1].gl_ClipDistance[1] + gl_in[2].gl_ClipDistance[1]) / 3.0;
  gl_ClipDistance[2] = (gl_in[0].gl_ClipDistance[2] + gl_in[1].gl_ClipDistance[2] + gl_in[2].gl_ClipDistance[2]) / 3.0;
  v_color = (gs_in[0].v_color + gs_in[1].v_color + gs_in[2].v_color) / 3.0;
  v_normal = normalize(gs_in[0].v_normal + gs_in[1].v_normal + gs_in[2].v_normal);
  v_eyeVertexPosition = (gs_in[0].v_eyeVertexPosition + gs_in[1].v_eyeVertexPosition + gs_in[2].v_eyeVertexPosition) / 3.0;
  v_directionLight = gs_in[0].v_directionLight;
  for(int t = 0; t < MAX_TEXTURE_UNITS; ++t)
      v_texcoord[t] = (t == u_paletteTexUnit)
                      ? vec3(zoneVal, 0.0, 0.0)
                      : (gs_in[0].v_texcoord[t] + gs_in[1].v_texcoord[t] + gs_in[2].v_texcoord[t]) / 3.0;
  EmitVertex();
}

void main()
{
  float v0 = gs_in[0].v_texcoord[u_paletteTexUnit].x;
  float v1 = gs_in[1].v_texcoord[u_paletteTexUnit].x;
  float v2 = gs_in[2].v_texcoord[u_paletteTexUnit].x;

  bool e01 = (abs(v0 - v1) > 0.001);
  bool e12 = (abs(v1 - v2) > 0.001);
  bool e20 = (abs(v2 - v0) > 0.001);
  int nChanges = int(e01) + int(e12) + int(e20);

  if(nChanges == 0)
  {
    emitFull(0, v0);
    emitFull(1, v0); 
    emitFull(2, v0);
    EndPrimitive();
  }
  else if(nChanges == 2)
  {
    int solo, a, b;
    float vSolo, vOther;
    if      (!e12) { solo=0; a=1; b=2; vSolo=v0; vOther=v1; }
    else if (!e20) { solo=1; a=2; b=0; vSolo=v1; vOther=v2; }
    else           { solo=2; a=0; b=1; vSolo=v2; vOther=v0; }

    emitFull(solo, vSolo);
    emitMid(solo, a, vSolo);
    emitMid(solo, b, vSolo);
    EndPrimitive();

    emitFull(a,       vOther);
    emitMid(solo, b,  vOther);
    emitMid(solo, a,  vOther);   
    EndPrimitive();


    emitFull(a,       vOther);
    emitFull(b,       vOther);
    emitMid(solo, b,  vOther);
    EndPrimitive();
  }
  else
  {
    emitFull(0, v0);
    emitMid(0, 1, v0);
    emitCenter(v0);
    EndPrimitive();
    emitFull(0, v0);
    emitCenter(v0);
    emitMid(2, 0, v0);
    EndPrimitive();

    emitFull(1, v1);
    emitMid(1, 2, v1);
    emitCenter(v1);
    EndPrimitive();
    emitFull(1, v1);
    emitCenter(v1);
    emitMid(0, 1, v1);
    EndPrimitive();

    emitFull(2, v2);
    emitMid(2, 0, v2);
    emitCenter(v2);
    EndPrimitive();
    emitFull(2, v2);
    emitCenter(v2);
    emitMid(1, 2, v2);
    EndPrimitive();
  }
}