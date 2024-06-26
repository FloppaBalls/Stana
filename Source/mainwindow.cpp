
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMovie>
#include <QLabel>
#include <QFile>
#include <QMenu>
#include <QMenuBar>
#include "StyleBase/ButtonStyleRepository.h"
#include <QStyleFactory>
/*
WARNING
There is a very rare crash that occurs because of the occuring background and it's related to scaling I think
createDIB: CreateDIBSection failed (1920x16777215, format: 6)

Explanation: So when I create a bitmap that large, sometimes there might not be enough memory to scale it all
Solution that needs to be implemented: When inserting the background image , split it into chunks everytime it becomes bigger that 900x600 for example
*/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSize screenSize = { 1920 , 1080};
    resize(screenSize);
    setWindowState(Qt::WindowMaximized);

    QFile styleFile = QFile(":/styles/style.qss");
    if(styleFile.open(QFile::ReadOnly))
        qDebug() << "File opened";
    else
        qDebug() << "File didn't open";

    QString styleSheet = QLatin1String(styleFile.readAll());
    styleFile.close();
    setStyleSheet(styleSheet);

    pEffect = new BackgroundShiftOnMouseEffect(this, QPixmap(":/Images/Images/Background/GreenSpace.png"), 3);
    pEffect->setGeometry(QRect(QPoint(0 , 0) , screenSize));
    pServerInfoProcessor = new ConnectionHandler(this);
    installEventFilter(this);

    pSelectorWidget = new UserSelectorWidget(nullptr, *pServerInfoProcessor);
    QSize selectorWidgetSize = QSize(pSelectorWidget->sizeHint().width(), 600);
    pSelectorWidget->setGeometry(QRect(QPoint(), selectorWidgetSize));
    pSelectorWidget->setFixedHeight(selectorWidgetSize.height());

    KeywordCombo* pKeywordShower = new KeywordCombo;
    pKeywordShower->setVisible(false);
    pSelectorWidget->setVisible(false);

    pPageManager = new PageManager(nullptr , *pServerInfoProcessor , *pSelectorWidget , *pKeywordShower);
    setCentralWidget(pPageManager);
  
    //set the parent here and not at constructor so that it will be drawn over all widgets
    pSelectorWidget->setParent(this);
    pKeywordShower->setParent(this);

    connect(pPageManager , &PageManager::pageChanged , pEffect , &BackgroundShiftOnMouseEffect::updateAccordingToPage);
    
    const auto& styles = QStyleFactory::keys();
    for (const auto& s : styles)
    {
        qDebug() << s;
    }
    //QLabel* label = new QLabel(this);
    //label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    //label->setWordWrap(true);
    //label->setGeometry(rect());
    //label->setFont(QFont("Times", 25));
    //label->setText("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    //    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    //    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    //    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    //    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    //    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    //    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    //    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    //    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    //    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    //qDebug() << QImageReader::supportedImageFormats();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent* ev) {

/*    int _scale = 2;
    QSize chunkSize = {128 , 120};
    QPixmap _originalBackground(":/images/GreenSpace.png");
    QPainter painter(this);

    int w = _originalBackground.width();
    int h = _originalBackground.height();


    int remainsX = w % chunkSize.width();
    int remainsY = h % chunkSize.height();
    //matrix width
    int _bgWidth = w / chunkSize.width() + bool(remainsX);
    //matrix height
    int _bgHeight = h / chunkSize.height() + bool(remainsY);

    std::vector<QPixmap> _backgroundMatrix(_bgHeight * _bgWidth);

    unsigned deltaY ;
    for(int y = 0 ; y < _bgHeight - 1; y += 1)
    {
        deltaY = y * _bgWidth;
        for(int x = 0 ; x < _bgWidth - 1; x += 1)
        {
            _backgroundMatrix[deltaY + x] =
                _originalBackground.copy(
                                       QRect(QPoint(x * chunkSize.width() ,y *chunkSize.height()) , chunkSize)
                                       ).scaled(chunkSize * _scale);
        }
    }

    //going through the last row
    int lastRowHeight = (remainsY) ? chunkSize.height() : remainsY;
    deltaY = (_bgHeight - 1) * _bgWidth;
    for(int x = 0 ; x < _bgWidth -1 ; x++)
    {
        QRect s = QRect(x * chunkSize.width() , (_bgHeight - 1) *chunkSize.height() , chunkSize.width() , lastRowHeight);
        _backgroundMatrix[deltaY + x] =
            _originalBackground.copy( s ).scaled(s.size() * _scale);
    }


    //going through the last column
    int lastRowWidth = (remainsX) ? chunkSize.width() : remainsX;
    for(int y = 0 ; y < _bgHeight -1 ; y++)
    {
        QRect s = QRect((_bgWidth - 1) * chunkSize.width() , y *chunkSize.height() , lastRowWidth ,  chunkSize.height());
        _backgroundMatrix[y * _bgWidth + (_bgWidth - 1)] =
            _originalBackground.copy( s ).scaled(s.size() * _scale);
    }
    _backgroundMatrix.back() = _originalBackground.copy(QRect(QPoint((_bgWidth -1) * chunkSize.width() , (_bgHeight - 1) * chunkSize.height()) , QSize(lastRowWidth , lastRowHeight)));

    chunkSize *= _scale;
    QPoint point;

    for(int y = 0 ; y < _bgHeight ; y++)
    {
        for(int x = 0 ; x < _bgWidth ; x++)
        {
                        point.setX(chunkSize.width() * x);
            painter.drawPixmap(point , _backgroundMatrix[y * _bgWidth + x]);
        }
        point.setY(chunkSize.height() * y);
    }*/

    QMainWindow::paintEvent(ev);
}

void MainWindow::mousePressEvent(QMouseEvent* ev)
{
    QMainWindow::mousePressEvent(ev);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* ev)
{
    if (ev->type() == QEvent::MouseButtonPress)
    {
        QPoint clickPos = static_cast<QMouseEvent*>(ev)->pos();
        QRect rect = QRect(pSelectorWidget->pos(), pSelectorWidget->size());
    
        if (rect.contains(clickPos) == false)
            pSelectorWidget->flip(false);
    }
    return QObject::eventFilter(obj, ev);
}
