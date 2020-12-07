/*
 * \brief   Qt-based virtual keyboard
 * \author  Christian Prochaska
 * \date    2020-12-07
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <QQuickView>
#include <QQuickWidget>
#include <QQuickItem>
#include <QApplication>
#include <QDebug>

static constexpr bool debug = false;

class Im_enabled_item : public QQuickItem
{
	Q_OBJECT

	private:

		QFile       _event_file;
		QTextStream _event_stream;

	protected:

		QVariant inputMethodQuery(Qt::InputMethodQuery query) const override
		{
			/*
			 * Without preferring lower case, the shift key will be
			 * pushed at startup.
			 */

			if (query == Qt::ImHints)
				return Qt::ImhPreferLowercase;

			return QQuickItem::inputMethodQuery(query);
		}

		void focusInEvent(QFocusEvent *event) override
		{
			QQuickItem::focusInEvent(event);

			/*
			 * This is needed to get the 'Qt::ImhPreferLowercase'
			 * flag applied (seen in QQuickTextInput).
			 */

			qGuiApp->inputMethod()->show();
		}


		void keyReleaseEvent(QKeyEvent *e) override
		{
			if (debug)
				qDebug() << "Im_enabled_item::keyReleaseEvent(): " << e;

			if (e->text() != "") {
				_event_stream << e->text();
				_event_stream.flush();
			} else if (e->key() == Qt::Key_Backspace) {
				_event_stream << QString("\x08");
				_event_stream.flush();
			}
		}

	public:

		Im_enabled_item(QQuickItem *parent = nullptr)
		: QQuickItem(parent),
		  _event_file("/event/text"),
		  _event_stream(&_event_file)
		{
			setFlag(ItemAcceptsInputMethod, true);

			if (!_event_file.open(QIODevice::WriteOnly))
				qFatal("ERROR: could not open event VFS file");

			_event_stream.setCodec("UTF-8");
		}

};


int main(int argc, char *argv[])
{
	qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

	QApplication app(argc, argv);

	qmlRegisterType<Im_enabled_item>("Im_enabled_item", 1, 0, "Im_enabled_item");

	QQuickWidget view(QString("qrc:/virtualkeyboard.qml"));

	view.setResizeMode(QQuickWidget::SizeRootObjectToView);

	view.resize(1024, 400);

	view.show();
	
	/*
	 * Trigger a virtual keyboard shift handler reset to enable the shift
	 * key.
	 */
	view.setFocus();
	view.activateWindow();

	return app.exec();
}

#include "main.moc"
