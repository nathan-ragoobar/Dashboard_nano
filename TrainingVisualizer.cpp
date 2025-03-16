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
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QProgressDialog>

std::unique_ptr<QApplication> TrainingVisualizer::app;

struct TrainingVisualizer::Private {
    struct TabData {
        QChartView* chartView;
        QLineSeries* series;
        TabData() : chartView(nullptr), series(nullptr) {}
    };

    struct RunData {
        QString fileName;
        QMap<QString, QLineSeries*> series; // Maps metric name to its series
        QMap<QString, QColor> colors; // Store color for each metric in this run
    };
    
    QMap<QString, RunData> runs; // Maps run name to its data
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
    QMap<QString, bool> metricVisibility;  // Tracks whether each metric is visible
    QMap<QString, bool> runVisibility;  // Tracks whether each run is visible
    QFutureWatcher<void>* fileLoadWatcher = nullptr;
    QProgressDialog* progressDialog = nullptr;
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

        // Initialize defaults for metric visibility settings
        metricVisibility["Accuracy"] = true;
        metricVisibility["Training Loss"] = false;
        metricVisibility["Validation Loss"] = false;
        metricVisibility["Learning Rate"] = false;
        metricVisibility["Perplexity"] = false;
        metricVisibility["Tokens per Second"] = false;
        
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
            
            // Hide template series from legend
            series->setVisible(false);
            
            // Store series for later reference
            metricSeries[metric] = series;
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
        
