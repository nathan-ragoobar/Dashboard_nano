// TrainingVisualizer.cpp
#include "TrainingVisualizer.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QtCharts>
#include <QVBoxLayout>

std::unique_ptr<QApplication> TrainingVisualizer::app;

struct TrainingVisualizer::Private {
    std::unique_ptr<QMainWindow> window;
    QTabWidget* tabWidget;
    QChartView* chartView;
    QLineSeries* series;
    
    Private() : window(std::make_unique<QMainWindow>()),
                tabWidget(nullptr),
                chartView(nullptr),
                series(nullptr) {
        setupUI();
    }

    void setupUI() {
        window->setWindowTitle("Training Dashboard");
        window->resize(800, 600);
    
        // Create toolbar and load button
        auto toolbar = new QToolBar();
        auto loadButton = new QPushButton("Load Data");
        toolbar->addWidget(loadButton);
        window->addToolBar(Qt::TopToolBarArea, toolbar);
    
        // Connect load button
        QObject::connect(loadButton, &QPushButton::clicked, [this]() {
            QString fileName = QFileDialog::getOpenFileName(window.get(),
                "Load Training Data", "",
                "CSV Files (*.csv);;All Files (*)");
            
            if (!fileName.isEmpty()) {
                loadDataFromFile(fileName);
            }
        });
    
        // Create tab widget
        tabWidget = new QTabWidget(window.get());
    
        // Create all tabs with charts
        createAccuracyTab();
        createPerplexityTab();
        createTokensTab();
        createLearningRateTab();
        createNormRateTab();
    
        window->setCentralWidget(tabWidget);
    }
    
    void createAccuracyTab() {
        auto widget = new QWidget();
        auto layout = new QVBoxLayout(widget);
    
        auto chart = new QChart();
        series = new QLineSeries(chart);
        chart->addSeries(series);
        chart->setTitle("Training Accuracy Over Time");
    
        // Create axes
        auto axisX = new QDateTimeAxis;
        axisX->setTitleText("Time");
        axisX->setFormat("hh:mm:ss");
        
        auto axisY = new QValueAxis;
        axisY->setTitleText("Accuracy");
        axisY->setRange(0, 1);
    
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    
        chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        layout->addWidget(chartView);
    
        tabWidget->addTab(widget, "Accuracy");
    }
    
    // Add similar methods for other tabs
    void createPerplexityTab() {
        auto widget = new QWidget();
        auto layout = new QVBoxLayout(widget);
    
        auto chart = new QChart();
        series = new QLineSeries(chart);
        chart->addSeries(series);
        chart->setTitle("Training Accuracy Over Time");
    
        // Create axes
        auto axisX = new QDateTimeAxis;
        axisX->setTitleText("Time");
        axisX->setFormat("hh:mm:ss");
        
        auto axisY = new QValueAxis;
        axisY->setTitleText("Accuracy");
        axisY->setRange(0, 1);
    
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    
        chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        layout->addWidget(chartView);
    
        tabWidget->addTab(widget, "Perplexity");
    }


    void createTokensTab() {
        auto widget = new QWidget();
        auto layout = new QVBoxLayout(widget);
    
        auto chart = new QChart();
        series = new QLineSeries(chart);
        chart->addSeries(series);
        chart->setTitle("Training Accuracy Over Time");
    
        // Create axes
        auto axisX = new QDateTimeAxis;
        axisX->setTitleText("Time");
        axisX->setFormat("hh:mm:ss");
        
        auto axisY = new QValueAxis;
        axisY->setTitleText("Accuracy");
        axisY->setRange(0, 1);
    
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    
        chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        layout->addWidget(chartView);
    
        tabWidget->addTab(widget, "Tokens");
    }

    void createLearningRateTab() {
        auto widget = new QWidget();
        auto layout = new QVBoxLayout(widget);
    
        auto chart = new QChart();
        series = new QLineSeries(chart);
        chart->addSeries(series);
        chart->setTitle("Training Accuracy Over Time");
    
        // Create axes
        auto axisX = new QDateTimeAxis;
        axisX->setTitleText("Time");
        axisX->setFormat("hh:mm:ss");
        
        auto axisY = new QValueAxis;
        axisY->setTitleText("Accuracy");
        axisY->setRange(0, 1);
    
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    
        chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        layout->addWidget(chartView);
    
        tabWidget->addTab(widget, "Learning Rate");
    }

    void createNormRateTab() {
        auto widget = new QWidget();
        auto layout = new QVBoxLayout(widget);
    
        auto chart = new QChart();
        series = new QLineSeries(chart);
        chart->addSeries(series);
        chart->setTitle("Training Accuracy Over Time");
    
        // Create axes
        auto axisX = new QDateTimeAxis;
        axisX->setTitleText("Time");
        axisX->setFormat("hh:mm:ss");
        
        auto axisY = new QValueAxis;
        axisY->setTitleText("Accuracy");
        axisY->setRange(0, 1);
    
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    
        chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        layout->addWidget(chartView);
    
        tabWidget->addTab(widget, "Normalization Rate");
    }

    void loadDataFromFile(const QString& fileName) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
    
