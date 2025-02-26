// TrainingVisualizer.cpp
#include "TrainingVisualizer.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QtCharts>
#include <QVBoxLayout>
#include <QFileSystemWatcher>
#include <QCheckBox>
#include <QSlider>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPixmap>

std::unique_ptr<QApplication> TrainingVisualizer::app;

struct TrainingVisualizer::Private {
    struct TabData {
        QChartView* chartView;
        QLineSeries* series;
        TabData() : chartView(nullptr), series(nullptr) {}
    };

    std::unique_ptr<QMainWindow> window;
    QTabWidget* tabWidget;
    QMap<QString, TabData> tabData;
    QChartView* chartView;  // Add these member variables
    QLineSeries* series;    // for backward compatibility
    QFileSystemWatcher fileWatcher;
    QString lastFileName;
    QChartView* mainChartView;
    QMap<QString, QLineSeries*> metricSeries;
    QTableWidget* statsTable;
    /*
    void createUnifiedChart(QVBoxLayout* layout);
    void createStatisticsPanel(QVBoxLayout* layout);
    void addRunCheckbox(QVBoxLayout* layout, const QString& name, const QString& color, bool checked);
    void addMetricCheckbox(QVBoxLayout* layout, const QString& name, bool checked);
    void toggleRunVisibility(const QString& run, bool visible);
    void updateChartSmoothing(double factor);
    void updateStatisticsTable();
    void loadDataFromFile(const QString& fileName);
    */
    Private() : window(std::make_unique<QMainWindow>()),
                tabWidget(nullptr),
                chartView(nullptr),
                series(nullptr) {
        setupUI();
    }

    void setupUI() {
        window->setWindowTitle("MetricViz");
        window->resize(1000, 700);
    
        // Create main widget and layout
        auto centralWidget = new QWidget();
        auto mainLayout = new QHBoxLayout(centralWidget);
        
        // Create sidebar
        auto sidebarWidget = new QWidget();
        auto sidebarLayout = new QVBoxLayout(sidebarWidget);
        sidebarWidget->setMaximumWidth(180);
        sidebarWidget->setStyleSheet("background-color:rgb(0, 0, 0); border-right: 1px solid #e0e0e0;");
        
        // Add run selection section
        auto runLabel = new QLabel("Runs");
        runLabel->setStyleSheet("font-weight: bold;");
        sidebarLayout->addWidget(runLabel);
        
        // For demo purposes we'll add a fixed list of runs
        // In a real app, you'd populate this dynamically
        addRunCheckbox(sidebarLayout, "run-01", "#3366cc", true);
        addRunCheckbox(sidebarLayout, "run-02", "#dc3912", false);
        addRunCheckbox(sidebarLayout, "run-03", "#ff9900", false);
        
        sidebarLayout->addSpacing(20);
        
        // Add metrics selection section
        auto metricsLabel = new QLabel("Metrics");
        metricsLabel->setStyleSheet("font-weight: bold;");
        sidebarLayout->addWidget(metricsLabel);
        
        // Create metric checkboxes and connect them to toggle visibility
        addMetricCheckbox(sidebarLayout, "Accuracy", true);
        addMetricCheckbox(sidebarLayout, "Training Loss", false);
        addMetricCheckbox(sidebarLayout, "Validation Loss", false);
        addMetricCheckbox(sidebarLayout, "Learning Rate", false);
        addMetricCheckbox(sidebarLayout, "Perplexity", false);
        addMetricCheckbox(sidebarLayout, "Tokens per Second", false);
        
        sidebarLayout->addSpacing(20);
        
        // Add smoothing control
        auto smoothingLabel = new QLabel("Smoothing");
        smoothingLabel->setStyleSheet("font-weight: bold;");
        sidebarLayout->addWidget(smoothingLabel);
        
        auto smoothingSlider = new QSlider(Qt::Horizontal);
        smoothingSlider->setRange(0, 100);
        smoothingSlider->setValue(50);  // Default value 0.5
        sidebarLayout->addWidget(smoothingSlider);
        
        auto smoothingValue = new QLabel("0.5");
        sidebarLayout->addWidget(smoothingValue);
        
        // Connect slider to update smoothing and label
        QObject::connect(smoothingSlider, &QSlider::valueChanged, 
            [this, smoothingValue](int value) {
                double smoothingFactor = value / 100.0;
                smoothingValue->setText(QString::number(smoothingFactor, 'f', 2));
                updateChartSmoothing(smoothingFactor);
            });
        
        sidebarLayout->addStretch();
        
        // Create main content area with single chart
        auto contentWidget = new QWidget();
        auto contentLayout = new QVBoxLayout(contentWidget);
        
        // Create main chart title
        auto chartTitle = new QLabel("Training Metrics");
        chartTitle->setStyleSheet("font-size: 16pt; font-weight: bold;");
        contentLayout->addWidget(chartTitle);
        
        // Create the single unified chart
        createUnifiedChart(contentLayout);
        
        // Add statistics panel at the bottom
        createStatisticsPanel(contentLayout);
        
        // Add main sections to the layout
        mainLayout->addWidget(sidebarWidget);
        mainLayout->addWidget(contentWidget, 1);  // Content takes remaining space
        
        window->setCentralWidget(centralWidget);
        
        // Add toolbar with load button and export options
        auto toolbar = new QToolBar();
        toolbar->setStyleSheet("background-color: #2a3f5f; color: white;");
        
        auto projectSelector = new QPushButton("MyProject ▼");
        projectSelector->setStyleSheet("background-color: #1e2f48; color: white; border-radius: 5px; padding: 5px 10px;");
        toolbar->addWidget(projectSelector);
        
        toolbar->addSeparator();
        
        auto loadButton = new QPushButton("Load Data");
        loadButton->setStyleSheet("background-color: #1e2f48; color: white; border-radius: 5px; padding: 5px 10px;");
        toolbar->addWidget(loadButton);
        
        toolbar->addSeparator();
        
        auto exportButton = new QPushButton("Export");
        exportButton->setStyleSheet("background-color: #1e2f48; color: white; border-radius: 5px; padding: 5px 10px;");
        toolbar->addWidget(exportButton);
        
        auto settingsButton = new QPushButton("Settings");
        settingsButton->setStyleSheet("background-color: #1e2f48; color: white; border-radius: 5px; padding: 5px 10px;");
        toolbar->addWidget(settingsButton);
        
        window->addToolBar(Qt::TopToolBarArea, toolbar);
        
        // Connect load button to file dialog
        QObject::connect(loadButton, &QPushButton::clicked, [this]() {
            QString fileName = QFileDialog::getOpenFileName(window.get(),
                "Load Training Data", "",
                "CSV Files (*.csv);;All Files (*)");
            
            if (!fileName.isEmpty()) {
                loadDataFromFile(fileName);
            }
        });
    }

