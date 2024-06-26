
#ifndef MAINPAGE_H
#define MAINPAGE_H
#include "Page.h"
#include <QHBoxLayout>
#include "CustomWidgets/OptionSection.h"
#include "Subpages/ChatPage.h"
#include "CustomWidgets/UserSelectorWidget.h"

class MainPage : public Page{
    Q_OBJECT
public:
    enum class SubPage{
      Chat,
      Friends
    };
public:
    MainPage(QWidget* parent , ConnectionHandler& processor , UserSelectorWidget& userSelectorWidget, KeywordCombo& keywordCombo);
private:
    void setupUi(UserSelectorWidget& userSelectorWidget, KeywordCombo& keywordCombo);
private:
    QHBoxLayout*        pLayout;
    OptionSectionFrame* pFrame;
    SubpageManager*     pManager;
};

#endif // MAINPAGE_H
