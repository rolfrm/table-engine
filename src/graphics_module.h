u64 control_new_named(const char * name);
u64 control_window;

void control_set_size(u64 control, double width, double height);
bool control_try_get_size(u64 control, double * width, double * height);

u64 get_class(u64 object);
void set_class(u64 object, u64 base);


void show_window(u64 windowid);
void unshow_window(u64 windowid);

int graphics_process_active_window_count();

void graphics_process();


void render_text(const char * text, size_t len, vec2 window_size, vec2 shared_offset);
void initialized_fonts();
