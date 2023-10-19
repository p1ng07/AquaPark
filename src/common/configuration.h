typedef struct {
  char* value;
  int length;
} string;

union conf_value {
  int i;
  string str;
  float f;
};

typedef struct {} configuration;

configuration extract_config_from_file(string file_path);
