u64_to_u64 * console_history_cnt;
u64_to_u64 * console_height;
u64_to_u64 * console_history;
u64_to_u64 * console_index;

u64 console_class;
u64 console_add_history_methods;
u64 console_command_entered_method;

CREATE_TABLE(console_height, u64, float);
CREATE_MULTI_TABLE(console_history, u64, u64);
CREATE_STRING_TABLE(strings, u64);
CREATE_TABLE_DECL2(console_index, u64, u64);
CREATE_TABLE2(console_index, u64, u64);

