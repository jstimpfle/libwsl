typedef
enum wsl_retcode
wsl_domain_initor(const char *decl);

typedef
void
wsl_domain_exitor(void *data);

typedef
enum wsl_retcode
wsl_column_initor(void *data);

typedef
void
wsl_column_exitor(void *column);

typedef
enum wsl_decode_retcode
wsl_value_decoder(unsigned char *buf, void *column);

typedef
enum wsl_encode_retcode
wsl_value_encoder(unsigned char *buf, void *column);

struct wsl_datatype {
        const char *name;
        wsl_domain_initor init_domain;
        wsl_domain_exitor exit_domain;
};

struct wsl_domain {
        wsl_column_initor init_column;
        wsl_column_exitor exit_column;
        wsl_value_encoder encode_value;
        wsl_value_decoder decode_value;
        void *data;
};

typedef struct wsl_ctx wsl_ctx;
typedef struct wsl_schema wsl_schema;

enum wsl_retcode
wsl_ctx_init(wsl_ctx **ctx);

void
wsl_ctx_exit(wsl_ctx *ctx);

enum wsl_retcode
wsl_schema_init(wsl_ctx *ctx, wsl_schema **schema);

void
wsl_schema_exit(wsl_schema *schema);

enum wsl_retcode
wsl_ctx_register_datatype(
        wsl_ctx *ctx,
        const char *name,
        wsl_domain_initor *initor,
        wsl_domain_exitor *exitor);

enum wsl_retcode
wsl_schema_register_domain(
        wsl_schema *schema,
        const char *name,
        wsl_column_initor *init_column,
        wsl_column_exitor *exit_column,
        wsl_value_encoder *encode_value,
        wsl_value_decoder *decode_value);

enum wsl_retcode
wsl_schema_register_table(
        wsl_schema *schema,
        const char *name,
        const char **domains);
