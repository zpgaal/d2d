// Copyright (c) 2005-2016 Gzp
// Distributed under MIT License.

#include "mainwindow.h"
#include "qgraphwidget.h"
#include "ui_mainwindow.h"
#include "scenemanager.h"

#include <QDebug>
#include <QApplication>
#include <QTreeWidgetItem>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileDialog>

MainWindow::MainWindow( SceneManager* aSceneManager, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mSceneManager( aSceneManager )
    , mInHandleLayersVisibilityChanged( false )
{
    ui->setupUi(this);

    ui->graphView->setSceneManager( mSceneManager );
    ui->cbxBackground->addItem("Black", QVariant(QGraphWidget::BackBlack));
    ui->cbxBackground->addItem("White", QVariant(QGraphWidget::BackWhite));
    ui->cbxBackground->addItem("Globe", QVariant(QGraphWidget::BackGlobe));
    ui->cbxBackground->addItem("BackGradient", QVariant(QGraphWidget::BackGradient));

    ui->cbxSceneArea->addItem("Increasing", QVariant(SceneManager::AreaAuto));
    ui->cbxSceneArea->addItem("Area [0..1]^2", QVariant(SceneManager::Area01));
    ui->cbxSceneArea->addItem("Globe", QVariant(SceneManager::AreaGlobe));

    ui->cbxHighQuality->setChecked(true);
    handleSceneAreaChange( mSceneManager->areaPolicy() );
    handleLayersChanged();

    connect( mSceneManager, SIGNAL(onAreaPolicyChanged(int)), this, SLOT(handleSceneAreaChange(int)) );
    connect( mSceneManager, SIGNAL(onLayersVisibilityChanged(LayerInfo)), this, SLOT(handleLayersVisibilityChanged(LayerInfo)) );
    connect( mSceneManager, SIGNAL(onLayersChanged()), this, SLOT(handleLayersChanged()) );

    setWindowTitle("QtViewer");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnSaveCache1_clicked()
{
    mSceneManager->saveCache(1);
}

void MainWindow::on_btnSaveCache2_clicked()
{
    mSceneManager->saveCache(2);
}

void MainWindow::on_btnSaveCache3_clicked()
{
    mSceneManager->saveCache(3);
}

void MainWindow::on_btnSaveCache4_clicked()
{
    mSceneManager->saveCache(4);
}

void MainWindow::on_btnLoadCache1_clicked()
{
    mSceneManager->loadCache(1, !(QApplication::keyboardModifiers() & Qt::ControlModifier));
}

void MainWindow::on_btnLoadCache2_clicked()
{
    mSceneManager->loadCache(2, !(QApplication::keyboardModifiers() & Qt::ControlModifier));
}

void MainWindow::on_btnLoadCache3_clicked()
{
    mSceneManager->loadCache(3, !(QApplication::keyboardModifiers() & Qt::ControlModifier));
}

void MainWindow::on_btnLoadCache4_clicked()
{
    mSceneManager->loadCache(4, !(QApplication::keyboardModifiers() & Qt::ControlModifier) );
}

void MainWindow::on_cbxBackground_currentIndexChanged(int index)
{
    QVariant d = ui->cbxBackground->itemData(index);
    ui->graphView->setBackground( d.toInt() );
}

void MainWindow::handleBackgroundChange(int aType)
{
    int idx = ui->cbxBackground->findData( aType );
    ui->cbxBackground->setCurrentIndex(idx);
}

void MainWindow::on_cbxSceneArea_currentIndexChanged(int index)
{
    QVariant d = ui->cbxSceneArea->itemData(index);
    mSceneManager->setAreaPolicy( d.toInt() );
}

void MainWindow::handleSceneAreaChange(int aType)
{
    int idx = ui->cbxSceneArea->findData( aType );
    ui->cbxSceneArea->setCurrentIndex(idx);
}


void MainWindow::on_tblLayers_itemChanged( QTreeWidgetItem* aItem, int aColumn )
{
    if( mInHandleLayersVisibilityChanged )
        return;

    QString layer = aItem->data( 0, DataLayerKey ).toString();
    ELayerCategory cat = static_cast<ELayerCategory>( aItem->data( aColumn, DataLayerCategoryKey ).toInt() );
    bool vis = aItem->checkState(aColumn) == Qt::Checked;
    mSceneManager->setLayerVisibility( layer, cat, vis );
}

void MainWindow::handleLayersVisibilityChanged( LayerInfo aInfo )
{
    mInHandleLayersVisibilityChanged = true;

    QList<QTreeWidgetItem*> roots = ui->tblLayers->findItems( aInfo.name, Qt::MatchExactly, 0 );
    Q_ASSERT( roots.size() <= 1 );

    for( QList<QTreeWidgetItem*>::iterator it = roots.begin(); it != roots.end(); ++it )
    {
        QTreeWidgetItem* item = *it;
        item->setCheckState(1, aInfo.getAllCheckState() );
        for( int i = 0; i < LayerCategoryCount; ++i )
        {
            item->setCheckState(2+i, aInfo.visibility[i] ? Qt::Checked : Qt::Unchecked );
        }
    }

    mInHandleLayersVisibilityChanged = false;
}

void MainWindow::handleLayersChanged()
{
    // todo: don't clear update only by the change, or at least update only by a timer    
    QStringList labels;
    labels.append( "Layer" );
    labels.append( "All" );
    for( int i = 0; i < LayerCategoryCount; ++i )
    {
        labels.append( getLayerCategoryName( static_cast<ELayerCategory>(i) ) );
    }
    ui->tblLayers->clear();
    ui->tblLayers->setColumnCount( LayerCategoryCount+2 );
    ui->tblLayers->setHeaderLabels( labels );

    LayerInfoList layers = mSceneManager->getLayers();    
    for( LayerInfoList::iterator it = layers.begin(); it != layers.end(); ++it )
    {
        QTreeWidgetItem* item = new QTreeWidgetItem();

        int col = 0;
        item->setText(col, it->name );
        item->setData(col,DataLayerKey,it->name);
        ++col;

        item->setCheckState(col, it->getAllCheckState() );
        item->setData(col,DataLayerCategoryKey,LayerCategoryCount);
        ++col;

        for( int i = 0; i < LayerCategoryCount; ++i )
        {
            item->setCheckState(col, it->visibility[i] ? Qt::Checked : Qt::Unchecked );
            item->setData(col,DataLayerCategoryKey,i);
            ++col;
        }

        ui->tblLayers->insertTopLevelItem( 0, item );
    }

    //resize the columns
    // todo: should be done only once maybe
    for( int i = 1; i < LayerCategoryCount+2; i++ )
    {
        ui->tblLayers->resizeColumnToContents(i);
    }
}

void MainWindow::on_btnClear_clicked()
{
    ui->graphView->clear();
}

void MainWindow::on_btnZoomIn_clicked()
{
    ui->graphView->zoomIn();
    ui->graphView->setAutoFit( false );
}

void MainWindow::on_btnZoomOut_clicked()
{
    ui->graphView->zoomOut();
    ui->graphView->setAutoFit( false );
}

void MainWindow::on_btnZoomFit_clicked()
{
    ui->graphView->zoomFit();    
}

void MainWindow::on_btnOpen_clicked()
{
  mLastSelectedFile = QFileDialog::getOpenFileName( this, QString( "Select an image file" ), mLastSelectedFile, QString("Json(*.txt *.json)"));
  if( !mLastSelectedFile.isEmpty() )
    mSceneManager->load( mLastSelectedFile, true );
}


void MainWindow::on_btnTest_clicked()
{
    int test = 0;

    switch( test ) {
        case 0: {
            // simple command test
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"clear\" }" ) );

            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"layer\":\"layer1\", \"x\":   0, \"y\": 0,  \"text\":\"center-red\",   \"color\":\"0xffff0000\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"layer\":\"layer1\", \"x\":   0, \"y\": 0,  \"text\":\"center-green\", \"color\":\"0xff00ff00\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"layer\":\"layer1\", \"x\":   0, \"y\": 0,  \"text\":\"center-blue\",  \"color\":\"0xff0000ff\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"layer\":\"layer1\", \"x\":   0, \"y\": 0,  \"text\":\"center-white\", \"color\":\"0xffffffff\" }") );

            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"x\":-180, \"y\":-90, \"text\":\"left bottom\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"x\":-180, \"y\": 90, \"text\":\"left top\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"x\": 180, \"y\":-90, \"text\":\"right bottom\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"x\": 180, \"y\": 90, \"text\":\"right top\" }") );

            mSceneManager->addCommand( QByteArray( "{ \"command\":\"point\", \"layer\":\"layer1\", \"x\": 90, \"y\":45, \"color\":\"0xffff0000\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"point\", \"x\":-90, \"y\":45, \"color\":\"0xff00ff00\" }") );

            mSceneManager->addCommand( QByteArray( "{ \"command\":\"line\", \"layer\":\"layer1\", \"x0\": 90, \"y0\":-45, \"x1\":-90, \"y1\":-45, \"color\":\"0xff00ff00\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"rect\", \"x0\": 90, \"y0\":-45, \"x1\":-90, \"y1\": 45, \"color\":\"0x7f1f2f3f\" }") );

            mSceneManager->addCommand( QByteArray( "{ \"command\":\"polyline\", \"x\": [20,90,20,90], \"y\": [20,20,90,90], \"color\":\"0xfff66f00\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"x\":20, \"y\":20, \"text\":\"pl0\", \"color\":\"0xfff66f00\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"x\":90, \"y\":20, \"text\":\"pl1\", \"color\":\"0xfff66f00\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"x\":20, \"y\":90, \"text\":\"pl2\", \"color\":\"0xfff66f00\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"x\":90, \"y\":90, \"text\":\"pl3\", \"color\":\"0xfff66f00\" }") );

            mSceneManager->addCommand( QByteArray( "{ \"command\":\"poly\", \"x\": [0,90,90], \"y\": [0,0,20], \"color\":\"0x7f1f2f3f\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"x\":0, \"y\":0, \"text\":\"p0\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"x\":90, \"y\":0, \"text\":\"p1\" }") );
            mSceneManager->addCommand( QByteArray( "{ \"command\":\"text\", \"x\":90, \"y\":20, \"text\":\"p2\" }") );

            mSceneManager->addCommand( QByteArray( "{ \"command\":\"cache\", \"id\":\"1\" }") );
        } break;

        case 1: {

            // stress test
            QByteArray cmd;
            int cnt = 100;
            int k = 0;
            for( int y = 0; y < cnt; y += 1 )
            {
                for( int x = 0; x < cnt; x += 1 )
                {
                    int sx = x * 10;
                    int sy = y * 10;

                    cmd.clear();
                    QTextStream(&cmd) << "{ \"command\":\"text\", \"x\":" << sx << ", \"y\":" << sy <<", \"text\":\"id_" << k << "\" }";
                    mSceneManager->addCommand(cmd);

                    cmd.clear();
                    QTextStream(&cmd) << "{ \"command\":\"text\", \"x\":" << sx << ", \"y\":" << sy <<", \"text\":\"idr_" << k << "\", \"color\":\"0xffff0000\" }";
                    mSceneManager->addCommand(cmd);

                    cmd.clear();
                    QTextStream(&cmd) << "{ \"command\":\"point\", \"x\":" << sx-5 << ", \"y\":" << sy << "}";
                    mSceneManager->addCommand(cmd);

                    ++k;
                }
            }
        } break;
        default: break;
    }
}

void MainWindow::on_btnScreenCapture_clicked()
{
    mLastSelectedScreenFile = QFileDialog::getSaveFileName( this, QString( "Select image file" ), mLastSelectedScreenFile, QString("Images(*.jpg *.png *.xpm)"));
    if( !mLastSelectedScreenFile.isEmpty() )
    {
        QRect rect = ui->graphView->geometry();
        QPixmap pixMap = ui->graphView->grab( QRect( 0,0, rect.width(), rect.height() ) );
        pixMap.save(mLastSelectedScreenFile);
    }
}
