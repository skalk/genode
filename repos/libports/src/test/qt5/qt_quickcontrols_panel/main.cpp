/*
 * \brief   QtQuick Controls2 Panel demo
 * \author  Stefan Kalkowski
 * \date    2020-12-14
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickView>
#include <QtQml>


class FileReport : public QObject
{
	Q_OBJECT

	public:

		Q_PROPERTY(QString target
		           WRITE   setTarget)

		explicit FileReport(QObject *parent = 0)
		: QObject(parent) {}

		Q_INVOKABLE bool write(const QString& data);

		QString target() { return _target; };

	public slots:

			void setTarget(const QString & target) {
				_target = target; };

	signals:

		void error(const QString & msg);

	private:

		QString _target;
};


bool FileReport::write(const QString& data)
{
	if (_target.isEmpty())
		return false;

	QFile file(_target);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return false;

	QTextStream out(&file);
	out << data;

	file.close();
	return true;
}


int main(int argc, char *argv[])
{
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
	QQmlApplicationEngine engine;
	qmlRegisterType<FileReport>("LocalIO", 1, 0, "FileReport");
	engine.load(QUrl("qrc:/qt_quickcontrols_panel.qml"));

    return app.exec();
}

#include "main.moc"
