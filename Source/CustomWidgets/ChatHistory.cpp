#include "ChatHistory.h"
#include "StyleBase/StyleRepository.h"

static QDate now = QDate::currentDate();

QSize  MessageEdit::sizeHint() const
{
    //return QSize(CustomTextEdit::sizeHint().width() , document().height;
    return QSize();
}

ChatRecord::ChatRecord(QWidget* parent) : QWidget(parent) {
    setupUi(false);
}

ChatRecord::ChatRecord(QWidget* parent, MessageInfo* pInfo0, bool waitingForResponse) : QWidget(parent)
{
    setupUi(waitingForResponse);
    setInfo(pInfo0);
    connect(pInfo0, &MessageInfo::outOfQueue, this, [=]() {
        pMainLayout->setStretch(0, 0);
        });
}
void ChatRecord::setupUi(bool waitingForResponse)
{
    QMargins noMargins = QMargins(0, 0, 0, 0);
    QFont chatFont = StyleRepository::Base::chatFont();

    pProfilePicture = new ProfilePicLabel;
    pProfilePicture->setPixmap1(StyleRepository::Base::defaultPfpPixmap());
    QSizePolicy policy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    policy.setRetainSizeWhenHidden(true);
    pProfilePicture->setSizePolicy(policy);

    pTime = new QLabel;
    pTime->setFont(StyleRepository::Base::informativeTextFont());

    pName = new QLabel;
    pName->setFont(chatFont);
    pName->setContentsMargins(noMargins);

    pNameAndTimeLayout = new QHBoxLayout;
    pNameAndTimeLayout->addWidget(pName);
    pNameAndTimeLayout->addWidget(pTime);
    pNameAndTimeLayout->addStretch(1);
    pNameAndTimeLayout->setContentsMargins(noMargins);

    pMessage = new CustomTextEdit;
    pMessage->setFont(chatFont);
    pMessage->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    pMessage->setReadOnly(true);
    pMessage->setShrinkToFit(true);
    pMessage->setObjectName("Message");
    pMessage->setContentsMargins(noMargins);
    pMessage->document()->setDocumentMargin(0);
    pMessage->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    pUILayout = new QVBoxLayout;
    pUILayout->addLayout(pNameAndTimeLayout);
    pUILayout->addWidget(pMessage);
    pUILayout->addStretch(1);
    pUILayout->setContentsMargins(noMargins);

    pMainLayout = new QHBoxLayout(this);
    pMainLayout->addWidget(pProfilePicture, 0, Qt::AlignTop);
    pMainLayout->addLayout(pUILayout);
    pMainLayout->setContentsMargins(noMargins);

    setContentsMargins(noMargins);
}

void ChatRecord::setStatus(bool isFirstMessageOfSequence)
{
    pName->setVisible(isFirstMessageOfSequence);
    pTime->setVisible(isFirstMessageOfSequence);
    pProfilePicture->setVisible(isFirstMessageOfSequence);
}

void ChatRecord::setInfo(MessageInfo* info)
{

    pInfo = info;
    connect(info, &ChatInfo::destroyed, this, [=]() {
        pInfo = &NullInfo::instance().nullMessage();
        });
    pMessage->setText(info->text());
    pName->setText(info->name());

    QDate date = pInfo->timestamp().date();

    if (now.year() == date.year() && now.month() == date.month())
    {
        if (now.day() == date.day())
            pTime->setText("Today at " + info->timestamp().time().toString());
        else if (now.day() - date.day() == 1)
            pTime->setText("Yesterday at " + info->timestamp().time().toString());
        else
            pTime->setText(pInfo->timestamp().toString());
    }
    else
        pTime->setText(pInfo->timestamp().toString());
}

