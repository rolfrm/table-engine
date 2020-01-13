u64 control_new_named(const char * name);
extern u64 window_class;
extern u64 control_class;
void control_set_size(u64 control, double width, double height);
bool control_try_get_size(u64 control, double * width, double * height);

u64 get_class(u64 object);
void set_class(u64 object, u64 base);

void * class_get_method(u64 object, u64 method);
void class_set_method(u64 class, u64 method, void * methodf);
extern u64 render_control_method;
extern u64 key_event_method;

extern vec2 window_size;


void show_window(u64 windowid);
void unshow_window(u64 windowid);

int graphics_process_active_window_count();

void graphics_process();


void render_text(const char * text, size_t len, vec2 window_size, vec2 shared_offset);
vec2 measure_text(const char * text, size_t len);
void initialized_fonts();

void control_add_sub(u64 object, u64 subobject);
u64 control_get_subs(u64 object, u64 * array, u64 count, u64 * index);
void render_control(u64 control);
void control_set_position(u64 control, f64 x, f64 y);
bool control_get_position(u64 control, f64 *x, f64 * y);
void control_set_focus(u64 window, u64 control);
extern vec2 current_control_offset;


//textbox
extern u64 textbox_class;
void textbox_set_text(u64 object, const char * text);
void textbox_init();

// console
extern u64 console_class;
void console_init();
