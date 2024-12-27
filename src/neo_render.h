struct vertex_data
{
  v3 position;
  v3 color;
  v2 tex_coords;
};

struct render_unit
{
  vertex_data *vertices;
  u32 vertex_count;
};

struct renderer
{
  render_unit *units;
  u32 unit_count;
  loaded_bmp tex_atlas;
};

internal void InitRenderer(renderer *Renderer, thread_context *Thread, app_memory *Memory);
