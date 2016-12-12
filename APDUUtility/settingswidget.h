//! \file settingsWidget.h
//! \brief Header file for settings class.
#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QtWidgets/QWidget>
#include "nativescard.h"
#include "ui_settingsWidget.h"

//! \class settingsWidget
//! \brief Settings widget class.
class settingsWidget : public QWidget
{
 Q_OBJECT
public:
 //!\brief Constructor
 //!\param[in] parent Parent widget, default is zero.
 settingsWidget(QWidget *parent = 0);
 //! \brief Destructor
 ~settingsWidget();
protected:
 //! \fn settingsWidget::closeEvent(QCloseEvent *event)
 //! \brief close main window event function
 //! \details Save settings on widget close. 
 //! \param[in] event pointer to QCloseEvent class, ignored.
 void closeEvent(QCloseEvent *event);
private slots:
 //! \fn void settingsWidget::reloadButtonClicked(void)
 //! \brief Reloads readers list.
 void reloadButtonClicked(void);
 //! \fn void settingsWidget::closeButtonClicked(void)
 //! \brief Close button clicked function. Save settings.
 void closeButtonClicked(void);
 //! \fn void settingsWidget::defaultReaderComboBoxTextChanged(const QString& text)
 //! \brief Provide default reader name combo box changed. Set the selected reader in line edit.
 void defaultReaderComboBoxTextChanged(const QString& text);
private:
 Ui_settingsWidget ui;//!< Qt inner ui-class
 QScopedPointer<Smartcards::WinSCard> cardIface{ new Smartcards::WinSCard };//!< Scoped pointer to Smart Card Interface
};


#endif
