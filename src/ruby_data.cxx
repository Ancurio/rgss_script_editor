extern "C" {
#include <ruby.h>
#include <zlib.h>
}

#include "ruby_data.hxx"
#include <QtCore/QFileInfo>

Script::Script(unsigned const m, std::string const& n, std::string const& d)
    : magic(m), name(n)
{
  // extract with zlib
  QVector<Bytef> buf(d.size() * 5);
  int result = Z_OK;
  uLongf  dst_len = buf.size();
  while((result = ::uncompress(
            buf.data(), &dst_len,
            reinterpret_cast<Bytef const*>(d.data()),
            d.size())) == Z_BUF_ERROR)
  {
    buf.resize(buf.size() + d.size());
    dst_len = buf.size();
  }
  Q_ASSERT(result == Z_OK);
  this->data.assign(buf.begin(), buf.end());
}

bool loadScripts(std::string const& file, ScriptList& data) {
  if(not QFileInfo(QString::fromUtf8(file.c_str())).exists()) { return false; }

  VALUE const stream = rb_file_open(file.c_str(), "rb");
  VALUE const scripts = rb_marshal_load(stream);

  if(rb_type(scripts) != T_ARRAY) { return false; }

  data.clear();

  for(int i = 0; i < RARRAY_LEN(scripts); ++i) {
    VALUE const script = RARRAY_PTR(scripts)[i];
    Q_ASSERT(RARRAY_LEN(script) == 3);
    VALUE const* const ary = RARRAY_PTR(script);
    Q_ASSERT(rb_type(ary[0]) == T_FIXNUM);
    Q_ASSERT(rb_type(ary[1]) == T_STRING);
    Q_ASSERT(rb_type(ary[2]) == T_STRING);
    data.push_back(Script(
        NUM2INT(ary[0]),
        std::string(RSTRING_PTR(ary[1]), RSTRING_LEN(ary[1])),
        std::string(RSTRING_PTR(ary[2]), RSTRING_LEN(ary[2]))));
  }

  return true;
}

bool dumpScripts(std::string const& file, ScriptList const& data) {
  Q_ASSERT(!data.empty());
  VALUE const ruby_data = rb_ary_new2(data.size());
  for(QVector<Script>::ConstIterator i = data.begin(); i < data.end(); ++i) {
    VALUE const script = rb_ary_new2(3);

    rb_ary_push(script, INT2NUM(i->magic));
    rb_ary_push(script, rb_str_new(i->name.data(), i->name.size()));

    // compress with zlib
    QVector<Bytef> buf(i->data.size() * 2);
    uLongf buf_len = buf.size();
    int result = compress(
        buf.data(), &buf_len,
        reinterpret_cast<Bytef const*>(i->data.data()),
        i->data.size());
    Q_ASSERT(result == Z_OK);
    rb_ary_push(script, rb_str_new(reinterpret_cast<char const*>(buf.data()), buf_len));

    Q_ASSERT(RARRAY_LEN(script) == 3);
    rb_ary_push(ruby_data, script);
  }

  Q_ASSERT(RARRAY_LEN(ruby_data) == int(data.size()));
  rb_marshal_dump(ruby_data, rb_file_open(file.c_str(), "wb"));

  return true;
}

bool parseScript(std::string const&) {
  // TODO
  return true;
}

RubyInstance::RubyInstance() {
#ifdef RUBY_INIT_STACK
  RUBY_INIT_STACK;
#endif
  ruby_init();
  ruby_init_loadpath();
}

RubyInstance::~RubyInstance() {
  ruby_finalize();
}
