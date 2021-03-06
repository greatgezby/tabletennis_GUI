#include "mainwindow.h"
#include "pingpongtablearea.h"

#include <cv.h>
#include <highgui.h>

#include "Codebook.h"
#include "Reconstruct.h"
#include "NNTracker.h"

#include "config.h"
#include "tools.h"

using namespace cv;

const int MainWindow::SCALE = 2;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    //ui(new Ui::MainWindow),
    trackLeft(NULL),
    trackRight(NULL),
    reconstruct(NULL)
{
    if (objectName().isEmpty())
        setObjectName(QStringLiteral("MainWindow"));
    resize(1012, 711);
    centralWidget = new QWidget(this);
    centralWidget->setObjectName(QStringLiteral("centralWidget"));
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
    centralWidget->setSizePolicy(sizePolicy);
    widget = new QWidget(centralWidget);
    widget->setObjectName(QStringLiteral("widget"));
    widget->setGeometry(QRect(10, 10, 992, 671));
    verticalLayout = new QVBoxLayout(widget);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setSizeConstraint(QLayout::SetMaximumSize);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(10);
    horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
    horizontalLayout->setSizeConstraint(QLayout::SetMinimumSize);
    horizontalLayout->setContentsMargins(10, 10, 10, 10);
    label = new QLabel(widget);
    label->setObjectName(QStringLiteral("label"));
    QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(sizePolicy1);
    label->setMinimumSize(QSize(480, 270));
    QPalette palette;
    QBrush brush(QColor(136, 138, 133, 255));
    brush.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active, QPalette::Base, brush);
    palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
    QBrush brush1(QColor(239, 235, 231, 255));
    brush1.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
    label->setPalette(palette);

    horizontalLayout->addWidget(label);

    label_2 = new QLabel(widget);
    label_2->setObjectName(QStringLiteral("label_2"));
    sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
    label_2->setSizePolicy(sizePolicy1);
    label_2->setMinimumSize(QSize(480, 270));
    QPalette palette1;
    palette1.setBrush(QPalette::Active, QPalette::Base, brush);
    palette1.setBrush(QPalette::Inactive, QPalette::Base, brush);
    palette1.setBrush(QPalette::Disabled, QPalette::Base, brush1);
    label_2->setPalette(palette1);

    horizontalLayout->addWidget(label_2);


    verticalLayout->addLayout(horizontalLayout);

    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setSpacing(6);
    horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
    horizontalLayout_2->setSizeConstraint(QLayout::SetMinimumSize);
    playButton = new QPushButton(widget);
    playButton->setObjectName(QStringLiteral("playButton"));
    connect(playButton, SIGNAL(clicked(bool)),
                this, SLOT(on_playButton_clicked()));

    horizontalLayout_2->addWidget(playButton);

    pauseButton = new QPushButton(widget);
    pauseButton->setObjectName(QStringLiteral("pauseButton"));
    connect(pauseButton, SIGNAL(clicked(bool)),
                this, SLOT(on_pauseButton_clicked()));

    horizontalLayout_2->addWidget(pauseButton);


    verticalLayout->addLayout(horizontalLayout_2);

    horizontalLayout_4 = new QHBoxLayout();
    horizontalLayout_4->setSpacing(6);
    horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));

    verticalLayout->addLayout(horizontalLayout_4);
    table = new PingPongTableArea;
    horizontalLayout_4->addWidget(table);

    setCentralWidget(centralWidget);
    menuBar = new QMenuBar(this);
    menuBar->setObjectName(QStringLiteral("menuBar"));
    menuBar->setGeometry(QRect(0, 0, 1012, 22));
    setMenuBar(menuBar);
    mainToolBar = new QToolBar(this);
    mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
    addToolBar(Qt::TopToolBarArea, mainToolBar);
    statusBar = new QStatusBar(this);
    statusBar->setObjectName(QStringLiteral("statusBar"));
    setStatusBar(statusBar);

    retranslateUi();

    label->setScaledContents(true);
    label_2->setScaledContents(true);

    firstPoint = true;
}

void MainWindow::retranslateUi(){
    setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
    label->setText(QApplication::translate("MainWindow", "TextLabel", Q_NULLPTR));
    label_2->setText(QApplication::translate("MainWindow", "TextLabel", Q_NULLPTR));
    playButton->setText(QApplication::translate("MainWindow", "Play", Q_NULLPTR));
    pauseButton->setText(QApplication::translate("MainWindow", "Pause", Q_NULLPTR));
}

MainWindow::~MainWindow()
{
    //delete ui;
    capLeft.release();
    capRight.release();
    //if(trackLeft!=NULL)delete trackLeft;
    //if(trackRight!=NULL)delete trackRight;
}