    void createUnifiedChart(QVBoxLayout* layout) {
        // Create and setup the main chart
        auto chart = new QChart();
        chart->setTitle("Training Metrics");
        chart->setMinimumSize(760, 380);  // Set minimum size
        chart->setAcceptHoverEvents(true);
        
        // Create axes
        auto axisX = new QValueAxis;
        axisX->setTitleText("Steps");
        axisX->setLabelFormat("%i");
        
        auto axisY = new QValueAxis;
        axisY->setTitleText("Value");
        axisY->setRange(0, 1);  // Default range, will be auto-adjusted later
        
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        
        // Create a map of metric name to series
        metricSeries.clear();
        for (const QString& metric : {"Accuracy", "Training Loss", "Validation Loss", 
                                     "Learning Rate", "Perplexity", "Tokens per Second"}) {
            // Create a series for each metric with a unique color
            auto series = new QLineSeries(chart);
            series->setName(metric);
            chart->addSeries(series);
            series->attachAxis(axisX);
            series->attachAxis(axisY);
            series->setPointsVisible(true);
            
            // Only show Accuracy by default
            series->setVisible(metric == "Accuracy");
            
            // Set colors
            QColor color;
            if (metric == "Accuracy") color = QColor("#3366cc");
            else if (metric == "Training Loss") color = QColor("#dc3912");
            else if (metric == "Validation Loss") color = QColor("#ff9900");
            else if (metric == "Learning Rate") color = QColor("#109618");
            else if (metric == "Perplexity") color = QColor("#990099");
            else if (metric == "Tokens per Second") color = QColor("#0099c6");
            
            QPen pen = series->pen();
            pen.setColor(color);
            pen.setWidth(2);
            series->setPen(pen);
            
            // Store series for later reference
            metricSeries[metric] = series;
            
            // Add tooltip
            QObject::connect(series, &QLineSeries::hovered, 
                [this, series](const QPointF &point, bool state) {
                    if (state) {
                        QToolTip::showText(
                            QCursor::pos(),
                            QString("%1\nStep: %2\nValue: %3")
                                .arg(series->name())
                                .arg(int(point.x()))
                                .arg(point.y(), 0, 'f', 4),
                            nullptr
                        );
                    } else {
                        QToolTip::hideText();
                    }
                });
        }
        
        // Create the chart view
        mainChartView = new QChartView(chart);
        mainChartView->setRenderHint(QPainter::Antialiasing);
        mainChartView->setRubberBand(QChartView::RectangleRubberBand);
        mainChartView->setDragMode(QGraphicsView::ScrollHandDrag);
        
        // Create legend and make it interactive
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);
        chart->legend()->setMarkerShape(QLegend::MarkerShapeRectangle);
        
