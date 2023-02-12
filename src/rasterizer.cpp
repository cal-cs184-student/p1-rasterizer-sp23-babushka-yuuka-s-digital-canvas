#include "rasterizer.h"

using namespace std;

namespace CGL {

  RasterizerImp::RasterizerImp(PixelSampleMethod psm, LevelSampleMethod lsm,
    size_t width, size_t height,
    unsigned int sample_rate) {
    this->psm = psm;
    this->lsm = lsm;
    this->width = width;
    this->height = height;
    this->sample_rate = sample_rate;

    sample_buffer.resize(width * height * sample_rate, Color::White);
  }

  void RasterizerImp::fill_superpixel(size_t x, size_t y, size_t w, Color c)
  {
      if (x < 0 || x >= width) return;
      if (y < 0 || y >= height) return;

      sample_buffer[(y * width + x)*sample_rate + w] = c;
  }

  // Used by rasterize_point and rasterize_line
  void RasterizerImp::fill_pixel(size_t x, size_t y, Color c) {
    // TODO: Task 2: You might need to this function to fix points and lines (such as the black rectangle border in test4.svg)
    // NOTE: You are not required to implement proper supersampling for points and lines
    // It is sufficient to use the same color for all supersamples of a pixel for points and lines (not triangles)


    for (int k = 0; k < 3; ++k) {
          this->rgb_framebuffer_target[3 * (y * width + x) + k] = (&c.r)[k] * 255;
        }
  }

  // Rasterize a point: simple example to help you start familiarizing
  // yourself with the starter code.
  //
  void RasterizerImp::rasterize_point(float x, float y, Color color) {
    // fill in the nearest pixel
    int sx = (int)floor(x);
    int sy = (int)floor(y);

    // check bounds
    if (sx < 0 || sx >= width) return;
    if (sy < 0 || sy >= height) return;

    for (int i = 0; i < sample_rate; i++) {
        fill_superpixel(sx, sy, i, color);
    }
  }

  // Rasterize a line.
  void RasterizerImp::rasterize_line(float x0, float y0,
    float x1, float y1,
    Color color) {
    if (x0 > x1) {
      swap(x0, x1); swap(y0, y1);
    }

    float pt[] = { x0,y0 };
    float m = (y1 - y0) / (x1 - x0);
    float dpt[] = { 1,m };
    int steep = abs(m) > 1;
    if (steep) {
      dpt[0] = x1 == x0 ? 0 : 1 / abs(m);
      dpt[1] = x1 == x0 ? (y1 - y0) / abs(y1 - y0) : m / abs(m);
    }

    while (floor(pt[0]) <= floor(x1) && abs(pt[1] - y0) <= abs(y1 - y0)) {
      rasterize_point(pt[0], pt[1], color);
      pt[0] += dpt[0]; pt[1] += dpt[1];
    }
  }

  // Rasterize a triangle.
  void RasterizerImp::rasterize_triangle(float x0, float y0,
    float x1, float y1,
    float x2, float y2,
    Color color) {
    // TODO: Task 1: Implement basic triangle rasterization here, no supersampling
      //rasterize_line(x0, y0, x1, y1, color);
      //rasterize_line(x1, y1, x2, y2, color);
      //rasterize_line(x2, y2, x0, y0, color);
      int sqrt_sample = sqrt(sample_rate);
      float step = (float)1 / (sqrt_sample + 1);

      Vector3D z(0, 0, 1), p1(x0, y0, 0), p2(x1, y1, 0), p3(x2, y2, 0);

      Vector3D l1 = p1 - p2, l2 = p2 - p3, l3 = p3 - p1;

      Vector3D n1 = cross(z, l1), n2 = cross(z, l2), n3 = cross(z, l3);

      int x_bounds[2]{};
      int y_bounds[2]{};
      x_bounds[0] = floor(min({ x0, x1, x2 }));
      x_bounds[1] = ceil(max({ x0, x1, x2 }));
      y_bounds[0] = floor(min({ y0, y1, y2 }));
      y_bounds[1] = ceil(max({ y0, y1, y2 }));
      for (int x = x_bounds[0]; x < x_bounds[1]; x++) {
          for (int y = y_bounds[0]; y < y_bounds[1]; y++) {
              int w = 0;
              for (int i = 0; i < sqrt_sample; i++) {
                  for (int j = 0; j < sqrt_sample; j++) {
                      float xt = (float)x + (((float)i + 1) * step);
                      float yt = (float)y + (((float)j + 1) * step);
                      Vector3D p(xt, yt, 0);
                      float a = dot(p - p2, n1);
                      float b = dot(p - p3, n2);
                      float c = dot(p - p1, n3);
                      if ((a < 0) && (b < 0) && (c < 0) ||
                          (a > 0) && (b > 0) && (c > 0)) {
                          fill_superpixel(x, y, w, color);
                      }
                      w++;
                  }
              }
          }
      }
    // TODO: Task 2: Update to implement super-sampled rasterization



  }


