#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QMap>
#include <QPainter>
#include <QPair>
#include <QVector>

#include <algorithm>
#include <cmath>

QT_CHARTS_USE_NAMESPACE


//////////// PESTAÑA ANALISIS ////////////////

void MainWindow::configurarAnalisis() {
    //configura el combo de periodo
    bool estadoSenales = ui->comboPeriodoAnalisis->blockSignals(true);

    ui->comboPeriodoAnalisis->clear();
    ui->comboPeriodoAnalisis->addItem("Todos");
    ui->comboPeriodoAnalisis->addItem("Hoy");
    ui->comboPeriodoAnalisis->addItem("Últimos 7 días");
    ui->comboPeriodoAnalisis->addItem("Este mes");

    ui->comboPeriodoAnalisis->blockSignals(estadoSenales);

    //como las columnas fueron agregadas desde Qt Designer, solo se asegura que existan 8
    ui->tableAnalisis->setColumnCount(8);

    //configura el comportamiento visual de la tabla
    ui->tableAnalisis->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableAnalisis->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableAnalisis->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableAnalisis->setSelectionMode(QAbstractItemView::SingleSelection);

    //textos iniciales
    ui->labelVentasxDia->setText("Ventas totales: $0");
    ui->labelProductosCriticos->setText("Productos críticos: 0");
}

