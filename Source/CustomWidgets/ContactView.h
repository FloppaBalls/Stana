
#ifndef CONTACTVIEW_H
#define CONTACTVIEW_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include "ProfilePicLabel.h"
#include "CustomStyledWidgets/CustomButton.h"
#include "CustomStyledWidgets/SelectableButton.h"
#include "Network/AccountInfoStorage.h"
#include <QMenu>
#include <QStackedWidget>
#include "StyleBase/ImagePainter.h"


class ContactView : public QWidget{
    Q_OBJECT
public:
    ContactView(QWidget* parent = nullptr);
    void setContactInfo(ContactInfo& info);
    void setContactInfo(AccountInfoStorage& info);
    void setProfilePicture(QPixmap pixmap);
    void setName(QString name);
    void setNameFont(const QFont& font);

    QString name() const;
    QPixmap pixmap() const;

    void setNameVisible(bool visible);
    bool operator==(const ContactView& rhs);
    const ContactInfo* contactInfo() const;
    void setStatusVisibility(bool enable);
private slots:
    void emitRemove(int id);
    void onInfoDeletion();
signals:
    void remove(int id);
    void moved(int id);
protected:
    const ContactInfo* pInfo;
    QHBoxLayout* pLayout;
    ProfilePicLabel*      pProfilePicture;
    QLabel*      pName;
    const QSize iconSize = {32 , 32};
};

#endif // CONTACTVIEW_H