        // If layout is provided, add to it, otherwise find the runs layout
        if (layout) {
            layout->addWidget(checkbox);
        } else {
            // Find the sidebar layout with runs
            // This is a bit hacky but works for this demo
            auto centralWidget = window->centralWidget();
            if (centralWidget) {
                auto mainLayout = centralWidget->layout();
                if (mainLayout) {
                    auto sidebarWidget = mainLayout->itemAt(0)->widget();
                    if (sidebarWidget) {
                        auto sidebarLayout = qobject_cast<QVBoxLayout*>(sidebarWidget->layout());
                        if (sidebarLayout) {
                            // Find the "Runs" label
                            for (int i = 0; i < sidebarLayout->count(); ++i) {
                                auto item = sidebarLayout->itemAt(i);
                                if (item && item->widget()) {
                                    auto label = qobject_cast<QLabel*>(item->widget());
                                    if (label && label->text() == "Runs") {
                                        // Found the Runs label, insert after it
                                        // Use insertItem instead of insertWidget
                                        sidebarLayout->insertWidget(i + 1, checkbox);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Connect checkbox to show/hide this run's data
        QObject::connect(checkbox, &QCheckBox::toggled, 
            [this, name](bool checked) {
                toggleRunVisibility(name, checked);
            });
    }
    
    void addMetricCheckbox(QVBoxLayout* layout, const QString& name, bool checked) {
        metricVisibility[name] = checked;  // Store the initial state
        
        auto checkbox = new QCheckBox(name);
        checkbox->setChecked(checked);
        checkbox->setStyleSheet("color: white;");
        layout->addWidget(checkbox);
        
        QObject::connect(checkbox, &QCheckBox::toggled, 
            [this, name](bool checked) {
                metricVisibility[name] = checked;  // Update the state
                
                // Update the series in all runs based on both metric and run visibility
                for (auto runIt = runs.begin(); runIt != runs.end(); ++runIt) {
                    QString runName = runIt.key();
                    RunData &runData = runIt.value();
                    if (runData.series.contains(name)) {
                        // Check if this run should be visible
                        bool runIsVisible = runVisibility.value(runName, true);
                        
                        // Only make the series visible if BOTH the run and metric are visible
                        runData.series[name]->setVisible(checked && runIsVisible);
                    }
                }
                
                updateStatisticsTable();
            });
    }
    
    void toggleRunVisibility(const QString& run, bool visible) {
        // Store the run visibility state
        runVisibility[run] = visible;
        
        // If run exists in our runs map, toggle visibility of its series
        if (runs.contains(run)) {
            RunData &runData = runs[run];
            for (auto it = runData.series.begin(); it != runData.series.end(); ++it) {
                const QString& metricName = it.key();
                // Only make the series visible if both the run AND the metric are supposed to be visible
                bool shouldBeVisible = visible && metricVisibility.value(metricName, false);
                it.value()->setVisible(shouldBeVisible);
            }
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
            // Reload the specific run that changed
            QString runName = "";
            for (auto it = runs.begin(); it != runs.end(); ++it) {
                if (it.value().fileName == path) {
                    runName = it.key();
                    break;
                }
            }
            
            if (!runName.isEmpty()) {
                loadDataFromFile(path, runName);
            }
        });
        
        // Generate a run name from the filename
        QFileInfo fileInfo(fileName);
        QString runName = fileInfo.baseName();
        
        // Check if this run already exists, and if so, append a number
        QString baseRunName = runName;
        int counter = 1;
        while (runs.contains(runName)) {
            runName = QString("%1-%2").arg(baseRunName).arg(counter++);
        }
        
        loadDataFromFile(fileName, runName);
        
        // Add a checkbox for this run if it's new
        QColor runColor = getUniqueColor(runs.size()); // You'll need to implement this function
        addRunCheckbox(nullptr, runName, runColor.name(), true); // Will need to modify addRunCheckbox
    }
    
   
   //modify to use async loading
    void loadDataFromFile(const QString& fileName, const QString& runName) {
        // Show a loading indicator
        if (!progressDialog) {
            progressDialog = new QProgressDialog("Loading data...", "Cancel", 0, 0, window.get());
            progressDialog->setWindowModality(Qt::WindowModal);
        }
        
        progressDialog->setLabelText(QString("Loading %1...").arg(runName));
        progressDialog->show();
        
        // Setup the future watcher if not already done
        if (!fileLoadWatcher) {
            fileLoadWatcher = new QFutureWatcher<void>();
            QObject::connect(fileLoadWatcher, &QFutureWatcher<void>::finished, [this]() {
                progressDialog->hide();
                updateAxisRanges();
                updateStatisticsTable();
            });
            
            QObject::connect(progressDialog, &QProgressDialog::canceled, [this]() {
                fileLoadWatcher->cancel();
            });
        }
        
        // Start the loading process in a separate thread
        QFuture<void> future = QtConcurrent::run([this, fileName, runName]() {
            processFile(fileName, runName);
        });
        
        fileLoadWatcher->setFuture(future);
    }

    // Create a new method to process the file (called from background thread)
    void processFile(const QString& fileName, const QString& runName) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Failed to open file";
            return;
        }
        
        // Create or get run data
        if (!runs.contains(runName)) {
            RunData newRun;
            newRun.fileName = fileName;
            runs[runName] = newRun;
            runVisibility[runName] = true;  // Initialize run visibility as true
        }
        RunData &runData = runs[runName];
        
        // Create a mapping from metric names to CSV column names
        QMap<QString, QString> metricToHeader {
            {"Accuracy", "accuracy"},
            {"Training Loss", "train_loss"},
            {"Validation Loss", "val_loss"},
            {"Perplexity", "perplexity"},
            {"Tokens per Second", "tokens_per_second"},
            {"Learning Rate", "learning_rate"}
        };
        
        // Remove any existing series for this run
        for (auto it = runData.series.begin(); it != runData.series.end(); ++it) {
            auto series = it.value();
            if (series) {
                mainChartView->chart()->removeSeries(series);
                delete series;
            }
        }
        runData.series.clear();
        
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
                
                // Create a new series for this metric
                auto series = new QLineSeries(mainChartView->chart());
                series->setName(QString("%1 - %2").arg(runName).arg(metricName));
                
                // Set color if already defined, otherwise generate a new one
                QColor color;

                // Store a base color for this run if not already assigned
                if (!runData.colors.contains("_baseColor")) {
                    // Generate a unique color for this run using the run index
                    int runIndex = 0;
                    for (auto it = runs.begin(); it != runs.end(); ++it) {
                        if (it.key() == runName) break;
                        runIndex++;
                    }
                    runData.colors["_baseColor"] = getUniqueColor(runIndex);
                }

                // Get the base run color
                color = runData.colors["_baseColor"];
                                
                // Apply slight variations based on the metric for better distinction
                if (metricName == "Training Loss") {
                    color = color.darker(115);  // Slightly darker
                } else if (metricName == "Validation Loss") {
                    color = color.lighter(115);  // Slightly lighter
                } else if (metricName == "Learning Rate") {
                    // Add green tint
                    color.setGreen(qMin(255, color.green() + 30));
                } else if (metricName == "Perplexity") {
                    // Add blue tint
                    color.setBlue(qMin(255, color.blue() + 30));
                } else if (metricName == "Tokens per Second") {
                    // Add red tint
                    color.setRed(qMin(255, color.red() + 30));
                }

                // Store the specific color for this metric in this run
                runData.colors[metricName] = color;
                
                QPen pen = series->pen();
                pen.setColor(color);
                pen.setWidth(2);
                series->setPen(pen);
                
                mainChartView->chart()->addSeries(series);
                
                // Attach to axes
                auto axes = mainChartView->chart()->axes();
                for (auto axis : axes) {
                    if (axis->orientation() == Qt::Horizontal)
                        series->attachAxis(axis);
                    else if (axis->orientation() == Qt::Vertical)
                        series->attachAxis(axis);
                }
                
                series->setVisible(metricVisibility.value(metricName, false));
                runData.series[metricName] = series;
                
                // Add tooltip similar to existing code
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
        }
        
        // Read data and populate series
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');
            if (fields.size() < 2) continue;
            
            // Get step value
            bool ok;
            double step = fields[stepsIndex].toDouble(&ok);
            if (!ok) continue;
            
            minX = qMin(minX, step);
            maxX = qMax(maxX, step);
            
            // For each metric with a valid column index, add data to its series
            for (auto it = metricColumnIndex.begin(); it != metricColumnIndex.end(); ++it) {
                QString metricName = it.key();
                int colIndex = it.value();
                
                if (!runData.series.contains(metricName) || colIndex >= fields.size())
                    continue;
                
                QString valueStr = fields[colIndex];
                if (valueStr == "NA") continue;
                
                bool ok;
                double value = valueStr.toDouble(&ok);
                if (!ok) continue;
                
                runData.series[metricName]->append(step, value);
                
                // Track min/max for Y axis scaling
                minY = qMin(minY, value);
                maxY = qMax(maxY, value);
            }
        }
        file.close();
        
        // Update axis ranges to accommodate all data
        updateAxisRanges();
        
        // Update statistics table
        updateStatisticsTable();
    }
    
    QColor getUniqueColor(int index) {
        // A list of distinguishable colors
        static const QList<QColor> colors = {
            QColor("#3366cc"), QColor("#dc3912"), QColor("#ff9900"),
            QColor("#109618"), QColor("#990099"), QColor("#0099c6"),
            QColor("#dd4477"), QColor("#66aa00"), QColor("#b82e2e"),
            QColor("#316395"), QColor("#994499"), QColor("#22aa99")
        };
        
        return colors[index % colors.size()];
    }

    void updateAxisRanges() {
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        bool hasData = false;
        
        // Check all series from all runs
        for (auto runIt = runs.begin(); runIt != runs.end(); ++runIt) {
            const RunData& runData = runIt.value();
            
            for (auto seriesIt = runData.series.begin(); seriesIt != runData.series.end(); ++seriesIt) {
                auto series = seriesIt.value();
                if (series->count() > 0) {
                    hasData = true;
                    
                    for (int i = 0; i < series->count(); ++i) {
                        minX = qMin(minX, series->at(i).x());
                        maxX = qMax(maxX, series->at(i).x());
                        minY = qMin(minY, series->at(i).y());
                        maxY = qMax(maxY, series->at(i).y());
                    }
                }
            }
        }
        
        // Adjust axis ranges if we have data
        if (hasData) {
            double paddingY = (maxY - minY) * 0.1;
            double paddingX = (maxX - minX) * 0.05;
            
            auto chart = mainChartView->chart();
            auto yAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
            auto xAxis = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
            
            if (yAxis) {
                yAxis->setRange(minY - paddingY, maxY + paddingY);
            }
            
            if (xAxis) {
                xAxis->setRange(minX - paddingX, maxX + paddingX);
            }
        }
    }

    void updateStatisticsTable() {
        statsTable->setRowCount(0);  // Clear existing rows
        
        // Add a row for each visible series from each run
        for (auto runIt = runs.begin(); runIt != runs.end(); ++runIt) {
            const QString& runName = runIt.key();
            const RunData& runData = runIt.value();
            
            for (auto seriesIt = runData.series.begin(); seriesIt != runData.series.end(); ++seriesIt) {
                const QString& metricName = seriesIt.key();
                QLineSeries* series = seriesIt.value();
                
                if (!series->isVisible() || series->count() == 0) {
                    continue;
                }
                
                int row = statsTable->rowCount();
                statsTable->insertRow(row);
                
                // Column 0: Run/Metric name with color
                QString displayName = QString("%1 - %2").arg(runName).arg(metricName);
                QTableWidgetItem* nameItem = new QTableWidgetItem(displayName);
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
                
                // Fill in the statistics
                statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(min, 'f', 4)));
                statsTable->setItem(row, 2, new QTableWidgetItem(QString::number(max, 'f', 4)));
                statsTable->setItem(row, 3, new QTableWidgetItem(QString::number(mean, 'f', 4)));
                statsTable->setItem(row, 4, new QTableWidgetItem(QString::number(last, 'f', 4)));
                statsTable->setItem(row, 5, new QTableWidgetItem(QString::number(steps)));
            }
        }
    }

};

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