ChatScrollArea::ChatScrollArea(QWidget* parent) : QScrollArea(parent)
{
    _moveScrollBar = false;
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    horizontalScrollBar()->setEnabled(false);
    QScrollBar* verticalScrollBar = QScrollArea::verticalScrollBar();

    setObjectName("ChatScrollArea");

    oldMin = verticalScrollBar->minimum(); 
    oldMax = verticalScrollBar->maximum();

    connect(verticalScrollBar, &QScrollBar::valueChanged, this, [=](int value) {
        if (value < verticalScrollBar->maximum() * 0.25f)
        emit loadMore();
    });
    connect(verticalScrollBar, &QScrollBar::rangeChanged, this, [=](int min , int max) {
        Q_UNUSED(min)
        if (_moveScrollBar == true)
        {
            verticalScrollBar->setValue(max);
            _moveScrollBar = false;
        }
        });

    connect(verticalScrollBar, &QScrollBar::rangeChanged, this, [=](int min , int max) {
       if (verticalScrollBar->value() != verticalScrollBar->maximum())
       {
           verticalScrollBar->setValue(max - oldMax + verticalScrollBar->value());
           oldMax = max;
       }
        qDebug() << "new max " << max << ' ' << "Page step" << verticalScrollBar->pageStep();
       });
    setStyleSheet(StyleRepository::Base::qSliderStyle());
    
}
void ChatScrollArea::moveScrollBarToBottom()
{
    _moveScrollBar = true;
}

void ChatScrollArea::setScrollAreaWidget(QWidget* widget) { pScrollAreaWidget = widget; }

void ChatScrollArea::resizeEvent(QResizeEvent* event)
{
    //setMinimumWidth(widget()->minimumSizeHint().width() + verticalScrollBar()->width());
    QScrollArea::resizeEvent(event);
}


void ChatRecord::setMessage(const QString& str) { pMessage->setText(str); }
void ChatRecord::setName(const QString& name) { pName->setText(name + ':'); }
QString ChatRecord::message() const { return pMessage->toPlainText(); }
QString ChatRecord::name() const { return pName->text(); }

ChatHistory::ChatHistory(QWidget* parent) : QWidget(parent)
{
    lastMessageLoadedIndex = 0;
    setupUi();
  
    connect(pScrollArea, &ChatScrollArea::loadMore, this, &ChatHistory::onLoadMore);
}
void ChatHistory::setupUi()
{
    const int recordSpacing = 0;
    pScrollAreaWidget = new QWidget;
    pScrollAreaWidget->setObjectName("ListContainer");
    pHistoryLayout = new QVBoxLayout(pScrollAreaWidget);
    pHistoryLayout->addStretch(1);
    pHistoryLayout->setSpacing(recordSpacing);

    pScrollArea = new ChatScrollArea();
    pScrollArea->setWidget(pScrollAreaWidget);
    pScrollArea->setScrollAreaWidget(pScrollAreaWidget);
    pScrollArea->setWidgetResizable(true);
    QHBoxLayout* pMainLayout = new QHBoxLayout(this);
    pMainLayout->addWidget(pScrollArea);
    pInfo = nullptr;
    setObjectName("ListContainer");
}
void ChatHistory::addNRecords(int n)
{
    int lastSize = recordList.size();
    recordList.resize(lastSize + n);

    for (int i = lastSize; i < lastSize + n; i++)
    {
        recordList[i] = new ChatRecord(nullptr, &NullInfo::instance().nullMessage());
        pHistoryLayout->insertWidget(0, recordList[i]);
    }
    //std::for_each(std::begin(recordList) + lastSize, std::end(recordList) , [=](ChatRecord* record){
    //    record =  new ChatRecord(nullptr , &NullInfo::instance().nullMessage());
    //    pHistoryLayout->insertWidget(pHistoryLayout->count() - 1 , record);
    //});
}
void ChatHistory::deleteNRecords(int n)
{
    if (n == 0) return;
    std::for_each(std::end(recordList) - n, std::end(recordList), [=](ChatRecord* record) {
        record->deleteLater();
        });
    recordList.resize(recordList.size() - n);
}

QSize ChatHistory::sizeHint() const
{
    return QWidget::sizeHint();
}

QSize ChatHistory::minimumSizeHint() const {
    return QWidget::sizeHint();
}

