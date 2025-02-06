struct vertex_data
{
  v3 position;
  v3 color;
  v2 tex_coords;
};

struct render_unit
{
  u32 index_count;
  u32 *indices;
  u32 vertex_count;
  vertex_data *vertices;
  mat4 model;

  render_unit *next;
};

struct renderer
{
  memory_arena frame_arena;

  u32 unit_count;
  u32 max_units;
  render_unit *head;

  u32 VAO, VBO, EBO;
  u32 texture_atlas, font_texture;
  u32 width, height;

  loaded_bmp tex_atlas;
  // TODO: Likely should move this out of the renderer.
  f32 card_width;
  f32 card_height;

  mat4 mvp;
};

