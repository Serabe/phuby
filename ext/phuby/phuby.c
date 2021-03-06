#include <phuby.h>


VALUE mPhuby;
VALUE cPhubyRuntime;

static int phuby_ub_write(const char *str, unsigned int strlen)
{
  VALUE self = rb_funcall(cPhubyRuntime, rb_intern("instance"), 0);
  VALUE handler = rb_iv_get(self, "@events");

  rb_funcall(handler, rb_intern("write"), 1, rb_str_new(str, strlen));
  return strlen;
}

static int phuby_header_handler(
    sapi_header_struct *sapi_header,
    sapi_header_op_enum op,
    sapi_headers_struct *sapi_headers)
{
  VALUE header = sapi_header->header ?
    rb_str_new2(sapi_header->header) :
    Qnil;

  VALUE rb_op = Qnil;

  int return_value = 0;

  switch(op) {
    case SAPI_HEADER_DELETE_ALL:
      rb_op = ID2SYM(rb_intern("delete_all"));
      break;
    case SAPI_HEADER_DELETE:
      rb_op = ID2SYM(rb_intern("delete"));
      break;
    case SAPI_HEADER_ADD:
    case SAPI_HEADER_REPLACE:
      rb_op = ID2SYM(rb_intern("store"));
      return_value = SAPI_HEADER_ADD;
      break;
    default:
      rb_raise(rb_eRuntimeError, "header_handler huh?");
  }

  VALUE self = rb_funcall(cPhubyRuntime, rb_intern("instance"), 0);
  VALUE handler = rb_iv_get(self, "@events");

  rb_funcall(handler, rb_intern("header"), 2, header, rb_op);

  return return_value;
}

static VALUE start(VALUE self)
{
  /* FIXME:
   * I got these from the book.  I don't know wtf they're for yet. */
  int argc = 1;
  char *argv[2] = { "embed4", NULL };
  /* end FIXME */

  php_embed_module.ub_write       = phuby_ub_write;
  php_embed_module.header_handler = phuby_header_handler;
  php_embed_init(argc, argv);

  return Qnil;
}

static VALUE stop(VALUE self)
{
  php_embed_shutdown();

  return Qnil;
}

/* IO reader callback for PHP IO streams */
static size_t rb_io_reader(void *handle, char *buf, size_t len)
{
  VALUE io = (VALUE)handle;
  VALUE string = rb_funcall(io, rb_intern("read"), 1, INT2NUM(len));

  if(Qnil == string) return 0;

  memcpy(buf, StringValuePtr(string), (unsigned int)RSTRING_LEN(string));

  return RSTRING_LEN(string);
}

static size_t rb_io_sizer(void *handle)
{
}

static void rb_io_closer(void *handle)
{
  // We'll let the caller close their own IO
  return;
}

static VALUE native_eval_io(VALUE self, VALUE io, VALUE filename)
{
  zend_file_handle script;

  script.type = ZEND_HANDLE_STREAM;
  script.filename = StringValuePtr(filename);
  script.opened_path = NULL;

  memset(&script.handle.stream.mmap, 0, sizeof(zend_mmap));

  script.handle.stream.handle = (void *)io;
  script.handle.stream.isatty = 0;

  script.handle.stream.reader = rb_io_reader;
  script.handle.stream.fsizer = rb_io_sizer;
  script.handle.stream.closer = rb_io_closer;

  script.free_filename = 0;

  php_execute_script(&script);

  return Qnil;
}

static VALUE native_eval(VALUE self, VALUE string, VALUE filename)
{

  zval return_value;

  zend_first_try {
    zend_eval_string(
      StringValuePtr(string),
      NULL,
      StringValuePtr(filename)
    );
  } zend_end_try();

  return Qnil;
}

static VALUE get(VALUE self, VALUE key)
{
  zval **value;

  if(zend_hash_find(EG(active_symbol_table),
        StringValuePtr(key),
        RSTRING_LEN(key) + 1,
        (void **)&value) == SUCCESS) {

    switch(Z_TYPE_P(*value)) {
      case IS_NULL:
        return Qnil;
        break;
      case IS_BOOL:
        if(Z_BVAL_P(*value))
          return Qtrue;
        else
          return Qfalse;
        break;
      case IS_LONG:
        return INT2NUM(Z_LVAL_P(*value));
        break;
      case IS_DOUBLE:
        return rb_float_new(Z_DVAL_P(*value));
        break;
      case IS_STRING:
        return rb_str_new(Z_STRVAL_P(*value), Z_STRLEN_P(*value));
        break;
      default:
        rb_raise(rb_eRuntimeError, "Whoa, I don't know how to convert that");
    }
  }

  return Qnil;
}

static VALUE set(VALUE self, VALUE key, VALUE value)
{
  zval *php_value;

  MAKE_STD_ZVAL(php_value);

  switch(TYPE(value))
  {
    case T_FIXNUM:
      ZVAL_LONG(php_value, NUM2INT(value));
      break;

    case T_TRUE:
      ZVAL_BOOL(php_value, value == Qtrue ? 1 : 0);
      break;

    case T_FLOAT:
    case T_BIGNUM:
      ZVAL_DOUBLE(php_value, NUM2DBL(value));
      break;

    case T_STRING:
      ZVAL_STRINGL(php_value, StringValuePtr(value), RSTRING_LEN(value), 1);
      break;
  }

  ZEND_SET_SYMBOL(EG(active_symbol_table), StringValuePtr(key), php_value);

  return value;
}

void Init_phuby()
{
  mPhuby = rb_define_module("Phuby");

  /* FIXME: This belongs in it's own .c file */
  cPhubyRuntime = rb_define_class_under(mPhuby, "Runtime", rb_cObject);

  rb_define_method(cPhubyRuntime, "start", start, 0);
  rb_define_method(cPhubyRuntime, "stop", stop, 0);
  rb_define_method(cPhubyRuntime, "[]", get, 1);
  rb_define_method(cPhubyRuntime, "[]=", set, 2);

  rb_define_private_method(cPhubyRuntime, "native_eval", native_eval, 2);
  rb_define_private_method(cPhubyRuntime, "native_eval_io", native_eval_io, 2);
}