        // Create a new series for the data
        auto newSeries = new QLineSeries();
        
        QTextStream in(&file);
        // Get header and find column indices
        QString headerLine = in.readLine();
        QStringList headers = headerLine.split(',');
        
        // Get current tab and determine which column to read
        QString currentTab = tabWidget->tabText(tabWidget->currentIndex());
        int dataColumnIndex = -1;
        
        // Map tab names to CSV column names
        if (currentTab == "Accuracy") dataColumnIndex = headers.indexOf("accuracy");
        else if (currentTab == "Perplexity") dataColumnIndex = headers.indexOf("perplexity");
        else if (currentTab == "Tokens") dataColumnIndex = headers.indexOf("tokens_per_second");
        else if (currentTab == "Learning Rate") dataColumnIndex = headers.indexOf("learning_rate");
        
        if (dataColumnIndex == -1) {
            delete newSeries;
            file.close();
            return;
        }
    
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');
            
            if (fields.size() > dataColumnIndex) {
                // Parse timestamp
                QDateTime timestamp = QDateTime::fromString(fields[0], "yyyy-MM-dd'T'HH:mm:ss");
                if (!timestamp.isValid()) continue;
    
                // Check for NA and parse value
                QString valueStr = fields[dataColumnIndex];
                if (valueStr == "NA") continue;
                
                bool ok;
                double value = valueStr.toDouble(&ok);
                if (!ok) continue;
    
                newSeries->append(timestamp.toMSecsSinceEpoch(), value);
            }
        }
    
        if (newSeries->count() > 0) {
            // Get current tab's chart
            auto currentWidget = tabWidget->currentWidget();
            auto chartView = currentWidget->findChild<QChartView*>();
            if (!chartView) return;
    
            auto chart = chartView->chart();
            
            // Remove old series and add new one
            chart->removeAllSeries();
            chart->addSeries(newSeries);
    
            // Reattach axes
            newSeries->attachAxis(chart->axes(Qt::Horizontal).first());
            newSeries->attachAxis(chart->axes(Qt::Vertical).first());
    
            // Update X axis range
            auto xAxis = qobject_cast<QDateTimeAxis*>(chart->axes(Qt::Horizontal).first());
            if (xAxis) {
                xAxis->setRange(
                    QDateTime::fromMSecsSinceEpoch(newSeries->at(0).x()),
                    QDateTime::fromMSecsSinceEpoch(newSeries->at(newSeries->count()-1).x())
                );
            }
    
            // Update Y axis range with padding
            double minY = std::numeric_limits<double>::max();
            double maxY = std::numeric_limits<double>::lowest();
            for (int i = 0; i < newSeries->count(); ++i) {
                double y = newSeries->at(i).y();
                minY = qMin(minY, y);
                maxY = qMax(maxY, y);
            }
    
            // Add 10% padding to Y axis
            double padding = (maxY - minY) * 0.1;
            auto yAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
            if (yAxis) {
                // Ensure range stays within logical bounds for the current tab
                QString currentTab = tabWidget->tabText(tabWidget->currentIndex());
                if (currentTab == "Accuracy") {
                    yAxis->setRange(
                        qMax(0.0, minY - padding),
                        qMin(1.0, maxY + padding)
                    );
                } else {
                    yAxis->setRange(minY - padding, maxY + padding);
                }
            }
        } else {
            delete newSeries;
        }
    
        file.close();
    }

};

bool TrainingVisualizer::initialize(int& argc, char** argv) {
    if (!app) {
        app = std::make_unique<QApplication>(argc, argv);
        return true;
    }
    return false;
}

TrainingVisualizer::TrainingVisualizer() : d(std::make_unique<Private>()) {}

TrainingVisualizer::~TrainingVisualizer() = default;

void TrainingVisualizer::addDataPoint(double timestamp, double accuracy) {
    if (d->series) {
        QDateTime datetime = QDateTime::fromSecsSinceEpoch(timestamp);
        d->series->append(datetime.toMSecsSinceEpoch(), accuracy);
        
        // Update axes ranges
        auto chart = d->chartView->chart();
        if (d->series->count() > 0) {
            chart->axes(Qt::Horizontal).first()->setRange(
                QDateTime::fromMSecsSinceEpoch(d->series->at(0).x()),
                QDateTime::fromMSecsSinceEpoch(d->series->at(d->series->count()-1).x())
            );
            
            // Find min/max accuracy for Y axis
            double minY = accuracy;
            double maxY = accuracy;
            for (int i = 0; i < d->series->count(); ++i) {
                double y = d->series->at(i).y();
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
            }
            
            // Add 10% padding to Y axis
            double padding = (maxY - minY) * 0.1;
            chart->axes(Qt::Vertical).first()->setRange(
                std::max(0.0, minY - padding),
                std::min(1.0, maxY + padding)
            );
        }
    }
}

void TrainingVisualizer::show() {
    if (d->window) {
        d->window->show();
    }
}

void TrainingVisualizer::processEvents() {
    if (app) {
        app->processEvents();
    }
}


int TrainingVisualizer::exec() {
    if (app) {
        return app->exec();
    }
    return 1;
}
