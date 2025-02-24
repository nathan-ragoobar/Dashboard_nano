/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCharts/QChartView>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *accuracyTab;
    QVBoxLayout *accuracyLayout;
    QChartView *accuracyChart;
    QWidget *perplexityTab;
    QVBoxLayout *perplexityLayout;
    QChartView *perplexityChart;
    QWidget *tokensTab;
    QVBoxLayout *tokensLayout;
    QChartView *tokensChart;
    QWidget *learningRateTab;
    QVBoxLayout *learningRateLayout;
    QChartView *learningRateChart;
    QWidget *normRateTab;
    QVBoxLayout *normRateLayout;
    QChartView *normRateChart;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        accuracyTab = new QWidget();
        accuracyTab->setObjectName("accuracyTab");
        accuracyLayout = new QVBoxLayout(accuracyTab);
        accuracyLayout->setObjectName("accuracyLayout");
        accuracyChart = new QChartView(accuracyTab);
        accuracyChart->setObjectName("accuracyChart");

        accuracyLayout->addWidget(accuracyChart);

        tabWidget->addTab(accuracyTab, QString());
        perplexityTab = new QWidget();
        perplexityTab->setObjectName("perplexityTab");
        perplexityLayout = new QVBoxLayout(perplexityTab);
        perplexityLayout->setObjectName("perplexityLayout");
        perplexityChart = new QChartView(perplexityTab);
        perplexityChart->setObjectName("perplexityChart");

        perplexityLayout->addWidget(perplexityChart);

        tabWidget->addTab(perplexityTab, QString());
        tokensTab = new QWidget();
        tokensTab->setObjectName("tokensTab");
        tokensLayout = new QVBoxLayout(tokensTab);
        tokensLayout->setObjectName("tokensLayout");
        tokensChart = new QChartView(tokensTab);
        tokensChart->setObjectName("tokensChart");

        tokensLayout->addWidget(tokensChart);

        tabWidget->addTab(tokensTab, QString());
        learningRateTab = new QWidget();
        learningRateTab->setObjectName("learningRateTab");
        learningRateLayout = new QVBoxLayout(learningRateTab);
        learningRateLayout->setObjectName("learningRateLayout");
        learningRateChart = new QChartView(learningRateTab);
        learningRateChart->setObjectName("learningRateChart");

        learningRateLayout->addWidget(learningRateChart);

        tabWidget->addTab(learningRateTab, QString());
        normRateTab = new QWidget();
        normRateTab->setObjectName("normRateTab");
        normRateLayout = new QVBoxLayout(normRateTab);
        normRateLayout->setObjectName("normRateLayout");
        normRateChart = new QChartView(normRateTab);
        normRateChart->setObjectName("normRateChart");

        normRateLayout->addWidget(normRateChart);

        tabWidget->addTab(normRateTab, QString());

        verticalLayout->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 24));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Training Metrics Visualizer", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(accuracyTab), QCoreApplication::translate("MainWindow", "Accuracy", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(perplexityTab), QCoreApplication::translate("MainWindow", "Perplexity", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tokensTab), QCoreApplication::translate("MainWindow", "Tokens/Second", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(learningRateTab), QCoreApplication::translate("MainWindow", "Learning Rate", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(normRateTab), QCoreApplication::translate("MainWindow", "Norm Rate", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