        layout->addWidget(mainChartView);
    }
    
    void createStatisticsPanel(QVBoxLayout* layout) {
        // Create the statistics table
        statsTable = new QTableWidget();
        statsTable->setColumnCount(6);
        statsTable->setHorizontalHeaderLabels({"Run", "Min", "Max", "Mean", "Last", "Steps"});
        statsTable->setMaximumHeight(170);
        statsTable->horizontalHeader()->setStyleSheet("background-color: #f0f0f0;");
        statsTable->horizontalHeader()->setStretchLastSection(true);
        
        // Set the table to fill the width
        statsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        
        layout->addWidget(statsTable);
    }
    
    void addRunCheckbox(QVBoxLayout* layout, const QString& name, const QString& color, bool checked) {
        auto checkbox = new QCheckBox(name);
        checkbox->setChecked(checked);
        
        // Create a colored square icon for the checkbox
        QPixmap pixmap(16, 16);
        pixmap.fill(QColor(color));
        checkbox->setIcon(QIcon(pixmap));
        
        layout->addWidget(checkbox);
        
        // Connect checkbox to show/hide this run's data
        QObject::connect(checkbox, &QCheckBox::toggled, 
            [this, name](bool checked) {
                toggleRunVisibility(name, checked);
            });
    }
    
    void addMetricCheckbox(QVBoxLayout* layout, const QString& name, bool checked) {
        auto checkbox = new QCheckBox(name);
        checkbox->setChecked(checked);
        layout->addWidget(checkbox);
        
        // Connect checkbox to show/hide this metric
        QObject::connect(checkbox, &QCheckBox::toggled, 
            [this, name](bool checked) {
                if (metricSeries.contains(name)) {
                    metricSeries[name]->setVisible(checked);
                    updateStatisticsTable();
                }
            });
    }
    
    void toggleRunVisibility(const QString& run, bool visible) {
        // In a real implementation, this would filter data by run ID
        // For now, we'll just show/hide all data
        for (auto series : metricSeries) {
            series->setVisible(visible);
        }
        updateStatisticsTable();
    }
    
    void updateChartSmoothing(double factor) {
        // Apply smoothing to each visible series
        // This would need a real smoothing algorithm in a production app
        // For now, we'll just log the smoothing factor change
        qDebug() << "Smoothing factor changed to:" << factor;
        
        // In a real implementation, you would:
        // 1. Store both raw and smoothed data
        // 2. Apply a smoothing algorithm (e.g., moving average) with the specified factor
        // 3. Update the visible series with smoothed data
    }
            
    void loadDataFromFile(const QString& fileName) {
        // Setup file watching similar to your original code
        if (!lastFileName.isEmpty()) {
            fileWatcher.removePath(lastFileName);
        }
        
        fileWatcher.addPath(fileName);
        lastFileName = fileName;
        
        QObject::connect(&fileWatcher, &QFileSystemWatcher::fileChanged, [this](const QString &path){
            loadDataFromFile(path);
        });
        
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Failed to open file";
            return;
        }
        
        // Create a mapping from metric names to CSV column names
        QMap<QString, QString> metricToHeader {
            {"Accuracy", "accuracy"},
            {"Training Loss", "train_loss"},
            {"Validation Loss", "val_loss"},
            {"Perplexity", "perplexity"},
            {"Tokens per Second", "tokens_per_second"},
            {"Learning Rate", "learning_rate"}
        };
        
        // Clear existing data in all series
        for (auto series : metricSeries) {
            series->clear();
        }
        
        // Read and parse the CSV
        QTextStream in(&file);
        QString headerLine = in.readLine();
        QStringList headers = headerLine.split(',');
        
        // Find the steps column
        int stepsIndex = headers.indexOf("Step");
        if (stepsIndex < 0) {
            qDebug() << "No 'Step' column found in CSV";
            file.close();
            return;
        }
        
        // Map metric names to column indices
        QMap<QString, int> metricColumnIndex;
        for (auto it = metricToHeader.begin(); it != metricToHeader.end(); ++it) {
            QString metricName = it.key();
            QString headerName = it.value();
            int colIndex = headers.indexOf(headerName);
            if (colIndex >= 0) {
                metricColumnIndex[metricName] = colIndex;
            }
        }
        
        // Read each line and populate series data
        int currentStep = 0;  // Fallback
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();
        
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');
            if (fields.size() < 2) continue;
            
            // Get step value
            bool ok;
            double step = fields[stepsIndex].toDouble(&ok);
            if (!ok) {
                step = currentStep++;
            }
            
            // For each metric with a valid column index, add data to its series
            for (auto it = metricColumnIndex.begin(); it != metricColumnIndex.end(); ++it) {
                QString metricName = it.key();
                int colIndex = it.value();
                
                if (colIndex >= fields.size()) continue;
                QString valueStr = fields[colIndex];
                if (valueStr == "NA") continue;
                
                bool ok;
                double value = valueStr.toDouble(&ok);
                if (!ok) continue;
                
                metricSeries[metricName]->append(step, value);
                
                // Track min/max for Y axis scaling
                minY = qMin(minY, value);
                maxY = qMax(maxY, value);
            }
        }
        file.close();
        
        // Adjust Y axis range with some padding
        if (maxY > minY) {
            double padding = (maxY - minY) * 0.1;
            auto chart = mainChartView->chart();
            auto yAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
            if (yAxis) {
                yAxis->setRange(minY - padding, maxY + padding);
            }
            
            // Also adjust X axis to show full range of steps
            auto xAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
            if (xAxis && !metricSeries["Accuracy"]->points().isEmpty()) {
                double minX = metricSeries["Accuracy"]->points().first().x();
                double maxX = metricSeries["Accuracy"]->points().last().x();
                double padding = (maxX - minX) * 0.05;
                xAxis->setRange(minX - padding, maxX + padding);
            }
        }
        
        // Update statistics table
        updateStatisticsTable();
    }
    
    void updateStatisticsTable() {
        statsTable->setRowCount(0);  // Clear existing rows
        
        // Add a row for each visible series
        for (auto it = metricSeries.begin(); it != metricSeries.end(); ++it) {
            QString metricName = it.key();
            QLineSeries* series = it.value();
            
            if (!series->isVisible() || series->count() == 0) {
                continue;
            }
            
            int row = statsTable->rowCount();
            statsTable->insertRow(row);
            
            // Column 0: Run/Metric name with color
            QTableWidgetItem* nameItem = new QTableWidgetItem(metricName);
            QColor color = series->pen().color();
            QPixmap pixmap(16, 16);
            pixmap.fill(color);
            nameItem->setIcon(QIcon(pixmap));
            statsTable->setItem(row, 0, nameItem);
            
            // Calculate statistics
            double min = std::numeric_limits<double>::max();
            double max = std::numeric_limits<double>::lowest();
            double sum = 0;
            double last = 0;
            int steps = 0;
            
            for (int i = 0; i < series->count(); ++i) {
                double value = series->at(i).y();
                min = qMin(min, value);
                max = qMax(max, value);
                sum += value;
                if (i == series->count() - 1) {
                    last = value;
                    steps = static_cast<int>(series->at(i).x());
                }
            }
            
            double mean = sum / series->count();
            
            // Column 1: Min
            statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(min, 'f', 4)));
            
            // Column 2: Max
            statsTable->setItem(row, 2, new QTableWidgetItem(QString::number(max, 'f', 4)));
            
            // Column 3: Mean
            statsTable->setItem(row, 3, new QTableWidgetItem(QString::number(mean, 'f', 4)));
            
            // Column 4: Last
            statsTable->setItem(row, 4, new QTableWidgetItem(QString::number(last, 'f', 4)));
            
            // Column 5: Steps
            statsTable->setItem(row, 5, new QTableWidgetItem(QString::number(steps)));
            
            /*
            // Set background color for alternating rows
            if (row % 2 == 1) {
                for (int col = 0; col < statsTable->columnCount(); ++col) {
                    if (statsTable->item(row, col)) {
                        statsTable->item(row, col)->setBackground(QColor("#f9f9f9"));
                    }
                }
            }
                */
        }
    }
}; // End of Private struct definition

// Define TrainingVisualizer class methods outside of the Private struct
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