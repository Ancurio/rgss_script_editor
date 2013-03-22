extern "C" {
#include <ruby.h>
#include <zlib.h>
}

#include "ruby_data.hxx"
#include <QtCore/QFileInfo>
#include <cassert>

Script::Script(unsigned const m, std::string const& n, std::string const& d)
    : magic(m), name(n)
{
  // extract with zlib
  boost::container::vector<Bytef> buf(d.size() * 5);
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
  assert(result == Z_OK);
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
    assert(RARRAY_LEN(script) == 3);
    VALUE const* const ary = RARRAY_PTR(script);
    assert(rb_type(ary[0]) == T_FIXNUM);
    assert(rb_type(ary[1]) == T_STRING);
    assert(rb_type(ary[2]) == T_STRING);
    data.push_back(Script(
        NUM2INT(ary[0]),
        std::string(RSTRING_PTR(ary[1]), RSTRING_LEN(ary[1])),
        std::string(RSTRING_PTR(ary[2]), RSTRING_LEN(ary[2]))));
  }

  return true;
}

bool dumpScripts(std::string const& file, ScriptList const& data) {
  assert(not data.empty());
  VALUE const ruby_data = rb_ary_new2(data.size());
  rb_ary_store(ruby_data, data.size() - 1, Qnil);
  assert(RARRAY_LEN(ruby_data) == int(data.size()));
  for(size_t i = 0; i < data.size(); ++i) {
    VALUE const script = rb_ary_new2(3);
    rb_ary_store(script, 3 - 1, Qnil);
    assert(RARRAY_LEN(script) == 3);

    RARRAY_PTR(script)[0] = INT2NUM(data[i].magic);
    RARRAY_PTR(script)[1] = rb_str_new(data[i].name.data(), data[i].name.size());

    // compress with zlib
    boost::container::vector<Bytef> buf(data[i].data.size() * 2);
    uLongf buf_len = buf.size();
    int result = compress(
        buf.data(), &buf_len,
        reinterpret_cast<Bytef const*>(data[i].data.data()),
        data[i].data.size());
    assert(result == Z_OK);
    std::string const script_data(buf.begin(), buf.end());
    RARRAY_PTR(script)[2] = rb_str_new(script_data.data(), script_data.size());

    RARRAY_PTR(ruby_data)[i] = script;
  }

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
