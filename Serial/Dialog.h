#pragma once

#include <QtWidgets/QDialog>

class SerialPort;

class Dialog : public QDialog
{
public:
	Dialog();
	virtual ~Dialog();

protected:
	void initialize();

public:
	bool loadOption();
	bool saveOption();

protected:
	QString lastFileDialogPath;

private:
	bool eventFilter(QObject* object, QEvent* event) override;
	void closeEvent(QCloseEvent* event) override;

private:
	QVBoxLayout* mainLayout;
	QHBoxLayout* helperLayout;
	QHBoxLayout* serialLayout;

	std::vector<SerialPort*> serialManagers;

};
