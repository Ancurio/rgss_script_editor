extern "C" {
#include <zlib.h>
}

#include "ruby_data.hxx"
#include <QtCore/QFileInfo>

#include <QByteArray>
#include <QSet>

#define LINEEND_INT "\n"
#define LINEEND_EXT "\r\n"



bool parseScript(std::string const&) {
  // TODO
  return true;
}

static QByteArray compressData(const QByteArray &data)
{
  uLongf destLen = data.size() + 8;
  QByteArray buffer;

  int status;

  do {
    destLen *= 2;
    buffer.resize(destLen);

    status = compress(reinterpret_cast<Bytef*>(buffer.data()), &destLen,
                      reinterpret_cast<const Bytef*>(data.constData()), data.length());

    if (status != Z_OK && status != Z_BUF_ERROR)
      throw QByteArray("zlib decompression error");
  }
  while (status == Z_BUF_ERROR);

  buffer.resize(destLen);

  return buffer;
}

static QByteArray decompressData(const QByteArray &data)
{
  uLongf destLen = data.size() + 8;
  QByteArray buffer;

  int status;

  do {
    destLen *= 2;
    buffer.resize(destLen);

    status = uncompress(reinterpret_cast<Bytef*>(buffer.data()), &destLen,
                        reinterpret_cast<const Bytef*>(data.constData()), data.size());

    if (status != Z_OK && status != Z_BUF_ERROR)
      throw QByteArray("zlib decompression error");
  }
  while (status == Z_BUF_ERROR);

  buffer.resize(destLen);

  return buffer;
}

/* Reads one byte from the device */
static char readByte(QIODevice &dev)
{
  char byte;
  int count = dev.read(&byte, 1);

  if (count < 1)
    throw QByteArray("Unable to read data");

  return byte;
}

static void verifyByte(QIODevice &dev, char expected,
                       const char *error = "Bad data")
{
  char byte = readByte(dev);

  if (byte != expected)
    throw QByteArray(error);
}

static int readFixnum(QIODevice &dev)
{
  char head = readByte(dev);

  if (head == 0)
          return 0;
  else if (head > 5)
          return head - 5;
  else if (head < -4)
          return head + 5;

  int pos = (head > 0);
  int len = pos ? head : head * -1;

  char n1, n2, n3, n4;

  if (pos)
          n2 = n3 = n4 = 0;
  else
          n2 = n3 = n4 = 0xFF;

  n1 = readByte(dev);

  if (len >= 2)
          n2 = readByte(dev);
  if (len >= 3)
          n3 = readByte(dev);
  if (len >= 4)
          n4 = readByte(dev);

  int result = ((0xFF << 0x00) & (n1 << 0x00))
             | ((0xFF << 0x08) & (n2 << 0x08))
             | ((0xFF << 0x10) & (n3 << 0x10))
             | ((0xFF << 0x18) & (n4 << 0x18));

  return result;
}

static QByteArray readString(QIODevice &dev)
{
  int len = readFixnum(dev);
  QByteArray data = dev.read(len);

  if (data.size() != len)
    throw QByteArray("Error reading data");

  return data;
}

static QByteArray readIVARString(QIODevice &dev)
{
  QByteArray data;

  /* Read inner raw string */
  verifyByte(dev, '"');
  data = readString(dev);

  /* Read encoding */
  int ivarCount = readFixnum(dev);

  // XXX Can this be zero?
  if (ivarCount > 1)
    throw QByteArray("Cannot handle multiple string IVARS");

  /* Read encoding symbol */
  // XXX We can't deal with anythind outside Utf8/ASCII
  char symByte = readByte(dev);

  if (symByte == ':') {
    /* This symbol must be :E */
    QByteArray encSym = readString(dev);

    if (encSym.size() != 1 || encSym[0] != 'E')
      throw QByteArray("Bad data");
  }
  else if (symByte == ';') {
    /* The :E symbol is always the first symlink */
    verifyByte(dev, 0);
  }
  else {
    throw QByteArray("Bad data");
  }

  char encByte = readByte(dev);

  if (encByte != 'T' && encByte != 'F')
    throw QByteArray("Bad data");

  return data;
}

/* Abstractly reads a string, either raw or IVAR+Encoding based */
static QByteArray readRubyString(QIODevice &dev)
{
  QByteArray data;
  char type = readByte(dev);

  if (type == '"') {
    data = readString(dev);
  }
  else if (type == 'I') {
    data = readIVARString(dev);
  }
  else {
    throw QByteArray("Bad data");
  }

  return data;
}

static QString UTF8ToQString(const QByteArray &data)
{
  return QString::fromUtf8(data.constData(), data.size());
}

static Script readScript(QIODevice &dev)
{
  Script script;

  /* Verify array prologue */
  verifyByte(dev, '[');
  char len = readFixnum(dev);
  if (len != 3)
    throw QByteArray("Bad data");

  /* Read magic */
  verifyByte(dev, 'i');
  script.magic = readFixnum(dev);

  /* Read name */
  script.name = UTF8ToQString(readRubyString(dev));

  /* Read script data */
  QByteArray data = readRubyString(dev);
  data = decompressData(data);
  script.data = UTF8ToQString(data);

  /* Convert line endings: Windows -> Unix */
  script.data.remove(QChar('\r'), Qt::CaseSensitive);

  return script;
}