void MainWindow::cargarDatosAnalisis() {
    QString periodo = ui->comboPeriodoAnalisis->currentText();
    QString textoPeriodo = (periodo == "Todos") ? "totales" : periodo.toLower();

    //textos iniciales
    ui->labelVentasxDia->setText("Ventas " + textoPeriodo + ": $0");
    ui->labelProductosCriticos->setText("Productos críticos: 0");

    if(!baseDatos.estaAbierta()) {
        ui->tableAnalisis->setRowCount(0);
        return;
    }

    QVector<ProductoDB> todosProductos = baseDatos.obtenerProductos("Todas", "Todos", "");
    QVector<HistorialVenta> ventas = baseDatos.obtenerHistorialVentas(periodo);

    ui->tableAnalisis->setRowCount(0);

    int contadorCriticos = 0;
    int sumaTotalVentas = 0;

    QMap<QString, int> ventasPorDia;
    QMap<QString, int> cantidadPorProducto;

    //calcula datos generales de ventas
    for(int i = 0; i < ventas.size(); i++) {
        sumaTotalVentas += ventas[i].getSubtotal();

        QString dia = ventas[i].getFecha().mid(5, 5);
        ventasPorDia[dia] += ventas[i].getSubtotal();

        cantidadPorProducto[ventas[i].getProducto()] += ventas[i].getCantidad();
    }

    ui->labelVentasxDia->setText("Ventas " + textoPeriodo + ": $" + QString::number(sumaTotalVentas));

    //cantidad de dias usada para el promedio movil de demanda
    int diasPromedioMovil = 7;

    //llena tabla de productos criticos segun punto de reorden
    for(int i = 0; i < todosProductos.size(); i++) {
        ProductoDB prod = todosProductos[i];

        //calcula demanda promedio diaria usando ventas recientes
        double demandaPromedio = baseDatos.calcularDemandaPromedioMovil(prod.getCodigo(), diasPromedioMovil);

        //calcula punto de reorden: demanda promedio * lead time + stock de seguridad
        double puntoReordenDecimal = demandaPromedio * prod.getLeadTime() + prod.getStockSeguridad();
        int puntoReorden = static_cast<int>(std::ceil(puntoReordenDecimal));

        //solo se muestran productos criticos
        if(prod.getStock() <= puntoReorden) {
            int fila = ui->tableAnalisis->rowCount();
            ui->tableAnalisis->insertRow(fila);

            ui->tableAnalisis->setItem(fila, 0, new QTableWidgetItem(prod.getCodigo()));
            ui->tableAnalisis->setItem(fila, 1, new QTableWidgetItem(prod.getNombre()));
            ui->tableAnalisis->setItem(fila, 2, new QTableWidgetItem(prod.getCategoria()));
            ui->tableAnalisis->setItem(fila, 3, new QTableWidgetItem(QString::number(prod.getStock())));
            ui->tableAnalisis->setItem(fila, 4, new QTableWidgetItem(QString::number(demandaPromedio, 'f', 2)));
            ui->tableAnalisis->setItem(fila, 5, new QTableWidgetItem(QString::number(prod.getLeadTime())));
            ui->tableAnalisis->setItem(fila, 6, new QTableWidgetItem(QString::number(puntoReorden)));

            QTableWidgetItem *itemEstado = new QTableWidgetItem("CRÍTICO");
            itemEstado->setForeground(Qt::red);

            ui->tableAnalisis->setItem(fila, 7, itemEstado);

            contadorCriticos++;
        }
    }

    ui->labelProductosCriticos->setText("Productos críticos: " + QString::number(contadorCriticos));


    ////////////// GRAFICO 1: VENTAS POR DIA ////////////////

    QChart *chart1 = new QChart();
    QBarSeries *series1 = new QBarSeries();
    QBarSet *set1 = new QBarSet("Ventas $");
    QStringList categorias1;

    for(auto it = ventasPorDia.begin(); it != ventasPorDia.end(); ++it) {
        categorias1 << it.key();
        *set1 << it.value();
    }

    if(categorias1.isEmpty()) {
        categorias1 << "Sin ventas";
        *set1 << 0;
    }

    series1->append(set1);
    chart1->addSeries(series1);
    chart1->setTitle("Ventas por día");
    chart1->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX1 = new QBarCategoryAxis();
    axisX1->append(categorias1);

    QValueAxis *axisY1 = new QValueAxis();
    axisY1->setLabelFormat("%d");

    chart1->addAxis(axisX1, Qt::AlignBottom);
    chart1->addAxis(axisY1, Qt::AlignLeft);

    series1->attachAxis(axisX1);
    series1->attachAxis(axisY1);

    QChart *chartAnterior1 = ui->chartVentas->chart();
    ui->chartVentas->setChart(chart1);

    if(chartAnterior1 != nullptr) {
        delete chartAnterior1;
    }

    ui->chartVentas->setRenderHint(QPainter::Antialiasing);


    ////////////// GRAFICO 2: PRODUCTOS MAS VENDIDOS ////////////////

    QVector<QPair<QString, int>> productosOrdenados;

    for(auto it = cantidadPorProducto.begin(); it != cantidadPorProducto.end(); ++it) {
        productosOrdenados.push_back(qMakePair(it.key(), it.value()));
    }

    std::sort(productosOrdenados.begin(), productosOrdenados.end(),
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                  return a.second > b.second;
              });

    QChart *chart2 = new QChart();
    QBarSeries *series2 = new QBarSeries();
    QBarSet *set2 = new QBarSet("Unidades");
    QStringList categorias2;

    int limite = productosOrdenados.size();

    if(limite > 5) {
        limite = 5;
    }

    for(int i = 0; i < limite; i++) {
        categorias2 << productosOrdenados[i].first;
        *set2 << productosOrdenados[i].second;
    }

    if(categorias2.isEmpty()) {
        categorias2 << "Sin ventas";
        *set2 << 0;
    }

    series2->append(set2);
    chart2->addSeries(series2);
    chart2->setTitle("Productos más vendidos por cantidad");
    chart2->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX2 = new QBarCategoryAxis();
    axisX2->append(categorias2);

    QValueAxis *axisY2 = new QValueAxis();
    axisY2->setLabelFormat("%d");

    chart2->addAxis(axisX2, Qt::AlignBottom);
    chart2->addAxis(axisY2, Qt::AlignLeft);

    series2->attachAxis(axisX2);
    series2->attachAxis(axisY2);

    QChart *chartAnterior2 = ui->chartProductosmasVendidos->chart();
    ui->chartProductosmasVendidos->setChart(chart2);

    if(chartAnterior2 != nullptr) {
        delete chartAnterior2;
    }

    ui->chartProductosmasVendidos->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::on_comboPeriodoAnalisis_currentTextChanged(const QString& texto) {
    //actualiza la pestaña de analisis al cambiar el periodo
    Q_UNUSED(texto);
    cargarDatosAnalisis();
}
