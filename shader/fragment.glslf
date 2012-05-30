// Check out this for details.
// http://www.opengl.org/documentation/specs/version2.1/glspec21.pdf

vec3 rgb2hsv(vec3 c) {

  float M = max(max(c.r, c.g), c.b);
  float m = min(min(c.r, c.g), c.b);
  float C = M - m;
  float del = 60.0 / C;

  float h = 0.0;
  float s = M > 0.0 ? 1.0 - m / M : 0.0;
  float v = M;

  if (M == c.r) {
    h = (c.g - c.b) * del;
    if (h < 0.0) h += 360.0;
  } else if (M == c.g)
    h = (c.b - c.r) * del + 120.0;
  else
    h = (c.r - c.g) * del + 240.0;

  return vec3(h, s, v);

}

vec3 hsv2rgb(vec3 c) {

  float H = c.x / 60.0;
  float Hi = H - 6.0 * floor(H / 6.0);
  float f = H - floor(H);
  
  float p = c.z * (1.0 - c.y);
  float q = c.z * (1.0 - f * c.y);
  float t = c.z * (1.0 - (1.0 - f) * c.y);

  if (Hi < 1.0)
    return vec3(c.z, t, p);
  else if (Hi < 2.0)
    return vec3(q, c.z, p);
  else if (Hi < 3.0)
    return vec3(p, c.z, t);
  else if (Hi < 4.0)
    return vec3(p, q, c.z);
  else if (Hi < 5.0)
    return vec3(t, p, c.z);
  else
    return vec3(c.z, p, q);

}

vec3 rgb2hsl(vec3 c) {

  float M = max(max(c.r, c.g), c.b);
  float m = min(min(c.r, c.g), c.b);
  float C = M - m;
  float del = 60.0 / C;

  float h = 0.0;
  float l = M + m;
  float t = 1.0 - abs(l - 1.0);
  float s = t > 0.0 ? C / t : 0.0;

  l /= 2.0;

  if (M == c.r) {
    h = (c.g - c.b) * del;
    if (h < 0.0) h += 360.0;
  } else if (M == c.g)
    h = (c.b - c.r) * del + 120.0;
  else
    h = (c.r - c.g) * del + 240.0;

  return vec3(h, s, l);

}

vec3 hsl2rgb(vec3 c) {

  float H = c.x / 60.0;
  float Hi = H - 6.0 * floor(H / 6.0);
  float f = H - floor(H);
  
  float C = (1.0 - abs(2.0 * c.z - 1.0)) * c.y;
  float X = C * (1.0 - abs(H - 2.0 * floor(H / 2.0) - 1.0));
  float m = c.z - C / 2.0;
  
  if (Hi < 1.0)
    return vec3(C + m, X + m, m);
  else if (Hi < 2.0)
    return vec3(X + m, C + m, m);
  else if (Hi < 3.0)
    return vec3(m, C + m, X + m);
  else if (Hi < 4.0)
    return vec3(m, X + m, C + m);
  else if (Hi < 5.0)
    return vec3(X + m, m, C + m);
  else
    return vec3(C + m, m, X + m);

}

uniform vec3 uHsbTune;
uniform sampler2D uSampler;

varying vec2 vTextureCoord;

void main() {

  vec4 Color = texture2D(uSampler, vTextureCoord);
  if (any(notEqual(uHsbTune, vec3(0, 0, 0))))
    Color.rgb = hsl2rgb(rgb2hsl(Color.rgb) + uHsbTune);
  gl_FragColor = Color;

}