void ChatHistory::addRecord(const QString& name, const QString& message)
{
    MessageInfo* messageInfo = pInfo->addMessageToQueue(name, message);
    ChatRecord* record = new ChatRecord(nullptr, messageInfo);

    setupMessage(*messageInfo, record , recordList.back());

    recordList.emplace_back(record);
    pHistoryLayout->insertWidget(pHistoryLayout->count() - 1, record);
    pScrollArea->moveScrollBarToBottom();
}
void ChatHistory::addRecord(MessageInfo& message)
{
    ChatRecord* record = new ChatRecord(nullptr, &message);

    setupMessage(message, record , recordList.back());

    QScrollBar* scrollBar = pScrollArea->verticalScrollBar();
    bool moveToBottom = false;

    if (scrollBar->value() == scrollBar->maximum())
        moveToBottom = true;

    recordList.emplace_back(record);
    pHistoryLayout->insertWidget(pHistoryLayout->count() - 1, record);
    if(moveToBottom)
        pScrollArea->moveScrollBarToBottom();
}
void ChatHistory::setChatInfo(ChatInfo* info) {
    if (pInfo)
    {
        disconnect(pInfo, SIGNAL(newMessageAdded(MessageInfo&)), this, SLOT(addRecord(MessageInfo&)));
    }

    pInfo = info;
    connect(pInfo, &ChatInfo::destroyed, this, [=]() {
        pInfo = &NullInfo::instance().nullChat();
        });
    connect(info, SIGNAL(newMessageAdded(MessageInfo&)), this, SLOT(addRecord(MessageInfo&)));
    std::vector<MessageInfo*> infoList = pInfo->lastNMessages(messagesLoaded);
    lastMessageLoadedIndex = std::min(messagesLoaded , (int)infoList.size());

    int recordDiff = int(infoList.size() - recordList.size());
    if (recordDiff > 0)
        addNRecords(recordDiff);
    else
        deleteNRecords(-recordDiff);


    pScrollArea->moveScrollBarToBottom();
    userHash.clear();
    sequence = 1;
    lastVal = -1;


    int& val = userHash[infoList[0]->name()];
    if (val == 0)
    {
        val = sequence;
        sequence++;
    }
    lastVal = val;
    recordList[0]->setInfo(infoList[0]);
    recordList[0]->setStatus(false);
    for (int i = 1; i < infoList.size(); i++)
    {
        setupMessage(*infoList[i], recordList[i], recordList[i - 1]);
        recordList[i]->setInfo(infoList[i]);
    }
    if (messagesLoaded != infoList.size())
        recordList.back()->setStatus(true);
}
void ChatHistory::onLoadMore()
{
    std::vector<MessageInfo*> infoList = pInfo->lastNMessages(lastMessageLoadedIndex , messagesLoaded);
    addNRecords(infoList.size());
    for (int i = lastMessageLoadedIndex; i < lastMessageLoadedIndex + infoList.size(); i++)
    {
        setupMessage(*infoList[i - lastMessageLoadedIndex], recordList[i] , recordList[i - 1]);
        recordList[i]->setInfo(infoList[i - lastMessageLoadedIndex]);
    }
    if (infoList.size() != 0)
    {
        //pScrollArea->moveScrollBarToBottom();
        lastMessageLoadedIndex += infoList.size();
    }
    else
    {
        recordList.back()->setStatus(true);
    }
}
void ChatHistory::setupMessage(MessageInfo& info, ChatRecord* record ,ChatRecord* previous)
{
    int& val = userHash[info.name()];
    if (val == 0)
    {
        val = sequence;
        sequence++;
    }

    if (val == lastVal)
    {
        record->setStatus(false);
        QMargins margins = QMargins(0, 0, 0, 0);
        record->setContentsMargins(margins);
    }
    else
    {
        previous->setStatus(true);
        lastVal = val;

        QMargins margins = QMargins(0, 30, 0, 0);
        previous->setContentsMargins(margins);
    }
    record->setInfo(&info);
}

void ChatHistory::empty()
{
    std::for_each(std::begin(recordList), std::end(recordList), [=](ChatRecord* record) {
        record->deleteLater();
        });
    recordList.resize(0);
}
int ChatHistory::chatId() const { return pInfo->id(); }

const ChatRecord& ChatHistory::recordAt(int index) { return *recordList[index]; }