void MainWindow::loadCameraMatrices(const string camera_matrix_path){
    reconstruct = new Reconstruct(camera_matrix_path);
}

void MainWindow::openVideo(const char *leftFilename, const char *rightFilename){
    if(leftFilename==NULL||rightFilename==NULL){
        capLeft.open(0);
        capRight.open(1);
    }else{
        printf("%s %s", leftFilename, rightFilename);
        capLeft.open(leftFilename);
        capRight.open(rightFilename);
    }
    capLeft>>frameLeft;
    capRight>>frameRight;
    if(frameLeft.empty()||frameRight.empty()){
        return;
    }
    Size szSmall = frameLeft.size();
    szSmall.width = szSmall.width/SCALE;
    szSmall.height = szSmall.height/SCALE;
    temp = Mat(szSmall, frameLeft.type());
    temp2 = Mat(szSmall, frameLeft.type());
    bgLeft = new BgSubtractor(szSmall, TRAIN_BG_MODEL_ITER, 4);
    bgRight = new BgSubtractor(szSmall, TRAIN_BG_MODEL_ITER, 4);
}

void MainWindow::initTracker(Classifier &classifier){
    if(trackLeft!=NULL||trackRight!=NULL){
        printf("CNN tracker already initialized!\n");
        return;
    }
    trackLeft = new NNTracker(classifier);
    trackRight = new NNTracker(classifier);
}

void MainWindow::nextFrame(){
    capLeft>>frameLeft;
    capRight>>frameRight;
    bool found = false;
    Point left ,right;
    if(!frameLeft.empty()&&!frameRight.empty()){
        cv::resize(frameLeft, temp, temp.size());
        cvtColor(temp, temp2, CV_BGR2YCrCb);
        if(bgLeft->process(temp2)){
            float prob;
            Rect bbox = Rect(0,0,0,0);
            found = trackLeft->track(temp, (Rect*)bgLeft->bboxes, bgLeft->numComponents, &prob, &bbox);
            if(found){
                rectangle(temp, bbox, CV_RGB(0x00, 0xff, 0x00), 4);
                left = Point(bbox.x*SCALE+bbox.width*SCALE/2, bbox.y*SCALE+bbox.height*SCALE/2);
            }
        }
        imageLeft = Mat2QImage(temp);
        label->setPixmap(QPixmap::fromImage(imageLeft));

        cv::resize(frameRight, temp, temp.size());
        cvtColor(temp, temp2, CV_BGR2YCrCb);
        if(bgRight->process(temp2)){
            float prob;
            Rect bbox = Rect(0,0,0,0);
            found &= trackRight->track(temp, (Rect*)bgRight->bboxes, bgRight->numComponents, &prob, &bbox);
            if(found){
                rectangle(temp, bbox, CV_RGB(0x00, 0xff, 0x00), 4);
                right = Point(bbox.x*SCALE+bbox.width*SCALE/2, bbox.y*SCALE+bbox.height*SCALE/2);
            }
        }
        imageRight = Mat2QImage(temp);
        label_2->setPixmap(QPixmap::fromImage(imageRight));
        if(found){
            CvPoint3D32f coord_world = reconstruct->uv2xyz(left, right);
            printf("%f, %f, %f\n", coord_world.x, coord_world.y, coord_world.z);
            table->setCurrentPoint(coord_world);
            if(firstPoint){
                firstPoint = false;
                lastPoint = coord_world;
                velocityZ = 0;
            }else{
                float velocityZ_new = coord_world.z - lastPoint.z;
                if(velocityZ<0&&velocityZ_new>0){
                    //is rebound
                    table->setLandingPoint(lastPoint);
                }
                lastPoint = coord_world;
                velocityZ = velocityZ_new;
            }
        }
    }
}

void MainWindow::on_playButton_clicked()
{
    int rate = capLeft.get(CV_CAP_PROP_FPS);
    capLeft>>frameLeft;
    capRight>>frameRight;
    if(!frameLeft.empty()&&!frameRight.empty()){
        imageLeft = Mat2QImage(frameLeft);
        imageRight = Mat2QImage(frameRight);
        //ui->label->setPixmap(QPixmap::fromImage(imageLeft));
        //ui->label_2->setPixmap(QPixmap::fromImage(imageRight));
        label->setPixmap(QPixmap::fromImage(imageLeft));
        label_2->setPixmap(QPixmap::fromImage(imageRight));
        timer = new QTimer(this);
        //timer->setInterval(1000/rate);
        connect(timer, SIGNAL(timeout()), this, SLOT(nextFrame()));
        timer->start();
    }
}

void MainWindow::on_pauseButton_clicked()
{
    if(timer){
        timer->stop();
    }
}
