/**
 * Initializer function for a domain object.
 *
 * @decl: domain declaration, for purposes of parameterization.
 * @parm_r: When this function returns successfully, a pointer to a
 *      parameterization object is stored here.
 *
 * Returns:
 *       - WSL_OK if successful
 *       - WSL_EPARSE if the declaration line could not be parsed
 *       - WSL_ENOMEM if the domain object could not be created due to lack
 *              of available memory.
 */
typedef
enum wsl_retcode
WSL_DOMAIN_INITOR(const char *decl, void **parm_r);

/**
 * Finalizer function for a domain object.
 *
 * @domain: A paramerization object that was created by the corresponding
 *      initializer function.
 *
 * This operation is always successful.
 */
typedef
void
WSL_DOMAIN_EXITOR(void *parm);

/**
 * Initializer function for a database column object
 *
 * @data: A parameterization object that was created by the corresponding
 *      initializer function.
 * @column_r: When this function returns successfully, a pointer to a column
 *      object is stored here.
 */
typedef
enum wsl_retcode
WSL_COLUMN_INITOR(void *parm, void **column_r);

/**
 * Finalizer function for a database column object
 *
 * @column: The database column object that should be destroyed.
 *
 * Returns:
 *      - WSL_OK if successful
 *      - WSL_ENOMEM if the column object could not be created due to lack
 *              of available memory.
 *
 * This operation is always successful.
 */
typedef
void
WSL_COLUMN_EXITOR(void *column);

/**
 * Encoder function for values of a given domain.
 *
 * @column: Column object that contains the value that is to be encoded.
 * @id: Index of the value in the column object.
 * @buf: Target memory location to which the encoded value should be written.
 * @end: End of writable memory; buf <= end;
 * @end_r: If successful, a pointer to the first byte of output memory that was
 *      not written to is stored here; buf <= end_r <= end (and in particular
 *      buf < end_r <= end since at least one byte should be written.)
 *
 * Returns:
 *      - WSL_OK if successful.
 *      - WSL_ENOMEM if more output memory is needed (than buf..end) to encode
 *              the value.
 *      - WSL_EINVAL if the value could not be encoded due to violated
 *              value constraints. This should normally not happen, but
 *              some specialized datatypes might need this.
 */
typedef
enum wsl_encode_retcode
WSL_VALUE_ENCODER(
        void *column,
        size_t id,
        unsigned char *buf,
        unsigned char *end,
        unsigned char *end_r);

/**
 * Decoder function for values of a given domain.
 *
 * @buf: Pointer to the NUL-terminated lexem which holds the encoded value.
 * @column: Column object where the decoded value should be stored.
 *
 * Returns:
 *      - WSL_OK if successful.
 *      - WSL_EPARSE if the lexem is invalid.
 *      - WSL_ENOMEM if a required memory allocation could not be satisfied.
 */
typedef
enum wsl_decode_retcode
WSL_VALUE_DECODER(unsigned char *buf, void *column);

/**
 * Datatype structure holding domain parser and destructor functions.
 *
 * @name: Name of the datatype ("domain parser")
 * @init_domain: Domain (parser and) initializer function
 * @exit_domain: Domain destructor function
 */
struct wsl_datatype {
        const char *name;
        WSL_DOMAIN_INITOR init_domain;
        WSL_DOMAIN_EXITOR exit_domain;
};

/**
 * Domain structure - a "parameterized datatype".
 *
 * @init_column: column initializer
 * @exit_column: column finalizer
 * @encode_value: value encoder
 * @decode_value: value decode
 * @parm: parameterization object
 */
struct wsl_domain {
        WSL_COLUMN_INITOR init_column;
        WSL_COLUMN_EXITOR exit_column;
        WSL_VALUE_ENCODER encode_value;
        WSL_VALUE_DECODER decode_value;
        void *parm;
};

typedef struct wsl_ctx WSL_CTX;
typedef struct wsl_schema WSL_SCHEMA;
typedef struct wsl_domain WSL_DOMAIN;
typedef struct wsl_table WSL_TABLE;
typedef struct wsl_key WSL_KEY;
typedef struct wsl_ref WSL_REF;

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
        WSL_DOMAIN_INITOR *initor,
        WSL_DOMAIN_EXITOR *exitor);

enum wsl_retcode
wsl_schema_register_domain(
        wsl_schema *schema,
        const char *name,
        WSL_COLUMN_INITOR *init_column,
        WSL_COLUMN_EXITOR *exit_column,
        WSL_VALUE_ENCODER *encode_value,
        WSL_VALUE_DECODER *decode_value);

enum wsl_retcode
wsl_schema_register_table(
        wsl_schema *schema,
        const char *name,
        const char **domains);
