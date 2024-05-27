
#ifndef USERINFO_H
#define USERINFO_H
#include <QString>
#include <QDebug>
#include <QDateTime>
#include <QTimeZone>
#include <algorithm>
#include "Tools.h"
#include <QFlags>
#include <QUrl>
#include <queue>

class MessageInfo : public QObject{
    Q_OBJECT
public:
    MessageInfo(QObject* parent = nullptr);
    MessageInfo(QObject* parent , QString name , QString text , QString timestamp);
    QString name() const;
    QString text() const;
    QDateTime timestamp() const;
    void setName(QString str);
    void setText(QString text);
    void setAt(int i , QString str);
    void setTimestamp(QString str);
    void setTimestamp(QDateTime str);
    static const QString imageSign;
signals:
    void outOfQueue();
private:
    QString _name;
    QString _text;
    QDateTime _timestamp;
};

class ContactInfo : public QObject{
    Q_OBJECT
public:
    enum class Status {
        Null = 0 ,
        Online = 1, 
        HasRequest = 2,
        IsBlocked = 4,
        HasBlockedYou = 8,
        Friend = 16
    };
    Q_DECLARE_FLAGS(ContactStatus, Status)
public:
    ContactInfo(QObject* parent = nullptr);
    ContactInfo(QObject* parent, QString name , ContactStatus flags , QString lastSeen);
    void setFlags(ContactStatus f);
    void addFlags(ContactStatus f);
    void removeFlags(ContactStatus f);
    void setName(QString name);
    void setLastSeen(QString lastSeen);
    void setFriendList(std::vector<int> friendList0);
    void setId(int id);


    QString name() const;
    QString lastSeen() const;
    const std::vector<int>& friendList() const;
    int id() const;
    ContactStatus flags() const;


    bool operator<(const ContactInfo& info);
    bool operator>(const ContactInfo& info);
    bool operator==(const ContactInfo& info);

    static std::vector<ContactInfo*> subtractFromList(std::vector<ContactInfo*> list1, std::vector<ContactInfo*> list2);

signals:
    void removed(int id);
    //void moved(int id);
    //true for blocking , false for unblocking
    void gotBlocked(bool b);
    void blockedYou(bool b);
    void statusChanged(bool online);
private:
    int _id;
    QString _name;
    QString _lastSeen;
    ContactStatus _flags;

    std::vector<int> _friendList;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ContactInfo::ContactStatus)

class ChatInfo : public QObject{
    Q_OBJECT
public:
    ChatInfo(QObject* parent = nullptr);
    void setMessageHistory(std::vector<MessageInfo*> vec);
    void setName(QString name);
    //also emits a signal that specifies that the name changed
    void setNewName(QString name);
    void setMembers(std::vector<int> members_id);
    void setMembers(const QString& str);
    void setId(int id);
    void setReadOnlyMembers(std::vector<int> members_id);
    void setReadOnlyMembers(const QString& str);
    //used when initializing
    void setAdminId(int id);
    void setNewAdmin(int id);
    void setType(bool isPrivate);
    void removeMember(int id , int userId , bool forcefullyRemoved);
    void addMember(int id);
    //the chat doesn't know who is the user of this app in the chat , so I have to give him the info
    void connectSlotsForPrivateChats(ContactInfo* info);
    

    void addMessage(MessageInfo* mes);
    MessageInfo* addMessage( QString name , QString message);
    MessageInfo* addMessageToQueue(QString name, QUrl url);
    MessageInfo* addMessageToQueue( QString name , QString message);
    MessageInfo* addWaitingMessage( QString name , QString message);

    void addMessageFromDatabase( QString name , QString message, QString timestamp);
    //returns true if it approved a new message , false otherwise
    bool approveLastInQueue();
    QString name() const;
    bool isPrivate() const;
    const std::vector<int>& members() const;
    const std::vector<int>& readOnlyMembers() const;
    const std::vector<MessageInfo*>& history() const;
    const MessageInfo& back() const;
    int id() const;
    int adminId() const;
    const std::vector<MessageInfo*> lastNMessages(int n) const;
    //start starts from the end: so if start = 5 , it is 5 indexes from the end towards the begining
    const std::vector<MessageInfo*> lastNMessages(int start , int n) const;