  void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
    float x1, float y1, Color c1,
    float x2, float y2, Color c2)
  {
    // TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
    // Hint: You can reuse code from rasterize_triangle
      int sqrt_sample = sqrt(sample_rate);
      float step = (float)1 / (sqrt_sample + 1);

      Vector3D z(0, 0, 1), A(x0, y0, 0), B(x1, y1, 0), C(x2, y2, 0);

      Vector3D AB = A - B, BC = B - C, CA = C - A;

      Vector3D nAB = cross(z, AB), nBC = cross(z, BC), nCA = cross(z, CA);

      Matrix3x3 M(x0, x1, x2, y0, y1, y2, 1, 1, 1);
      Matrix3x3 M_inv = M.inv();

      int x_bounds[2]{};
      int y_bounds[2]{};
      x_bounds[0] = floor(min({ x0, x1, x2 }));
      x_bounds[1] = ceil(max({ x0, x1, x2 }));
      y_bounds[0] = floor(min({ y0, y1, y2 }));
      y_bounds[1] = ceil(max({ y0, y1, y2 }));
      for (int x = x_bounds[0]; x < x_bounds[1]; x++) {
          for (int y = y_bounds[0]; y < y_bounds[1]; y++) {
              int w = 0;
              for (int i = 0; i < sqrt_sample; i++) {
                  for (int j = 0; j < sqrt_sample; j++) {
                      float xt = (float)x + (((float)i + 1) * step);
                      float yt = (float)y + (((float)j + 1) * step);
                      Vector3D X(xt, yt, 1);

                      Vector3D B_coords = M_inv * X;

                      float dota = dot(X - B, nAB);
                      float dotb = dot(X - C, nBC);
                      float dotc = dot(X - A, nCA);
                      if ((dota < 0) && (dotb < 0) && (dotc < 0) ||
                          (dota > 0) && (dotb > 0) && (dotc > 0)) {
                          Color c = B_coords.x * c0 + B_coords.y * c1 + B_coords.z * c2;
                          fill_superpixel(x, y, w, c);
                      }
                      w++;
                  }
              }
          }
      }


  }


  void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
    float x1, float y1, float u1, float v1,
    float x2, float y2, float u2, float v2,
    Texture& tex)
  {
    // TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
    // TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
    // Hint: You can reuse code from rasterize_triangle/rasterize_interpolated_color_triangle
      int sqrt_sample = sqrt(sample_rate);
      float step = (float)1 / (sqrt_sample + 1);
      Vector3D z(0, 0, 1), A(x0, y0, 0), B(x1, y1, 0), C(x2, y2, 0);

      Vector3D AB = A - B, BC = B - C, CA = C - A;

      Vector3D nAB = cross(z, AB), nBC = cross(z, BC), nCA = cross(z, CA);

      Matrix3x3 M(x0, x1, x2, y0, y1, y2, 1, 1, 1);
      Matrix3x3 Mtex(u0, u1, u2, v0, v1, v2, 1, 1, 1);
      Matrix3x3 M_inv = M.inv();

      int x_bounds[2]{};
      int y_bounds[2]{};
      x_bounds[0] = floor(min({ x0, x1, x2 }));
      x_bounds[1] = ceil(max({ x0, x1, x2 }));
      y_bounds[0] = floor(min({ y0, y1, y2 }));
      y_bounds[1] = ceil(max({ y0, y1, y2 }));
      for (int x = x_bounds[0]; x < x_bounds[1]; x++) {
          for (int y = y_bounds[0]; y < y_bounds[1]; y++) {
              int w = 0;
              for (int i = 0; i < sqrt_sample; i++) {
                  for (int j = 0; j < sqrt_sample; j++) {
                      float xt = (float)x + (((float)i + 1) * step);
                      float yt = (float)y + (((float)j + 1) * step);
                      Vector3D X(xt, yt, 1);

                      Vector3D B_coords = M_inv * X;

                      float dota = dot(X - B, nAB);
                      float dotb = dot(X - C, nBC);
                      float dotc = dot(X - A, nCA);
                      if ((dota < 0) && (dotb < 0) && (dotc < 0) ||
                          (dota > 0) && (dotb > 0) && (dotc > 0)) {
                          
                          Vector3D tex_coord = Mtex * B_coords;

                          Color c(Color(1, 0, 1));
                          switch (psm)
                          {
                          case CGL::P_NEAREST:
                              c = tex.sample_nearest(Vector2D(tex_coord.x, tex_coord.y), 0);
                              break;
                          case CGL::P_LINEAR:
                              c = tex.sample_bilinear(Vector2D(tex_coord.x, tex_coord.y), 0);
                              break;
                          default:
                              break;
                          }

                          fill_superpixel(x, y, w, c);
                      }
                      w++;
                  }
              }
          }
      }



  }

  void RasterizerImp::set_sample_rate(unsigned int rate) {
    // TODO: Task 2: You may want to update this function for supersampling support

    this->sample_rate = rate;


    this->sample_buffer.resize(width * height * sample_rate, Color::White);
  }


  void RasterizerImp::set_framebuffer_target(unsigned char* rgb_framebuffer,
    size_t width, size_t height)
  {
    // TODO: Task 2: You may want to update this function for supersampling support

    this->width = width;
    this->height = height;
    this->rgb_framebuffer_target = rgb_framebuffer;


    this->sample_buffer.resize(width * height * sample_rate, Color::White);
  }


  void RasterizerImp::clear_buffers() {
    std::fill(rgb_framebuffer_target, rgb_framebuffer_target + 3 * width * height, 255);
    std::fill(sample_buffer.begin(), sample_buffer.end(), Color::White);
  }


  // This function is called at the end of rasterizing all elements of the
  // SVG file.  If you use a supersample buffer to rasterize SVG elements
  // for antialising, you could use this call to fill the target framebuffer
  // pixels from the supersample buffer data.
  //
  void RasterizerImp::resolve_to_framebuffer() {
    // TODO: Task 2: You will likely want to update this function for supersampling support

    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
          Color col = Color::Black;
          for (int i = 0; i < sample_rate; i++) {
                col += sample_buffer[(y * width + x)*sample_rate + i];
          }
          col.r /= sample_rate;
          col.g /= sample_rate;
          col.b /= sample_rate;
          fill_pixel(x, y, col);
      }
    }

  }

  Rasterizer::~Rasterizer() { }


}// CGL