/* Reads and verifies the Marshal header */
static void verifyHeader(QIODevice &dev)
{
  const char *error = "Bad marshal header";

  verifyByte(dev, 4, error);
  verifyByte(dev, 8, error);
}

static void writeByte(QIODevice &dev, char byte)
{
  int count = dev.write(&byte, 1);

  if (count < 1)
    throw QByteArray("Error writing data");
}

static void writeFixnum(QIODevice &dev, int value)
{
  if (value == 0) {
    writeByte(dev, 0);
    return;
  }
  else if (value > 0 && value < 123) {
    writeByte(dev, (char) value + 5);
    return;
  }
  else if (value < 0 && value > -124) {
    writeByte(dev, (char) value - 5);
    return;
  }

  char len;

  if (value > 0) {
    /* Positive number */
    if (value <= 0x7F)
      len = 1;
    else if (value <= 0x7FFF)
      len = 2;
    else if (value <= 0x7FFFFF)
      len = 3;
    else
      len = 4;
  }
  else {
    /* Negative number */
    if (value >= (int) 0x80)
      len = -1;
    else if (value >= (int) 0x8000)
      len = -2;
    else if (value <= (int) 0x800000)
      len = -3;
    else
      len = -4;
  }

  /* Write length */
  writeByte(dev, len);

  /* Write bytes */
  if (len >= 1 || len <= -1)
    writeByte(dev, (value & 0x000000FF) >> 0x00);
  if (len >= 2 || len <= -2)
    writeByte(dev, (value & 0x0000FF00) >> 0x08);
  if (len >= 3 || len <= -3)
    writeByte(dev, (value & 0x00FF0000) >> 0x10);
  if (len == 4 || len == -4)
    writeByte(dev, (value & 0xFF000000) >> 0x18);
}

static void writeString(QIODevice &dev, const QByteArray &data)
{
  writeFixnum(dev, data.size());
  if (dev.write(data) < data.size())
    throw QByteArray("Error writing data");
}

static void writeIVARString(QIODevice &dev, const QByteArray &data)
{
  /* Write inner string */
  writeByte(dev, '"');
  writeString(dev, data);

  /* Write IVAR count */
  writeFixnum(dev, 1);
  // XXX It's no big deal, but maybe we should symlink all
  // further references to ':E' as Ruby would do?
  writeByte(dev, ':');
  writeString(dev, "E");
  /* Always write Utf8 encoding */
  writeByte(dev, 'T');
}

static void writeRubyString(QIODevice &dev, const QByteArray &data,
                            Script::Format format)
{
  if (format == Script::XP) {
    writeByte(dev, '"');
    writeString(dev, data);
  }
  else { /* format == ScriptArchive::VXAce */
    writeByte(dev, 'I');
    writeIVARString(dev, data);
  }
}

static void writeScript(QIODevice &dev, const Script &script,
                        Script::Format format)
{
  /* Write array prologue */
  writeByte(dev, '[');
  writeFixnum(dev, 3);

  /* Write magic */
  writeByte(dev, 'i');
  writeFixnum(dev, script.magic);

  /* Write name */
  writeRubyString(dev, script.name.toUtf8(), format);

  /* Convert line endings: Unix -> Windows (for compat) */
  QString sdata;
  sdata.reserve(script.data.size());

  for (size_t i = 0; i < script.data.size(); ++i) {
    QChar c = script.data.at(i);

    if (c == QChar('\n')) {
      sdata.append(QChar('\r'));
    }

    sdata.append(c);
  }

  /* Write script data */
  QByteArray data = sdata.toUtf8();
  data = compressData(data);
  writeRubyString(dev, data, format);
}

ScriptList readScripts(QIODevice &dev)
{
  verifyHeader(dev);

  /* Read array prologue */
  verifyByte(dev, '[');
  int scriptCount = readFixnum(dev);

  /* Read scripts */
  ScriptList scripts;

  try {
    for (int i = 0; i < scriptCount; ++i) {
      scripts.append(readScript(dev));
    }
  }
  catch (const QByteArray &error) {
    throw error;
  }

  return scripts;
}

void writeScripts(const ScriptList &scripts,
                  QIODevice &dev, Script::Format format)
{
  /* Write header */
  writeByte(dev, 4);
  writeByte(dev, 8);

  /* Write array prologue */
  writeByte(dev, '[');
  writeFixnum(dev, scripts.count());

  /* Write scripts */
  for (int i = 0; i < scripts.count(); ++i)
    writeScript(dev, scripts[i], format);
}

int generateMagic(const ScriptList &scripts)
{
  QSet<int> ids;
  for (size_t i = 0; i < scripts.size(); ++i)
    ids.insert(scripts[i].magic);

  int result;

  /* Generate a value that's not yet taken */
  do {
    result = qrand();
  } while (ids.contains(result) || result < 0);

  return result;
}
