
typedef enum{
  MODEL_TYPE_POLYGONAL = 100,
  MODEL_TYPE_TEXT = 200,
  MODEL_TYPE_FONT = 300, // sets the font for every sub element
}model_type;

typedef struct{
  vec2 dim_px;
  texture tex;
  bool loaded;
  bool alpha;
  char * path;
}text_cache;


typedef struct{
  // a view is essentially a model that is being drawn to a texture.
  // it can be static or non-static
  // an example of a non-static view could be a mirror
  // an example of a static view is a generated model, screenshot or picture.

  vec2 dim_px;
  texture tex;

  mat4 view_matrix;
  mat4 camera_matrix;
  u32 model;
  bool loaded;
  bool alpha;

}view;

typedef struct{
  model_type type;
  u64 model_id;

  // this data will be loaded based on the model id.
  blit3d_polygon * verts;
  blit3d_polygon * uvs;
  
  text_cache cache;
  u32 view_id;      
  union{
    
  //distance_field_model * dist;
    struct {
      char * text;
    };
    struct{

      
    };
  };
  vec4 color;
  vec3 offset;
  vec3 rotation;
  vec3 scale;
}model;

typedef struct{
  blit3d_context * blit3d;

  mat4 view_matrix;
  mat4 camera_matrix;

  
}model_context;
