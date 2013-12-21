#include "ruby_data.hxx"
#include "qt/main_window.hxx"

#include <QtGui/QApplication>
#include <QtCore/QTextCodec>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

  QString initial_path;
  if(argc >= 2) {
    initial_path = QString::fromUtf8(argv[1]);
  }

  RGSS_MainWindow main(initial_path);
  main.show();

  main.activateWindow();
  main.raise();

  return app.exec();
}
