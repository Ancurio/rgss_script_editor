
#include "ruby_data.hxx"

#include <QtCore/QFileInfo>
#include <QtWidgets/QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>

#include <iostream>

using namespace std;

int saveScripts(QString const& file, const ScriptList &scripts) {
  /* Determine marshal format */
  QFileInfo finfo(file);
  QString fileExt = finfo.suffix();
  Script::Format format;

  if (fileExt == "rxdata")
    format = Script::XP;
  else if (fileExt == "rvdata")
    format = Script::XP;
  else if (fileExt == "rvdata2")
    format = Script::VXAce;
  else {
    cerr << "Unrecognized file extension: " << fileExt.toUtf8().constData() << endl;
    return 1;
  }

  QFile archiveFile(file);
  if (!archiveFile.open(QFile::WriteOnly)) {
    cerr << "Cannot open for writing: " << file.toUtf8().constData() << endl;
    return 1;
  }

  try {
    writeScripts(scripts, archiveFile, format);
    archiveFile.close();
  }
  catch (const QByteArray &) {
    cerr << "Cannot save: " << file.toUtf8().constData() << endl;
    return 1;
  }

  return 0;
}

int import(QString src_folder, ScriptList &scripts) {
	if (src_folder.isEmpty())
		return 1;

	/* Open index */
	QFile indFile(src_folder + "/index");
	if (!indFile.open(QFile::ReadOnly)) {
		cerr << "Cannot open index file" << endl;
		return 1;
	}

	QTextStream indStream(&indFile);
	int scIdx = 0;

	while (!indStream.atEnd())
	{
		QString scName = indStream.readLine();

		QString scFilename = QString("%1").arg(scIdx, 3, 10, QLatin1Char('0'));
		QFile scFile(src_folder + "/" + scFilename);

		if (!scFile.open(QFile::ReadOnly)) {
			cerr << QString("Cannot open script \"%1\" (%2)").arg(scName, scFilename).toUtf8().constData() << endl;
			return 1;
		}

		QByteArray scData = scFile.readAll();
		scFile.close();

		Script script;
		script.magic = 0;
		script.name = scName;
		script.data = scData;

		scripts.append(script);

		++scIdx;
	}

	return 0;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		puts("Wrong number of arguments (cli <input directory> <output filename>)");
		return 1;
	}
	const QString src_folder = QString::fromLocal8Bit(argv[1]);
	const QString out_file = QString::fromLocal8Bit(argv[2]);
	ScriptList scripts;
	if (import(src_folder, scripts)) return 1;
	if (saveScripts(out_file, scripts)) return 1;
	return 0;
}