    bool operator<(const ChatInfo* rhs) const;
    bool operator<(ChatInfo* rhs);
    bool operator>(const ChatInfo* rhs) const;
    bool operator>(ChatInfo* rhs);
    bool operator==(const ChatInfo* rhs) const;
    bool operator==(ChatInfo* rhs);

    void mediaMessageUploaded(QString fileName);
signals:
    void newMessageAdded( MessageInfo& m );
    //this signal is connected to the function that sends message info to the server
    void newMessageInQueue(const MessageInfo& info , int chatId);
    void nameChanged(const QString& name);
    void gotBlocked(bool b);
    void blockedYou(bool b);
    void memberRemoved(int id);
    void newMember(int id);
    void removed(bool forcefully = true);
    void newAdmin(int id);
private:
    std::vector<MessageInfo*> _history;
    std::vector<MessageInfo*> _queue;
    std::queue<MessageInfo*> _mediaMessageQueue;

    std::vector<int> _members;
    std::vector<int> _readOnlyMembers;
    int _adminId;
    QString _name;
    int _id;
    bool _waitingForResponse;
    bool _private;
};


class NullInfo{
public:
    static NullInfo& instance();
    ~NullInfo();
    NullInfo(const NullInfo&) = delete;
    NullInfo& operator=(const NullInfo&) = delete;

    MessageInfo& nullMessage() const noexcept;
    ChatInfo& nullChat() const noexcept;
    ContactInfo& nullContact() const noexcept;
private:
    NullInfo();
private:
    MessageInfo* pNullMessage;
    ChatInfo* pNullChat;
    ContactInfo* pNullContact;
};

class UserInfo : public QObject{
public:
    UserInfo(QObject* parent = nullptr);
    virtual void setName(QString str);
    virtual void setPassword(QString pas);
    virtual void setEmail(QString email);

    QString name() const;
    QString password() const;
    QString email() const;
    int id() const;

    void addChat(ChatInfo* chat);
    void addFriend(ContactInfo* contact);
    void addToBlockedList(ContactInfo* contact);
    void addToRequestList(ContactInfo* contact);
    void addUserByFlag(ContactInfo* contact);
    void removeChat(QString name);

    void removeFriend(int id);

    void showChatInDebug() const noexcept;
    ChatInfo& getChatById(int id) ;
    const std::vector<ChatInfo*>& chatList() const noexcept;
    const std::vector<ChatInfo*>& requestList() const noexcept;
    std::vector<ContactInfo*>& friendList() noexcept;
    std::vector<ContactInfo*>& requestList() noexcept;
    std::vector<ContactInfo*>& blockedList() noexcept;

    void removeRequest(int id);
    /* When using this app and switching accounts , to make switching faster , I could preserve the ChatInfo objects and ContactInfo objects according to my use
     * so that I don't lose performance by reallocating again memory. I think I will do this towards the end of development */
    void clearAllData() noexcept;
    //clears all the chats , friends etc etc
    void clearAccountData() noexcept;
    void transformChats() noexcept;
    ContactInfo* findUser_KnownLists(int id) const;
    ContactInfo* findUser_UnknownList(int id) const;
    ContactInfo* findUser(int id) const;
    std::vector<ContactInfo*> findUsers(std::vector<int> list);
    //returns true if a move occurred
    void moveUserToUnknownList(ContactInfo* id);
    void moveUserToBlocked(ContactInfo* id);
    ChatInfo* privateChatById(int id);
    ChatInfo* chatById(int id);
    ChatInfo& firstChat() const;

    bool chatListEmpty() const;
    std::vector<QString> namesForContacts(std::vector<int> idList) const;
protected:
    void adaptChat(ChatInfo* info);
    void addToStrangerList(ContactInfo* info);
protected:
    QString _name;
    QString _password;
    QString _email;
    int _id;
    std::vector<ChatInfo*>   _chatList;
    std::vector<ChatInfo*>   _requestChatList;
    std::vector<ContactInfo*> _friendList;
    std::vector<ContactInfo*> _requestList;
    std::vector<ContactInfo*> _blockedList;
    //for people encountered on groups that are not your friends
    std::vector<ContactInfo*> _strangerList;

};

#endif // USERINFO_H
